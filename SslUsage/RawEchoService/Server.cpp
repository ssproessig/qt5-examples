#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>


int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("RawEchoServer application");
    (void) parser.addHelpOption();

    (void) parser.addOption({{"p", "port"}, "port to listen to", "port", "9876"});
    (void) parser.addOption( //
            {{"i", "interface"}, "interface to listen to", "interface", "127.0.0.1"});
    parser.process(QCoreApplication::arguments());

    auto const interface = QHostAddress(parser.value("interface"));
    auto const port = parser.value("port").toUShort();


    ///////////////////////////////////////////////////////////////////////////////////////////////
    // actual socket communication part
    QTcpServer srv;
    QObject::connect(&srv, &QTcpServer::newConnection, [&srv]() {
        auto* const client = srv.nextPendingConnection();
        qDebug() << "new client connected from" << //
                client->peerAddress().toString() << ":" << client->peerPort();

        client->write("Welcome to RawEchoServer!\n");

        (void) QObject::connect(client, &QTcpSocket::readyRead, [client]() { //
            auto const& data = client->readAll();
            qDebug() << "received: " << data;

            client->write(data);
        });

        (void) QObject::connect(client, &QTcpSocket::disconnected, [client]() {
            qDebug() << "client from" << //
                    client->peerAddress().toString() << ":" << client->peerPort() << "disconnected";
        });
    });

    if (!srv.listen(interface, port))
    {
        qCritical() << "unable to bind to" << interface << ":" << port << "!";
        return 1;
    }

    qDebug() << "raw echo server listening on" << interface << ":" << port << "!";
    ///////////////////////////////////////////////////////////////////////////////////////////////

    return a.exec();
}
