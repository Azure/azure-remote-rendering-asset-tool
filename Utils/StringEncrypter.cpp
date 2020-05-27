#include <Utils/StringEncrypter.h>

#include <windows.h>

#include <Wincrypt.h>

namespace StringEncrypter
{
    bool encrypt(const QString& plainString, QString& encryptedOutput)
    {
        auto ascii = plainString.toLatin1();
        DATA_BLOB plainData = {(DWORD)ascii.size(), (BYTE*)ascii.data()};
        DATA_BLOB encryptedData;

        if (CryptProtectData(
                &plainData,
                NULL,
                NULL,
                NULL,
                NULL,
                0,
                &encryptedData))
        {
            // convert the byte array to a base 64 string
            encryptedOutput = QString::fromLatin1(QByteArray((const char*)encryptedData.pbData, encryptedData.cbData).toBase64());
            LocalFree(encryptedData.pbData);
            return true;
        }
        else
        {
            return false;
        }
    }

    bool decrypt(const QString& encryptedString, QString& plainOutput)
    {
        // decode encryptedString as base64
        QByteArray encryptedArray = QByteArray::fromBase64(encryptedString.toLatin1());
        DATA_BLOB encryptedData = {(DWORD)encryptedArray.size(), (BYTE*)encryptedArray.data()};
        DATA_BLOB decryptedData;

        if (CryptUnprotectData(
                &encryptedData,
                NULL,
                NULL, // Optional entropy
                NULL, // Reserved
                NULL, // Optional PromptStruct
                0,
                &decryptedData))
        {
            plainOutput = QString::fromLatin1((const char*)decryptedData.pbData, decryptedData.cbData);
            LocalFree(decryptedData.pbData);
            return true;
        }
        else
        {
            return false;
        }
    }
} // namespace StringEncrypter