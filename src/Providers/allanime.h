#pragma once
#include <QDebug>
#include "Providers/Extractors/gogocdn.h"
#include "showprovider.h"
#include <QJsonArray>
#include <Data/video.h>

class AllAnime : public ShowProvider
{
private:
    QMap<QString, QString> headers = {
                                      {"authority", "api.allanime.day"},
                                      {"accept-language", "en-GB,en;q=0.9,zh-CN;q=0.8,zh;q=0.7"},
                                      {"origin", "https://allmanga.to"},
                                      {"referer", "https://allmanga.to/"},
                                      };
public:
    AllAnime() = default;
    QString name() const override { return "AllAnime"; }
    QString hostUrl = "https://allmanga.to/";
    QList<int> getAvailableTypes() const override {
        return {ShowData::ANIME};
    };
    
    QList<ShowData> search(QString query, int page, int type = 0) override {
        QString url = "https://api.allanime.day/api?variables={\"search\":{\"query\":\""
                            + QUrl::toPercentEncoding(query) + "\"},\"limit\":26,\"page\":"
                            + QString::number(page)
                            + ",\"translationType\":\"sub\",\"countryOrigin\":\"ALL\"}&extensions={\"persistedQuery\":{\"version\":1,\"sha256Hash\":\"06327bc10dd682e1ee7e07b6db9c16e9ad2fd56c1b769e47513128cd5c9fc77a\"}}";
        QList<ShowData> animes;
        QJsonArray jsonResponse = NetworkClient::get(url, headers)
                                      .toJson()["data"]
                                      .toObject()["shows"]
                                      .toObject()["edges"]
                                      .toArray();

        for (const QJsonValue& value : jsonResponse) {
            QJsonObject animeJson = value.toObject();
            QString coverUrl = animeJson["thumbnail"].toString();
            coverUrl.replace("https:/", "https://wp.youtube-anime.com");
            if (coverUrl.startsWith("images3"))
                coverUrl = "https://wp.youtube-anime.com/aln.youtube-anime.com/" + coverUrl;

            QString title = animeJson.value("name").toString();
            QString link = animeJson.value("_id").toString();

            // Adding checks for empty values if necessary
            if (!title.isEmpty() && !link.isEmpty()) {
                animes.emplaceBack (title, link, coverUrl, this);
            }
        }
        return animes;
    }

    QList<ShowData> popular(int page, int type = 0) override;;

    QList<ShowData> latest(int page, int type = 0) override;

    void loadDetails(ShowData& anime) const override;
    int getTotalEpisodes(const QString& link) const override {

        return 0;
    }
    QList<VideoServer> loadServers(const PlaylistItem* episode) const override {
        QJsonObject jsonResponse = NetworkClient::get(episode->link, headers).toJson();
        QList<VideoServer> servers;

        QJsonArray sourceUrls = jsonResponse["data"].toObject()["episode"].toObject()["sourceUrls"].toArray();
        for (const QJsonValue &value : sourceUrls) {
            QJsonObject server = value.toObject();
            QString name = server["sourceName"].toString();
            QString link = server["sourceUrl"].toString();
            servers.emplaceBack (name, link);

        }

        return servers;
    }

    QList<Video> extractSource(const VideoServer& server) const override {
        QString endPoint = NetworkClient::get(hostUrl + "getVersion").toJson()["episodeIframeHead"].toString();
        auto decryptedLink = decryptSource(server.link);
        //qInfo().noquote() << "Log (AllAnime): Decrypted link" << decryptedLink;
        // QString source;

        if (decryptedLink.startsWith ("/apivtwo/")) {
            decryptedLink.insert (14,".json");
            QJsonObject jsonResponse = NetworkClient::get(endPoint + decryptedLink, headers).toJson();
            QJsonArray links = jsonResponse["links"].toArray();

            // qInfo() .noquote()<< decryptedLink;
            // qDebug ().noquote() << endPoint.toStdString () + decryptedLink;
            //qDebug() .noquote() << "response json \n" << QJsonDocument(jsonResponse).toJson ();

            for (const QJsonValue& value : links) {
                QJsonObject linkObject = value.toObject();
                if (!linkObject["dash"].toBool ())
                {
                    QString source = linkObject["link"].toString();
                    return { Video(source) };
                }

            }
        } else if (decryptedLink.contains ("streaming.php")) {
            GogoCDN gogo;
            return { Video(gogo.extract (decryptedLink)) };
        }

        return {};
    }

    QString decryptSource(const QString& input) const {
        if (input.startsWith("-")) {
            // Extract the part after the last '-'
            QString hexString = input.section('-', -1);
            QByteArray bytes;

            // Convert each pair of hex digits to a byte and append to bytes array
            for (int i = 0; i < hexString.length(); i += 2) {
                bool ok;
                QString hexByte = hexString.mid(i, 2);
                bytes.append(static_cast<char>(hexByte.toInt(&ok, 16) & 0xFF));
            }

            // XOR each byte with 56 and convert to char
            QString result;
            for (char byte : bytes) {
                result += QChar(static_cast<char>(byte ^ 56));
            }

            return result;
        } else {
            // If the input does not start with '-', return it unchanged
            return input;
        }
    }
};





