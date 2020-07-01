#include "Shared.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>


namespace
{
struct SecureServer final : QTcpServer
{
    void incomingConnection(qintptr const aSocketDescriptor) override
    {
        auto* sslSocket = new QSslSocket(this);

        if (sslSocket->setSocketDescriptor(aSocketDescriptor))
        {
            connect(sslSocket, QOverload<SslErrs>::of(&QSslSocket::sslErrors), [&](SslErrs errors) {
                dumpSslErrors(errors, *sslSocket);
            });
            addPendingConnection(sslSocket);

            setupSslConfigurationFor(*sslSocket, loadKey("server"), loadCert());

            sslSocket->startServerEncryption();
        }
        else
        {
            sslSocket->deleteLater();
        }
    }
};

} // namespace


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
    // actual socket communication part - same as for RawEchoServer, only using SecureServer
    try
    {
        SecureServer srv;

        (void) QObject::connect(&srv, &QTcpServer::newConnection, [&srv]() {
            auto* const client = srv.nextPendingConnection();
            qDebug() << "new client connected from" << //
                    client->peerAddress().toString() << ":" << client->peerPort();

            client->write("Welcome to SecureEchoServer!\n");

            (void) QObject::connect(client, &QTcpSocket::readyRead, [client]() {
                auto const& data = client->readAll();
                qDebug() << "echoing back: " << data;

                client->write(data);
            });

            (void) QObject::connect(client, &QTcpSocket::disconnected, [client]() {
                qDebug() << "client from" << //
                        client->peerAddress().toString() << ":" << client->peerPort()
                         << "disconnected";
            });
        });


        if (!srv.listen(interface, port))
        {
            throw std::logic_error(QString("unable to listen to %1:%2")
                                           .arg(interface.toString())
                                           .arg(port)
                                           .toStdString());
        }

        qDebug() << "secure echo server listening on" << interface << ":" << port << "!";
        return a.exec();
    }
    catch (std::exception const& ex)
    {
        qCritical() << "exception: " << ex.what();
        return 1;
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////
}
