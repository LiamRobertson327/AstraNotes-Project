#include "EncryptionService.h"

#include <QCoreApplication>
#include <QDir>

#include <QByteArray>
#include <QDebug>
#include <QString>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>
#include <openssl/params.h>
#include <openssl/kdf.h>
#include <openssl/provider.h>
#include <openssl/core_names.h>

// Some older OpenSSL 3.x package headers (commonly on Linux distros) may
// not define newer ARGON2-specific OSSL_KDF_PARAM_* macros. Provide
// fallbacks to the expected parameter name strings so the code compiles
// and works against older OpenSSL 3.x releases.
#ifndef OSSL_KDF_PARAM_ARGON2_MEMCOST
#define OSSL_KDF_PARAM_ARGON2_MEMCOST "argon2_memcost"
#endif
#ifndef OSSL_KDF_PARAM_ARGON2_LANES
#define OSSL_KDF_PARAM_ARGON2_LANES "argon2_lanes"
#endif
#ifndef OSSL_KDF_PARAM_THREADS
#define OSSL_KDF_PARAM_THREADS "threads"
#endif


namespace {
constexpr int kKeySize = 32;      // AES-256
constexpr int kIvSize = 12;       // 96-bit IV for GCM
constexpr int kSaltSize = 16;     // 128-bit salt for Argon2id
constexpr int kTagSize = 16;      // 128-bit authentication tag

// Argon2id parameters
constexpr int kArgon2idMemory = 64 * 1024;;      // Memory cost in KiB (64 MiB)
constexpr int kArgon2idIterations = 3;      // Time cost
constexpr int kArgon2idParallelism = 1;     // Threads/lanes

QString toErrorString(const QString &fallback) {
    if (!fallback.isEmpty()) {
        return fallback;
    }
    return "Incorrect password or corrupted encrypted data";
}

QByteArray randomBytes(int size) {
    QByteArray bytes(size, Qt::Uninitialized);
    if (RAND_bytes(reinterpret_cast<unsigned char*>(bytes.data()), size) <= 0) {
        return {};
    }
    return bytes;
}
} // namespace



bool EncryptionService::deriveKey(const QString &password,
                                  const QByteArray &salt,
                                  QByteArray *key,
                                  QString *errorMessage) {
    if (!key) return false;
    key->resize(kKeySize);

    const QByteArray passwordBytes = password.toUtf8();

    unsigned int memcost = static_cast<unsigned int>(kArgon2idMemory);
    unsigned int iterations = static_cast<unsigned int>(kArgon2idIterations);
    unsigned int threads = static_cast<unsigned int>(kArgon2idParallelism);
    unsigned int lanes = static_cast<unsigned int>(kArgon2idParallelism);
    unsigned int key_len = static_cast<unsigned int>(kKeySize);

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    // 1. Portable Path Discovery
    QString appDir = QCoreApplication::applicationDirPath();
    QString modulePath = QDir::toNativeSeparators(appDir + "/ossl-modules");

    // 2. Initialize OpenSSL Provider
    OSSL_PROVIDER_set_default_search_path(nullptr, modulePath.toUtf8().constData());
    OSSL_PROVIDER *deflt = OSSL_PROVIDER_try_load(nullptr, "default", 1);
    if(!deflt){
        deflt = OSSL_PROVIDER_load(nullptr, "default");
    }
    
    EVP_KDF *kdf = EVP_KDF_fetch(nullptr, "ARGON2ID", nullptr);
    if (!kdf) {
        if (errorMessage) *errorMessage = "Argon2id module not found at: " + modulePath;
        if (deflt) OSSL_PROVIDER_unload(deflt);
        return false;
    }

    EVP_KDF_CTX *kdf_ctx = EVP_KDF_CTX_new(kdf);
    EVP_KDF_free(kdf); 


    // Build params using the core names (OSSL_KDF_PARAM_*) and construct_uint32
    OSSL_PARAM params[8];
    params[0] = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_PASSWORD,
                                                  const_cast<char*>(passwordBytes.constData()),
                                                  static_cast<size_t>(passwordBytes.size()));
    params[1] = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_SALT,
                                                  const_cast<char*>(salt.constData()),
                                                  static_cast<size_t>(salt.size()));
    params[2] = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_MEMCOST, &memcost);
    params[3] = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ITER, &iterations);
    params[4] = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_THREADS, &threads);
    params[5] = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_LANES, &lanes);
    params[6] = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_SIZE, &key_len);
    params[7] = OSSL_PARAM_construct_end();

    int res = EVP_KDF_derive(kdf_ctx,
                             reinterpret_cast<unsigned char*>(key->data()),
                             key->size(),
                             params);

    // collect OpenSSL errors (if any) for better diagnostics
    if (res <= 0) {
        unsigned long err = 0;
        QString opensslErrs;
        while ((err = ERR_get_error()) != 0) {
            char buf[256] = {0};
            ERR_error_string_n(err, buf, sizeof(buf));
            if (!opensslErrs.isEmpty()) opensslErrs += "; ";
            opensslErrs += QString::fromLatin1(buf);
        }
        if (errorMessage) {
            if (!opensslErrs.isEmpty()) {
                *errorMessage = QStringLiteral("Key derivation failed: %1").arg(opensslErrs);
            } else {
                *errorMessage = "Key derivation failed.";
            }
        }
    }

    EVP_KDF_CTX_free(kdf_ctx);
    if (deflt) OSSL_PROVIDER_unload(deflt);

    return (res > 0);
