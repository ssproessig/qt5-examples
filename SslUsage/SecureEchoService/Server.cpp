#include "Shared.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>


namespace
{
struct SecureServer final : QTcpServer
{
    QSslKey key;
    QSslCertificate cert;

    explicit SecureServer(QCommandLineParser const& parser)
    {
        key = loadKey(parser.value("key"), parser.value("pwd").toUtf8());
        cert = loadCert(parser.value("cert"));
    }

    void incomingConnection(qintptr const aSocketDescriptor) override
    {
        auto* const sslSocket = new QSslSocket(this);

        if (sslSocket->setSocketDescriptor(aSocketDescriptor))
        {
            connect(sslSocket, QOverload<SslErrs>::of(&QSslSocket::sslErrors), [&](SslErrs errors) {
                dumpSslErrors(errors, *sslSocket);
            });
            addPendingConnection(sslSocket);

            setupSslConfigurationFor(*sslSocket, key, cert);

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
    (void) parser.addOption( //
            {{"c", "cert"}, "Client certificate to use", "cert", ":/Certificate"});
    (void) parser.addOption( //
            {{"k", "key"}, "Client key to use", "key", ":/Key"});
    (void) parser.addOption( //
            {{"w", "pwd"}, "Key password to use", "pwd", "server"});
    parser.process(QCoreApplication::arguments());

    auto const interface = QHostAddress(parser.value("interface"));
    auto const port = parser.value("port").toUShort();


    ///////////////////////////////////////////////////////////////////////////////////////////////
    // actual socket communication part - same as for RawEchoServer, only using SecureServer
    SecureServer srv(parser);

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
                    client->peerAddress().toString() << ":" << client->peerPort() << "disconnected";
        });
    });


    if (!srv.listen(interface, port))
    {
        qCritical() << "unable to listen to" << interface << ":" << port << "!";
        return 1;
    }

    qDebug() << "secure echo server listening on" << interface << ":" << port << "!";
    return QCoreApplication::exec();

    ///////////////////////////////////////////////////////////////////////////////////////////////
}
