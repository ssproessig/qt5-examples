#include "BackportedQPasswordDigestor.h"

#include <QDebug>
#include <QMessageAuthenticationCode>
#include <QtEndian>


namespace
{
// back-ported from:
//   https://code.woboq.org/qt5/qtbase/src/corelib/tools/qcryptographichash.cpp.html#_ZN18QCryptographicHash10hashLengthENS_9AlgorithmE
// some replacement values from:
//   https://code.woboq.org/qt5/qtbase/src/3rdparty/rfc6234/sha.h.html#SHA384HashSize
int QCryptographicHash__hashLength(QCryptographicHash::Algorithm method)
{
    switch (method)
    {
        case QCryptographicHash::Sha1:
            return 20;
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
        case QCryptographicHash::Md4:
            return 16;
        case QCryptographicHash::Md5:
            return 16;
        case QCryptographicHash::Sha224:
            return 28;
        case QCryptographicHash::Sha256:
            return 32;
        case QCryptographicHash::Sha384:
            return 48;
        case QCryptographicHash::Sha512:
            return 64;
        case QCryptographicHash::RealSha3_224:
        case QCryptographicHash::Keccak_224:
            return 224 / 8;
        case QCryptographicHash::RealSha3_256:
        case QCryptographicHash::Keccak_256:
            return 256 / 8;
        case QCryptographicHash::RealSha3_384:
        case QCryptographicHash::Keccak_384:
            return 384 / 8;
        case QCryptographicHash::RealSha3_512:
        case QCryptographicHash::Keccak_512:
            return 512 / 8;
#endif
    }
    return 0;
}
} // namespace


// back-ported from:
//    https://code.woboq.org/qt5/qtbase/src/network/ssl/qpassworddigestor.cpp.html#142
// and patched to use local QCryptographicHash__hashLength()
QByteArray BackportedQt::deriveKeyPbkdf2(QCryptographicHash::Algorithm algorithm,
                                         const QByteArray& password,
                                         const QByteArray& salt,
                                         int iterations,
                                         quint64 dkLen)
{ // https://tools.ietf.org/html/rfc8018#section-5.2
    // The RFC recommends checking that 'dkLen' is not greater than '(2^32 - 1) * hLen'
    int const hashLen = QCryptographicHash__hashLength(algorithm);
    const quint64 maxLen = quint64(std::numeric_limits<quint32>::max() - 1) * hashLen;
    if (dkLen > maxLen)
    {
        qWarning().nospace() << "Derived key too long:\n"
                             << algorithm << " was chosen which produces output of length "
                             << maxLen << " but " << dkLen << " was requested.";
        return QByteArray();
    }
    if (iterations < 1 || dkLen < 1)
        return QByteArray();
    QByteArray key;
    quint32 currentIteration = 1;
    QMessageAuthenticationCode hmac(algorithm, password);
    QByteArray index(4, Qt::Uninitialized);
    while (quint64(key.length()) < dkLen)
    {
        hmac.addData(salt);
        qToBigEndian(currentIteration, index.data());
        hmac.addData(index);
        QByteArray u = hmac.result();
        hmac.reset();
        QByteArray tkey = u;
        for (int iter = 1; iter < iterations; iter++)
        {
            hmac.addData(u);
            u = hmac.result();
            hmac.reset();
            std::transform(
                    tkey.cbegin(), tkey.cend(), u.cbegin(), tkey.begin(), std::bit_xor<char>());
        }
        key += tkey;
        currentIteration++;
    }
    return key.left(dkLen);
}
