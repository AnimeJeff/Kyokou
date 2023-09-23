#ifndef GOGOCDN_H
#define GOGOCDN_H

#include "videoextractor.cpp"

#include <QCryptographicHash>
#include <QByteArray>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <cstring>
#include <iostream>
#include <string>
#include <network/client.h>
#include <QRegularExpression>
#include <cryptopp/cryptlib.h>
#include <cryptopp/aes.h>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <tools/Qt-AES/qaesencryption.h>
#include "tools/functions.h"
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
    GogoCDN() {};
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
            std::string jumbledJson = decrypt(dataEncrypted, keysAndIv.secondKey, keysAndIv.iv);
            //qDebug() << "jumbledJson: " << jumbledJson.c_str ();
            Functions::replaceAll(jumbledJson,"o\"<P{#meme=\"\"","{\"e\":[{\"file\":\"");
            auto source = nlohmann::json::parse (jumbledJson)["source"][0]["file"].get <std::string>();
            auto source_bk = nlohmann::json::parse (jumbledJson)["source_bk"][0]["file"].get <std::string>();
            //qDebug()<<"source"<<QString::fromStdString (source)<<"\n";
            //            qDebug()<<"source_bk"<<QString::fromStdString (source_bk)<<"\n";

            return QString::fromStdString (source);
            //            qDebug()<<server->source<<"not my fault";
        }
        return "";
    }
    static std::string getHostFromUrl(const std::string& url) {
        std::regex regex("^(?:https?://)?(?:www\\.)?([^:/\\s]+)");
        std::smatch match;
        if (std::regex_search(url, match, regex)) {
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
        catch (const CryptoPP::Exception& e) {
            std::cerr << e.what() << std::endl;
            return "";
        }
        return plaintext;
    }

//        std::string encrypt(const std::string& plaintext,std::string key,std::string iv) {

//            EVP_CIPHER_CTX *ctx;
//            ctx = EVP_CIPHER_CTX_new();

//            const EVP_CIPHER* cipher = EVP_aes_256_cbc();

//            if(!EVP_EncryptInit_ex(ctx, cipher, NULL,
//                                    reinterpret_cast<const unsigned char*>(key.c_str()),
//                                    reinterpret_cast<const unsigned char*>(iv.c_str()))){
//                return "";
//            }

//            std::string ciphertext(plaintext.size() + AES_BLOCK_SIZE, '\0');

//            int len = 0;
//            if(!EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(&ciphertext[0]), &len,
//                                   reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.size())){
//                return "";
//            }

//            int ciphertext_len = len;

//            if(!EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&ciphertext[0])+len, &len)){
//                return "";
//            }

//            ciphertext_len += len;
//            ciphertext.resize(ciphertext_len);

//            EVP_CIPHER_CTX_free(ctx);

//            return Functions::base64Encode (ciphertext);
//        }

    //    std::string decrypt(std::string ciphertext, std::string key, std::string iv)
    //    {
    //        ciphertext = Functions::base64Decode (ciphertext);
    //        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    //        EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (const unsigned char*)key.c_str(), (const unsigned char*)iv.c_str());

    //        unsigned char plaintext[ciphertext.length()];
    //        int p_len, f_len;

    //        EVP_DecryptUpdate(ctx, plaintext, &p_len, (const unsigned char*)ciphertext.c_str(), ciphertext.length());
    //        EVP_DecryptFinal_ex(ctx, plaintext + p_len, &f_len);

    //        EVP_CIPHER_CTX_free(ctx);

    //        std::string decoded((const char*)plaintext, p_len + f_len);
    //        qDebug() << int(decoded[decoded.size ()-1]) << " : " << decoded[decoded.size ()-1];
    //        decoded.erase(decoded.find(decoded[decoded.size ()-1]), std::string::npos);
    //        return decoded;
    //    }

public:

};



#endif // GOGOCDN_H
