#include "Shared.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QSslCipher>
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
    (void) parser.addOption( //
            {{"c", "cert"}, "Client certificate to use", "cert", ":/Certificate"});
    (void) parser.addOption( //
            {{"k", "key"}, "Client key to use", "key", ":/Key"});
    (void) parser.addOption( //
            {{"w", "pwd"}, "Key password to use", "pwd", "client_01"});

    parser.process(QCoreApplication::arguments());

    auto const host = parser.value("host");
    auto const port = parser.value("port").toUShort();

    auto const certFileName = parser.value("cert");
    auto const keyFileName = parser.value("key");
    auto const keyPwd = parser.value("pwd");


    ///////////////////////////////////////////////////////////////////////////////////////////////
    // actual secure socket communication part - changes to
    // 1. use QSslSocket instead of QTcpSocket
    // 2. connect to QSslSocket::sslErrors signal to find out why connection failed
    // 3. a) use connectToHostEncrypted() and waitForEncrypted(<timeout_in_ms>)
    //    b) OR use the signal encrypted() if you want to continue already and wait
    //       for secure connection asynchronously -QSslSocket has some internal buffering,
    //       you can even send already...
    QSslSocket s;
    setupSslConfigurationFor(s, loadKey(keyFileName, "client_01"), loadCert(certFileName));

    QObject::connect(&s, QOverload<SslErrs>::of(&QSslSocket::sslErrors), [&s](SslErrs errors) {
        dumpSslErrors(errors, s);
    });

    QObject::connect(&s, &QTcpSocket::readyRead, [&s, &a]() {
        auto const data = s.readAll();
        qDebug() << "received: " << data;
        QCoreApplication::quit();
    });

    qInfo() << "connecting to" << host << ":" << port << "...";
    s.connectToHostEncrypted(host, port);
    if (s.waitForEncrypted(5000))
    {
        qInfo() << "we have secure communication";
        qDebug() << "session cipher" << s.sessionCipher();

        s.write("hello from secure client!");
    }
    else
    {
        qCritical() << "unable to connect securely";
        return 1;
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////

    return QCoreApplication::exec();
}
