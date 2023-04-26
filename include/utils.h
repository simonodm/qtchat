#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <vector>
#include <string>

namespace Utils {
    std::string convertToHex(int num, int digits);
    void createPath(const std::string &path);
    std::string loadFile(const std::string &path);
    void saveFile(const std::string &path, const std::string &content);
    std::string trim(std::string str);
    std::vector<std::string> split(const std::string& str, const std::string delimiter);
    std::string concat(const std::vector<std::string>::const_iterator &start, const std::vector<std::string>::const_iterator &end, std::string delimiter);
}


#endif // STRING_UTILS_H
