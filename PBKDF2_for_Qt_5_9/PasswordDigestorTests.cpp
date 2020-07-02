#include "PasswordDigestorTests.h"

#include "BackportedQPasswordDigestor.h"

#include <QtTest/QtTest>


namespace
{
QByteArray
getPBKDF2For(const QByteArray& data, const QByteArray& salt, int iterations, quint64 dkLen)
{
    return BackportedQt::deriveKeyPbkdf2(QCryptographicHash::Sha256, data, salt, iterations, dkLen);
}
} // namespace


void PasswordDigestorTests::test_PBKDF2_samples()
{
    // sample data generated for https://neurotechnics.com/tools/pbkdf2-test
    // note: following "Identity & Data Security for Web Development" (978-1-491-93701-3) the salt
    // shall be the same length as the output of the hash function used - in our case SHA-256, so we
    // use 32 bytes
    QCOMPARE(getPBKDF2For("password",
                          "fbb2bbd3c246321fe451f1bb541fd8e443fc3de78b3b15398bd83e86624cdec5",
                          4096,
                          32)
                     .toHex(),
             QByteArray {"779646390eeecef780ed156083f7b18c36f903543a50bcce8cca5a5c0b3b7b07"});

    QCOMPARE(getPBKDF2For("password",
                          "c7561cd86929e93ce1e5b69f5bab95a6e374d4b25f9573ce696cd5ab92642549",
                          8192,
                          32)
                     .toHex(),
             QByteArray {"3b4c7f2c18a1ecfa8319bf32db3dfebddc6e6ef13e7a2cc424990f669da8ae80"});
}


QTEST_MAIN(PasswordDigestorTests)
