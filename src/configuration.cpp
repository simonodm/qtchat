#include "configuration.h"
#include "utils.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

#include <QDir>

Configuration::Configuration(const std::string &publicKeyFile, const std::string &privateKeyFile, const UserInfo &userInfo, int port, const KeyCombination &keys)
    : publicKeyFile(publicKeyFile),
      privateKeyFile(privateKeyFile),
      userInfo(userInfo),
      port(port),
      keys(keys) {}

Configuration::Configuration(const std::string &publicKeyFile, const std::string &privateKeyFile, const UserInfo &userInfo, int port)
    : Configuration(publicKeyFile, privateKeyFile, userInfo, port, getKeys(publicKeyFile, privateKeyFile)) {}

void Configuration::saveToFile(const std::string &path) const {
    Utils::createPath(path);

    std::ofstream fileStream;
    fileStream.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    fileStream.open(path);

    fileStream << "public_key_path: " << publicKeyFile << std::endl;
    fileStream << "private_key_path: " << privateKeyFile << std::endl;
    fileStream << "username: " << userInfo.getUsername() << std::endl;
    fileStream << "port: " << port << std::endl;

    fileStream.close();
}

Configuration Configuration::defaultConfiguration() {
    auto privateKeyFile = getDefaultConfigDirectory() + "private_key.pem";
    auto publicKeyFile = getDefaultConfigDirectory() + "public_key.pem";
    auto userInfo = UserInfo("New User");
    auto port = 8100;
    auto keys = getKeys(publicKeyFile, privateKeyFile);

    return Configuration(publicKeyFile, privateKeyFile, userInfo, port, keys);
}

std::string Configuration::getDefaultConfigPath() {
    return getDefaultConfigDirectory() + "config.ini";
}

Configuration Configuration::loadFromFile(const std::string &path) {
    auto parameters = loadConfigFile(path);

    auto publicKeyFile = parameters["public_key_path"];
    auto privateKeyFile = parameters["private_key_path"];
    auto userInfo = UserInfo(parameters["username"]);
    auto port = std::stoi(parameters["port"]);
    auto keys = getKeys(publicKeyFile, privateKeyFile);

    return Configuration(publicKeyFile, privateKeyFile, userInfo, port, keys);
}

std::string Configuration::getDefaultConfigDirectory() {
    return QDir::homePath().toStdString() + "/qtchat/";
}

std::unordered_map<std::string, std::string> Configuration::loadConfigFile(const std::string &path) {
    std::ifstream fileStream;
    fileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fileStream.open(path);

    std::unordered_map<std::string, std::string> parameters;
    std::string line;

    // EOF has to be detected before getting line due to badbit being set
    while(fileStream.peek() != EOF && std::getline(fileStream, line)) {
        auto tokens = Utils::split(line, ": ");
        if(tokens.size() == 2) {
            parameters.insert(std::make_pair(Utils::trim(tokens[0]), Utils::trim(tokens[1])));
        }
    }

    return parameters;
}

KeyCombination Configuration::getKeys(const std::string &publicKeyFile, const std::string &privateKeyFile) {
    try {
        return loadKeysFromFiles(publicKeyFile, privateKeyFile);
    }
    catch (...) {
        auto keys = RSAKeyGenerator::generateKey(4096);
        saveKeys(keys, publicKeyFile, privateKeyFile);
        return keys;
    }
}

KeyCombination Configuration::loadKeysFromFiles(const std::string &publicKeyFile, const std::string &privateKeyFile) {
    auto publicKeyStr = Utils::loadFile(publicKeyFile);
    std::shared_ptr<RSAPublicKey> publicKey(RSAPublicKey::decodeFromPEM(publicKeyStr));

    auto privateKeyStr = Utils::loadFile(privateKeyFile);
    std::shared_ptr<RSAPrivateKey> privateKey(RSAPrivateKey::decodeFromPEM(privateKeyStr));

    return KeyCombination(publicKey, privateKey);
}

void Configuration::saveKeys(const KeyCombination &keyCombination, const std::string &publicKeyFile, const std::string &privateKeyFile) {
    auto publicKeyStr = keyCombination.getPublicKey()->encode();
    auto privateKeyStr = keyCombination.getPrivateKey()->encode();
    Utils::saveFile(publicKeyFile, publicKeyStr);
    Utils::saveFile(privateKeyFile, privateKeyStr);
}
