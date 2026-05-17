#ifndef ENCRYPTIONSERVICE_H
#define ENCRYPTIONSERVICE_H

#include <QByteArray>
#include <QString>

class EncryptionService {
public:
    struct EncryptedPayload {
        QString ciphertextBase64;
        QString saltBase64;
        QString ivBase64;
        QString tagBase64;
    };

    struct DecryptResult {
        bool success = false;
        QString plaintext;
        QString errorMessage;
    };

    static EncryptedPayload encrypt(const QString &plaintext, const QString &password);
    static DecryptResult decrypt(const QString &ciphertextBase64,
                                 const QString &password,
                                 const QString &saltBase64,
                                 const QString &ivBase64,
                                 const QString &tagBase64);

private:
    static bool deriveKey(const QString &password,
                          const QByteArray &salt,
                          QByteArray *key,
                          QString *errorMessage);
};

#endif // ENCRYPTIONSERVICE_H