#else
    // Fallback logic for older OpenSSL if necessary
    return false; 
#endif
}

#include <QMessageBox>
EncryptionService::EncryptedPayload EncryptionService::encrypt(const QString &plaintext,
                                                               const QString &password) {
    EncryptedPayload payload;
    const QByteArray salt = randomBytes(kSaltSize);
    const QByteArray iv = randomBytes(kIvSize);
    if (salt.isEmpty() || iv.isEmpty()) {
        qWarning() << "[EncryptionService::encrypt] Failed to generate random salt or IV";
        return payload;
    }

    QByteArray key;
    QString errorMessage;
    if (!deriveKey(password, salt, &key, &errorMessage)) {
        qWarning() << "[EncryptionService::encrypt]" << errorMessage;
        return payload;
    }

    // Create cipher context for AES-256-GCM
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qWarning() << "[EncryptionService::encrypt] Failed to create cipher context";
        return payload;
    }

    bool cipher_fetched = false;
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_CIPHER *cipher = EVP_CIPHER_fetch(nullptr, "AES-256-GCM", nullptr);
    cipher_fetched = true;
    if (!cipher) {
        qWarning() << "[EncryptionService::encrypt] AES-256-GCM cipher not available (fetch)";
        EVP_CIPHER_CTX_free(ctx);
        return payload;
    }
#else
    EVP_CIPHER *cipher = const_cast<EVP_CIPHER*>(EVP_aes_256_gcm());
#endif

    const QByteArray plaintextBytes = plaintext.toUtf8();
    QByteArray ciphertext(plaintextBytes.size() + kTagSize, Qt::Uninitialized);
    int len = 0;
    int ciphertext_len = 0;

    // Initialize encryption
    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, reinterpret_cast<const unsigned char*>(key.constData()),
                           reinterpret_cast<const unsigned char*>(iv.constData())) <= 0) {
        qWarning() << "[EncryptionService::encrypt] Failed to initialize encryption";
        EVP_CIPHER_free(cipher);
        EVP_CIPHER_CTX_free(ctx);
        return payload;
    }

    // Encrypt
    if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(ciphertext.data()), &len,
                          reinterpret_cast<const unsigned char*>(plaintextBytes.constData()),
                          plaintextBytes.size()) <= 0) {
        qWarning() << "[EncryptionService::encrypt] Encryption failed";
        EVP_CIPHER_free(cipher);
        EVP_CIPHER_CTX_free(ctx);
        return payload;
    }
    ciphertext_len = len;

    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(ciphertext.data()) + len, &len) <= 0) {
        qWarning() << "[EncryptionService::encrypt] Encryption finalization failed";
        EVP_CIPHER_free(cipher);
        EVP_CIPHER_CTX_free(ctx);
        return payload;
    }
    ciphertext_len += len;

    // Get authentication tag
    QByteArray tag(kTagSize, Qt::Uninitialized);
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, kTagSize,
                            reinterpret_cast<unsigned char*>(tag.data())) <= 0) {
        qWarning() << "[EncryptionService::encrypt] Failed to get authentication tag";
        EVP_CIPHER_free(cipher);
        EVP_CIPHER_CTX_free(ctx);
        return payload;
    }

    if (cipher_fetched) EVP_CIPHER_free(cipher);
    EVP_CIPHER_CTX_free(ctx);

    ciphertext.truncate(ciphertext_len);
    payload.ciphertextBase64 = QString::fromLatin1(ciphertext.toBase64());
    payload.saltBase64 = QString::fromLatin1(salt.toBase64());
    payload.ivBase64 = QString::fromLatin1(iv.toBase64());
    payload.tagBase64 = QString::fromLatin1(tag.toBase64());

    return payload;
}


