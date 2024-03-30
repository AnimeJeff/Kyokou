#ifndef GOGOCDN_H
#define GOGOCDN_H

#include "videoextractor.cpp"

#include <QCryptographicHash>
#include <QByteArray>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <iostream>
#include <string>

#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonArray>

#include <cryptopp/cryptlib.h>
#include <cryptopp/aes.h>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>

#include <regex>

class GogoCDN
{
private:

    struct Keys {
        std::string key;
        std::string secondKey;
        std::string iv;
        //        unsigned char key[33];
        //        unsigned char secondKey[33];
        //        unsigned char iv[17];
    };
    Keys keysAndIv
        {
            "37911490979715163134003223491201",
            "54674138327930866480207815084989",
            "3134003223491201"
        };
public:
    GogoCDN(){};
    QString extract(std::string link)
    {
        auto response = NetworkClient::get(link);
        if (Functions::containsSubstring (link,"streaming.php"))
        {
            std::string it = response.document().selectFirst("//script[@data-name='episode']").attr("data-value").as_string ();
            //qDebug() << "it: " << it.c_str ();
            std::string decrypted = decrypt (it, keysAndIv.key, keysAndIv.iv);
            //qDebug() << "decrypted: " << decrypted.c_str ();
            decrypted.erase(std::remove(decrypted.begin(), decrypted.end(), '\t'), decrypted.end());  // Remove all occurrences of '\t'
            std::string id = Functions::findBetween(decrypted, "", "&");
            std::string end = Functions::substringAfter(decrypted, id);

            std::string encryptedId = encrypt(id, keysAndIv.key, keysAndIv.iv);
            //qDebug() << "encryptedId: " << encryptedId.c_str ();
            std::string encryptedUrl = "https://" + getHostFromUrl(link)+ "/encrypt-ajax.php?id=" + encryptedId + end + "&alias=" + id;
            //qDebug() << "encryptedUrl: " << encryptedUrl.c_str ();
            std::string encrypted = NetworkClient::post(encryptedUrl, {{"X-Requested-With", "XMLHttpRequest"}}).body;
            //qDebug() << "encrypted: " << encrypted.c_str ();
            std::string dataEncrypted = Functions::findBetween(encrypted, "{\"data\":\"", "\"}");
            //qDebug() << "dataEncrypted: " << dataEncrypted.c_str ();
            std::string jumbledJsonString = decrypt(dataEncrypted, keysAndIv.secondKey, keysAndIv.iv);
            //qDebug() << "jumbledJson: " << jumbledJson.c_str ();
            Functions::replaceAll(jumbledJsonString,"o\"<P{#meme=\"\"","{\"e\":[{\"file\":\"");

            QJsonObject sourceJson = QJsonDocument::fromJson(jumbledJsonString.c_str()).object();

            auto source = sourceJson["source"].toArray ()[0].toObject ()["file"].toString ();
            auto source_bk = sourceJson["source_bk"].toArray ()[0].toObject ()["file"].toString ();
            return source;
        }
        return "";
    }
    static std::string getHostFromUrl(const std::string& url){
        std::regex regex("^(?:https?://)?(?:www\\.)?([^:/\\s]+)");
        std::smatch match;
        if (std::regex_search(url, match, regex)){
            return match[1];
        }
        return "";
    }
    static std::string encrypt(const std::string& str, const std::string& key, const std::string& iv)
    {
        CryptoPP::AES::Encryption aesKey(reinterpret_cast<const unsigned char*>(key.data()), key.size());
        CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryptor(aesKey, reinterpret_cast<const unsigned char*>(iv.data()));

        std::string ciphertext;
        CryptoPP::StringSource(str, true,
                               new CryptoPP::StreamTransformationFilter(cbcEncryptor,
                                                                        new CryptoPP::Base64Encoder(
                                                                            new CryptoPP::StringSink(ciphertext), false
                                                                            ), CryptoPP::BlockPaddingSchemeDef::PKCS_PADDING
                                                                        )
                               );
        return ciphertext;
    }

    static std::string decrypt(const std::string& ciphertext, const std::string& key, const std::string& iv)
    {
        std::string plaintext;
        try {
            CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryption(reinterpret_cast<const byte*>(key.data()), key.size(),
                                                                     reinterpret_cast<const byte*>(iv.data()));
            CryptoPP::StringSource(ciphertext, true,
                                   new CryptoPP::Base64Decoder(
                                       new CryptoPP::StreamTransformationFilter(decryption,
                                                                                new CryptoPP::StringSink(plaintext))));
        }
        catch (const CryptoPP::Exception& e){
            std::cerr << e.what() << std::endl;
            return "";
        }
        return plaintext;
    }


public:

};



#endif // GOGOCDN_H
