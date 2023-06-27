#include "nivod.h"

std::string Nivod::createSign(const std::map<std::string, std::string> &bodyMap,
                              const std::string &secretKey) {
    std::stringstream ss;
    std::string signQuery = _QUERY_PREFIX;
    for (auto const &[key, value] : queryMap) {
        signQuery += key + "=" + value + "&";
    }
    
    ss << _BODY_PREFIX;
    for (const auto &[key, value] : bodyMap) {
        ss << key << '=' << value << '&';
    }
    
    std::string input = signQuery + ss.str() + _SECRET_PREFIX + secretKey;
    return Functions::MD5(input);
}

std::string Nivod::decryptedByDES(const std::string &input){
    std::string key = "diao.com";
    std::vector<byte> keyBytes(key.begin(), key.end());
    std::vector<byte> inputBytes;
    for (size_t i = 0; i < input.length(); i += 2) {
        byte byte = static_cast<unsigned char>(std::stoi(input.substr(i, 2), nullptr, 16));
        inputBytes.push_back(byte);
    }

    size_t length = inputBytes.size();
    size_t padding = length % 8 == 0 ? 0 : 8 - length % 8;
    inputBytes.insert(inputBytes.end(), padding, 0);

    std::vector<byte> outputBytes(length + padding, 0);
    CryptoPP::ECB_Mode<CryptoPP::DES>::Decryption decryption(keyBytes.data(), keyBytes.size());
    CryptoPP::ArraySink sink(outputBytes.data(), outputBytes.size());
    CryptoPP::ArraySource source(inputBytes.data(), inputBytes.size(), true, new CryptoPP::StreamTransformationFilter(decryption, new CryptoPP::Redirector(sink)));
    std::string decrypted(outputBytes.begin(), outputBytes.end());
    size_t pos = decrypted.find_last_of('}');
    if (pos != std::string::npos) {
        decrypted = decrypted.substr(0, pos + 1);
    }
    return decrypted;
}