EncryptionService::DecryptResult EncryptionService::decrypt(const QString &ciphertextBase64,
                                                            const QString &password,
                                                            const QString &saltBase64,
                                                            const QString &ivBase64,
                                                            const QString &tagBase64) {
    DecryptResult result;
    const QByteArray ciphertext = QByteArray::fromBase64(ciphertextBase64.toLatin1(), QByteArray::Base64Encoding);
    const QByteArray salt = QByteArray::fromBase64(saltBase64.toLatin1(), QByteArray::Base64Encoding);
    const QByteArray iv = QByteArray::fromBase64(ivBase64.toLatin1(), QByteArray::Base64Encoding);
    const QByteArray tag = QByteArray::fromBase64(tagBase64.toLatin1(), QByteArray::Base64Encoding);

    if (ciphertext.isEmpty() || salt.isEmpty() || iv.isEmpty() || tag.isEmpty()) {
        result.errorMessage = "Missing encryption data";
        return result;
    }

    QByteArray key;
    QString errorMessage;
    if (!deriveKey(password, salt, &key, &errorMessage)) {
        result.errorMessage = errorMessage;
        return result;
    }

    // Create cipher context for AES-256-GCM
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        result.errorMessage = "Failed to create cipher context";
        return result;
    }

    bool cipher_fetched = false;
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    EVP_CIPHER *cipher = EVP_CIPHER_fetch(nullptr, "AES-256-GCM", nullptr);
    cipher_fetched = true;
    if (!cipher) {
        result.errorMessage = "AES-256-GCM cipher not available (fetch)";
        EVP_CIPHER_CTX_free(ctx);
        return result;
    }
#else
    EVP_CIPHER *cipher = const_cast<EVP_CIPHER*>(EVP_aes_256_gcm());
#endif

    QByteArray plaintext(ciphertext.size(), Qt::Uninitialized);
    int len = 0;
    int plaintext_len = 0;

    // Initialize decryption
    if (EVP_DecryptInit_ex(ctx, cipher, nullptr, reinterpret_cast<const unsigned char*>(key.constData()),
                           reinterpret_cast<const unsigned char*>(iv.constData())) <= 0) {
        result.errorMessage = toErrorString("Failed to initialize decryption");
        EVP_CIPHER_free(cipher);
        EVP_CIPHER_CTX_free(ctx);
        return result;
    }

    // Set authentication tag
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, kTagSize,
                            const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(tag.constData()))) <= 0) {
        result.errorMessage = toErrorString("Invalid authentication tag");
        EVP_CIPHER_free(cipher);
        EVP_CIPHER_CTX_free(ctx);
        return result;
    }

    // Decrypt
    if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(plaintext.data()), &len,
                          reinterpret_cast<const unsigned char*>(ciphertext.constData()),
                          ciphertext.size()) <= 0) {
        result.errorMessage = toErrorString("Decryption failed");
        EVP_CIPHER_free(cipher);
        EVP_CIPHER_CTX_free(ctx);
        return result;
    }
    plaintext_len = len;

    // Finalize decryption - this verifies the authentication tag
    if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(plaintext.data()) + len, &len) <= 0) {
        result.errorMessage = toErrorString("Authentication tag verification failed - wrong password or corrupted data");
        EVP_CIPHER_free(cipher);
        EVP_CIPHER_CTX_free(ctx);
        return result;
    }
    plaintext_len += len;

    if (cipher_fetched) EVP_CIPHER_free(cipher);
    EVP_CIPHER_CTX_free(ctx);

    plaintext.truncate(plaintext_len);
    result.success = true;
    result.plaintext = QString::fromUtf8(plaintext);
    return result;
}
