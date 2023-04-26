#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "encryption.h"

#include <string>
#include <unordered_map>

/**
 * @brief Represents user's personal information.
 */
class UserInfo {
public:
    UserInfo() : username_("Unknown") {}
    UserInfo(const std::string &username) : username_(username) {}
    std::string getUsername() const { return username_; }
private:
    std::string username_;
};

/**
 * @brief Represents application configuration and provides functionality to load and save it.
 */
struct Configuration {
public:
    Configuration(const std::string &publicKeyFile, const std::string &privateKeyFile, const UserInfo &userInfo, int port, const KeyCombination &keys);
    Configuration(const std::string &publicKeyFile, const std::string &privateKeyFile, const UserInfo &userInfo, int port);
    void saveToFile(const std::string &path) const;
    static Configuration defaultConfiguration();
    static Configuration loadFromFile(const std::string &path);
    static std::string getDefaultConfigPath();

    std::string privateKeyFile;
    std::string publicKeyFile;
    KeyCombination keys;
    UserInfo userInfo;
    int port;

private:
    static std::string getDefaultConfigDirectory();
    static std::unordered_map<std::string, std::string> loadConfigFile(const std::string &path);

    static KeyCombination getKeys(const std::string &publicKeyFile, const std::string &privateKeyFile);
    static KeyCombination loadKeysFromFiles(const std::string &publicKeyFile, const std::string &privateKeyFile);
    static void saveKeys(const KeyCombination &keyCombination, const std::string &publicKeyFile, const std::string &privateKeyFile);
};

#endif // CONFIGURATION_H
