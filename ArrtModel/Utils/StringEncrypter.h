#pragma once
#include <QString>

// utility namespace for encrypting and decrypting a string.
// Encryption is done using CryptProtectData, which encodes based on the local machine and the user.
// Decrypting is only possible from the same machine and user.
// Encrypted strings are mime64 encoded

namespace StringEncrypter
{
    bool encrypt(const QString& plainString, QString& encryptedOutput);
    bool decrypt(const QString& encryptedString, QString& plainOutput);
}; // namespace StringEncrypter