#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QTcpSocket>


int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("RawEchoServer application");
    (void) parser.addHelpOption();

    (void) parser.addOption({{"p", "port"}, "port to connect to", "port", "9876"});
    (void) parser.addOption( //
            {{"i", "host"}, "host to connect to", "host", "127.0.0.1"});
    parser.process(QCoreApplication::arguments());

    auto const host = parser.value("host");
    auto const port = parser.value("port").toUShort();


    ///////////////////////////////////////////////////////////////////////////////////////////////
    // actual socket communication part
    QTcpSocket s;
    QObject::connect(&s, &QTcpSocket::connected, [&s]() { s.write("hello from client!"); });
    QObject::connect(&s, &QTcpSocket::readyRead, [&s, &a]() {
        auto const data = s.readAll();
        qDebug() << "received: " << data;
        QCoreApplication::quit();
    });

    s.connectToHost(host, port);
    ///////////////////////////////////////////////////////////////////////////////////////////////

    return QCoreApplication::exec();
}
