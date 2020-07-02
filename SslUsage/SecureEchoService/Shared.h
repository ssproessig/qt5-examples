#pragma once

#include <QDebug>
#include <QFile>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslSocket>
#include <QString>

#include <algorithm>
#include <stdexcept>


QByteArray readFromQrc(QString const& aFileName)
{
    QFile f(aFileName);
    if (f.open(QIODevice::ReadOnly))
    {
        return f.readAll();
    }

    qCritical() << QString("unable to read resource: %1").arg(aFileName);
    return {};
}


QSslKey loadKey(QString const& aFileName, QByteArray const& pwd)
{
    auto const key = QSslKey(readFromQrc(aFileName), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, pwd);
    qDebug() << "error loading key?" << key.isNull();
    qInfo() << "Loaded key: " << key;

    return key;
}


QSslCertificate loadCert(QString const& aFileName)
{
    auto const cert = QSslCertificate(readFromQrc(aFileName), QSsl::Pem);
    qDebug() << "error loading cert?" << cert.isNull();
    qInfo() << "Loaded certificate: " << cert;

    return cert;
}


void setupSslConfigurationFor(QSslSocket& s, QSslKey const& key, QSslCertificate const& cert)
{
    // alter the sockets existing configuration
    auto sslConfig = s.sslConfiguration();
    {
        // enforce TLS v1.3
        sslConfig.setProtocol(QSsl::TlsV1_3);

        // don't use a CA chain - only our own Certificate Authority is allowed
        sslConfig.setCaCertificates({QSslCertificate(readFromQrc(":/CA"), QSsl::Pem)});

        // load the passed certificate with the given key
        sslConfig.setPrivateKey(key);
        sslConfig.setLocalCertificate(cert);

        // verify the connection peer's certificate - fail connection if it fails
        // note: QSslSocket::QueryPeer will only warn if the peer verification failed - discouraged
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
    }
    s.setSslConfiguration(sslConfig);
}


void dumpCert(QSslCertificate const& cert)
{
    qDebug() << "---";
    qDebug() << "digest: " << cert.digest().toHex();
    qDebug() << "serial: " << cert.serialNumber().toHex();
    qDebug() << "---subject";
    qDebug() << "CN: " << cert.subjectInfo(QSslCertificate::CommonName).join(' ');
    qDebug() << "ORG:" << cert.subjectInfo(QSslCertificate::Organization).join(' ');
    qDebug() << "---issuer";
    qDebug() << "CN: " << cert.issuerInfo(QSslCertificate::CommonName);
    qDebug() << "ORG:" << cert.issuerInfo(QSslCertificate::Organization);
    qDebug() << "";
}


using SslErrs = QList<QSslError> const&;
void dumpSslErrors(SslErrs errors, QSslSocket const& forSocket)
{
    qDebug() << "SSL errors:";
    for (auto const& error : errors)
    {
        qCritical() << error.errorString();
    }

    qDebug() << "peer certificate chain:";
    auto const& peerCertificateChain = forSocket.peerCertificateChain();
    std::for_each(peerCertificateChain.cbegin(), peerCertificateChain.cend(), dumpCert);
}
