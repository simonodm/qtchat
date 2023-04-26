#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include "aes.h"
#include "modes.h"
#include <QString>

#include <lib/cryptopp/rsa.h>
#include <lib/cryptopp/pem.h>

/**
 * @brief An abstract class for a key which can be encoded to string.
 */
class Key {
public:
    virtual std::string encode() const = 0;
};

/**
 * @brief An abstract class for a key which can encrypt text.
 */
class EncryptingKey : public Key {
public:
    virtual std::string encrypt(const std::string &message) = 0;
};

/**
 * @brief An abstract class for a key which can decrypt text.
 */
class DecryptingKey : public Key {
public:
    virtual std::string decrypt(const std::string &message) = 0;
};

/**
 * @brief Represents an RSA public key. Can encrypt messages.
 */
struct RSAPublicKey : public EncryptingKey {
    RSAPublicKey(long e, long n);
    RSAPublicKey(CryptoPP::RSA::PublicKey &key) : publicKey_(key) {}
    std::string encrypt(const std::string &message) override;
    std::string encode() const override;

    /**
     * @brief Decodes the public key from its PEM representation
     * @param key PEM representation of the key
     * @return A pointer to the decoded public key
     */
    static RSAPublicKey* decodeFromPEM(const std::string &key);

private:
    CryptoPP::RSA::PublicKey publicKey_;
};

/**
 * @brief Represents RSA private key. Can decrypt messages.
 */
struct RSAPrivateKey : public DecryptingKey {
    RSAPrivateKey(long e, long n, long d);
    RSAPrivateKey(CryptoPP::RSA::PrivateKey &key) : privateKey_(key) {}
    std::string decrypt(const std::string &message) override;
    std::string encode() const override;

    /**
     * @brief Decodes the private key from its PEM representation
     * @param key PEM representation of the key
     * @return A pointer to the decoded private key
     */
    static RSAPrivateKey* decodeFromPEM(const std::string &key);

private:
    CryptoPP::RSA::PrivateKey privateKey_;
};

/**
 * @brief Represents an AES key. Can encrypt and decrypt messages.
 */
struct AESKey : public EncryptingKey, public DecryptingKey {
public:
    AESKey();
    AESKey(const std::string &key);
    std::string encrypt(const std::string &message);
    std::string decrypt(const std::string &message);
    std::string encode() const;

private:
    std::string key_;
};

/**
 * @brief Represents an encrypting/decrypting key pair, e. g. public/private key pair.
 */
struct KeyCombination {
    KeyCombination(std::shared_ptr<EncryptingKey> publicKey, std::shared_ptr<DecryptingKey> privateKey) : publicKey_(publicKey), privateKey_(privateKey) {}
    std::shared_ptr<EncryptingKey> getPublicKey() const { return publicKey_; }
    std::shared_ptr<DecryptingKey> getPrivateKey() const { return privateKey_; }

private:
    std::shared_ptr<EncryptingKey> publicKey_;
    std::shared_ptr<DecryptingKey> privateKey_;
};

/**
 * @brief A static RSA public/private key pair generator.
 */
class RSAKeyGenerator {
public:
    static KeyCombination generateKey(unsigned int bitsize = 1024);
};

#endif // ENCRYPTION_H
