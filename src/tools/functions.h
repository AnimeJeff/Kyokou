#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <algorithm>
#include <array>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <locale>
#include <QString>
#include <QDebug>
#include <cryptopp/base64.h>
#include <cryptopp/des.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/md5.h>
#include <cryptopp/hex.h>
#include <CLI11/CLI11.hpp>
#include <iostream>
#include <filesystem>
#include <QException>
#define QS(str) QString::fromStdString(str)
//void print(const QString& str){
//    qDebug()<<str;
//}
//void print(const std::string& str){
//    print(QString::fromStdString (str));
//}

namespace Functions{
inline void httpsIfy(std::string& text) {
    if (text.substr(0, 2) == "//") {
        text = "https:" + text;
    }
}

inline bool containsSubstring(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

std::string urlDecode(const std::string &str);

std::string urlEncode(const std::string &str);

std::string findBetween(const std::string& str, const std::string& a, const std::string& b);

std::string substringAfter(const std::string& str, const std::string& delimiter);

std::string substringBefore(const std::string& str, const std::string& delimiter);

void replaceAll(std::string &s, const std::string &search,const std::string &replace);

std::string reverseString(const std::string& str);

std::string rc4(const std::string& key, const std::string& input);

std::string rot13(std::string n);

std::string base64Encode(const std::string& t,const std::string& baseTable="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

std::string base64Decode(const std::string& input,const std::string& baseTable =  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

std::vector<std::string> split(const std::string& str, char delimiter);

std::vector<std::string> split(const std::string& str, const std::string& delimiter);

std::string MD5(const std::string& str);

};


#endif // FUNCTIONS_H
