#ifndef GOGOCDN_H
#define GOGOCDN_H

#include "videoextractor.cpp"

#include <QCryptographicHash>
#include <QByteArray>
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

// #include <regex>
#include "Components/functions.h"

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
    QString extract(const QString &link)
    {
        if (link.contains ("streaming.php"))
        {
            auto response = NetworkClient::get(link);
            std::string it = response.document().selectFirst("//script[@data-name='episode']").node ().attribute("data-value").as_string ();
            //qDebug() << "it: " << it.c_str ();
            auto decrypted = QString::fromStdString (decrypt(it, keysAndIv.key, keysAndIv.iv));
            //qDebug() << "decrypted: " << decrypted.c_str ();
            decrypted.remove('\t');
            auto id = Functions::findBetween(decrypted, "", "&");
            auto end = Functions::substringAfter(decrypted, id);

            auto encryptedId = QString::fromStdString (encrypt(id.toStdString (), keysAndIv.key, keysAndIv.iv));
            //qDebug() << "encryptedId: " << encryptedId.c_str ();
            QString encryptedUrl = "https://" + Functions::getHostFromUrl(link)
                                   + "/encrypt-ajax.php?id=" + encryptedId + end + "&alias=" + id;
            //qDebug() << "encryptedUrl: " << encryptedUrl.c_str ();
            std::string encrypted = NetworkClient::post(encryptedUrl, {{"X-Requested-With", "XMLHttpRequest"}}).body;
            //qDebug() << "encrypted: " << encrypted.c_str ();
            QString dataEncrypted = Functions::findBetween(QString::fromStdString (encrypted), "{\"data\":\"", "\"}");
            //qDebug() << "dataEncrypted: " << dataEncrypted.c_str ();
            auto jumbledJsonString = QString::fromStdString (decrypt(dataEncrypted.toStdString (), keysAndIv.secondKey, keysAndIv.iv));
            jumbledJsonString.replace("o\"<P{#meme=\"\"","{\"e\":[{\"file\":\"");
            //qDebug() << "jumbledJson: " << jumbledJson.c_str ();


            QJsonObject sourceJson = QJsonDocument::fromJson(jumbledJsonString.toUtf8()).object ();

            auto source = sourceJson["source"].toArray ()[0].toObject ()["file"].toString ();
            auto source_bk = sourceJson["source_bk"].toArray ()[0].toObject ()["file"].toString ();
            return source;
        }
        return "";
    }

    std::string encrypt(const std::string& str, const std::string& key, const std::string& iv) {
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

    std::string decrypt(const std::string& ciphertext, const std::string& key, const std::string& iv) {
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
