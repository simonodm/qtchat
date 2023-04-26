#include "encryption.h"

#include <lib/cryptopp/osrng.h>

using namespace CryptoPP;

RSAPublicKey::RSAPublicKey(long e, long n) {
    publicKey_.Initialize(n, e);
}

std::string RSAPublicKey::encrypt(const std::string &message) {
    RSAES_OAEP_SHA_Encryptor e(publicKey_);
    std::string cipher;
    AutoSeededRandomPool rng;

    StringSource ss(message, true,
        new PK_EncryptorFilter(rng, e, new StringSink(cipher))
    );

    return cipher;
}

std::string RSAPublicKey::encode() const {
    std::string result;
    StringSink ss(result);
    PEM_Save(ss, publicKey_);

    return result;
}

RSAPublicKey* RSAPublicKey::decodeFromPEM(const std::string &key) {
    RSA::PublicKey pk;
    StringSource ss(key, true);
    PEM_Load(ss, pk);

    auto result = new RSAPublicKey(pk);

    return result;
}

RSAPrivateKey::RSAPrivateKey(long e, long n, long d) {
    privateKey_.Initialize(n, e, d);
}

std::string RSAPrivateKey::decrypt(const std::string &message) {
    RSAES_OAEP_SHA_Decryptor d(privateKey_);
    std::string recovered;
    AutoSeededRandomPool rng;

    StringSource ss(message, true,
        new PK_DecryptorFilter(rng, d, new StringSink(recovered))
    );

    return recovered;
}

std::string RSAPrivateKey::encode() const {
    std::string result;
    StringSink ss(result);
    PEM_Save(ss, privateKey_);

    return result;
}

RSAPrivateKey* RSAPrivateKey::decodeFromPEM(const std::string &key) {
    RSA::PrivateKey privateKey;
    StringSource ss(key, true);
    PEM_Load(ss, privateKey);

    auto result = new RSAPrivateKey(privateKey);

    return result;
}

AESKey::AESKey() {
    AutoSeededRandomPool rng;
    CryptoPP::byte key[AES::DEFAULT_KEYLENGTH];
    rng.GenerateBlock(key, sizeof(key));
    for(int i = 0; i < sizeof(key); ++i) {
        key_ += key[i];
    }
}

AESKey::AESKey(const std::string &key) {
    key_ = key;
}

std::string AESKey::encrypt(const std::string &message) {
    ECB_Mode<AES>::Encryption encryptor;
    CryptoPP::byte keyBytes[key_.length()];
    std::copy(key_.begin(), key_.end(), keyBytes);

    encryptor.SetKey(keyBytes, key_.length());

    std::string encrypted;
    StringSource s(message, true,
        new StreamTransformationFilter(encryptor,
            new StringSink(encrypted)
        )
    );

    return encrypted;
}

std::string AESKey::decrypt(const std::string &message) {
    ECB_Mode<AES>::Decryption decryptor;
    CryptoPP::byte keyBytes[key_.length()];
    std::copy(key_.begin(), key_.end(), keyBytes);

    decryptor.SetKey(keyBytes, key_.length());

    std::string decrypted;
    StringSource s(message, true,
        new StreamTransformationFilter(decryptor,
            new StringSink(decrypted)
        )
    );

    return decrypted;
}

std::string AESKey::encode() const {
    return key_;
}

KeyCombination RSAKeyGenerator::generateKey(unsigned int bitsize) {
    AutoSeededRandomPool rng;
    InvertibleRSAFunction params;
    params.GenerateRandomWithKeySize(rng, bitsize);

    auto publicRsaKey = RSA::PublicKey(params);
    auto privateRsaKey = RSA::PrivateKey(params);

    auto publicKey = std::make_shared<RSAPublicKey>(publicRsaKey);
    auto privateKey = std::make_shared<RSAPrivateKey>(privateRsaKey);

    KeyCombination combination(publicKey, privateKey);

    return combination;
}
