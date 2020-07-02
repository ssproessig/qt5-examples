#pragma once

#include <QByteArray>
#include <QCryptographicHash>


namespace BackportedQt
{
/**
 * Derive a key using the PBKDF2-algorithm as defined in
 * @l {https://tools.ietf.org/html/rfc8018#section-5.2} {RFC 8018}.
 * This function takes the @a data and @a salt, and then applies HMAC-X, where
 * the X is @a algorithm, repeatedly. It internally concatenates intermediate
 * results to the final output until at least @a dkLen amount of bytes have
 * been computed and it will execute HMAC-X @a iterations times each time a
 * concatenation is required. The total number of times it will execute HMAC-X
 * depends on @a iterations, @a dkLen and @a algorithm and can be calculated
 * as
 * @c{iterations * ceil(dkLen / QCryptographicHash::hashLength(algorithm))}.
 * @sa deriveKeyPbkdf1, QMessageAuthenticationCode, QCryptographicHash
 */
QByteArray deriveKeyPbkdf2(QCryptographicHash::Algorithm algorithm,
                           const QByteArray& password,
                           const QByteArray& salt,
                           int iterations,
                           quint64 dkLen);
} // namespace BackportedQt
