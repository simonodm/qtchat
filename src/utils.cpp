#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <filesystem>

#include "utils.h"

namespace fs = std::filesystem;

std::string Utils::convertToHex(int num, int digits) {
    std::stringstream ss;
    ss << std::hex << std::setw(digits) << std::setfill('0') << num;
    return ss.str();
}

std::string Utils::loadFile(const std::string &path) {
    std::ifstream fileStream;
    fileStream.open(path);

    std::stringstream stream;
    stream << fileStream.rdbuf();
    auto result = stream.str();

    fileStream.close();

    return result;
}

void Utils::saveFile(const std::string &path, const std::string &content) {
    std::ofstream fileStream;
    fileStream.open(path);

    fileStream << content;

    fileStream.close();
}

std::string Utils::trim(std::string str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char c) { return !std::isspace(c); }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), str.end());
    return str;
}

std::vector<std::string> Utils::split(const std::string& str, const std::string delimiter)
{
    auto start = 0U;
    auto end = str.find(delimiter);
    std::vector<std::string> words;
    while(end != std::string::npos)
    {
        words.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }

    words.push_back(str.substr(start, str.length() - start));

    return words;
}

std::string Utils::concat(const std::vector<std::string>::const_iterator &start, const std::vector<std::string>::const_iterator &end, std::string delimiter) {
    auto curr = start;
    std::stringstream iss;
    while(curr != end) {
        iss << *curr << delimiter;
        ++curr;
    }

    return iss.str();
}

void Utils::createPath(const std::string &path) {
    auto tokens = Utils::split(path, "/");
    auto directories = Utils::concat(tokens.cbegin(), --tokens.cend(), "/");

    if(!fs::exists(directories)) {
        fs::create_directories(directories);
    }
}
