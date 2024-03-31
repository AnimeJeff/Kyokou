#pragma once
#include <QDebug>
#include "showprovider.h"
#include <QJsonArray>

class AllAnime : public ShowProvider
{
private:
    std::map<std::string, std::string> headers = {
                                                  {"authority", "api.allanime.day"},
                                                  {"accept-language", "en-GB,en;q=0.9,zh-CN;q=0.8,zh;q=0.7"},
                                                  {"origin", "https://allmanga.to"},
                                                  {"referer", "https://allmanga.to/"},
                                                  };
public:
    AllAnime() = default;
    QString name() const override { return "AllAnime"; }
    std::string hostUrl = "https://allmanga.to/";
    QList<int> getAvailableTypes() const override {
        return {ShowData::ANIME};
    };
    
    QList<ShowData> search(QString query, int page, int type = 0) override {
        std::string urlString = "https://api.allanime.day/api?variables={\"search\":{\"query\":\"" + QUrl::toPercentEncoding(query).toStdString () + "\"},\"limit\":26,\"page\":" + std::to_string (page) + ",\"translationType\":\"sub\",\"countryOrigin\":\"ALL\"}&extensions={\"persistedQuery\":{\"version\":1,\"sha256Hash\":\"06327bc10dd682e1ee7e07b6db9c16e9ad2fd56c1b769e47513128cd5c9fc77a\"}}";
        QList<ShowData> animes;
        QJsonArray jsonResponse = NetworkClient::get(urlString, headers)
                                      .JSONOBJECT()["data"]
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
                animes.append(ShowData(title, link.toStdString (), coverUrl, this));
            }
        }
        return animes;
    }

    QList<ShowData> popular(int page, int type = 0) override {
        //size = 25
        std::string url = "https://api.allanime.day/api?variables={%22type%22:%22anime%22,%22size%22:25,%22dateRange%22:7,%22page%22:" + std::to_string(page) + ",%22allowAdult%22:true,%22allowUnknown%22:false}&extensions={%22persistedQuery%22:{%22version%22:1,%22sha256Hash%22:%221fc9651b0d4c3b9dfd2fa6e1d50b8f4d11ce37f988c23b8ee20f82159f7c1147%22}}";
        QList<ShowData> animes;

        QJsonObject jsonResponse = NetworkClient::get(url, headers).JSONOBJECT();
        QJsonArray recommendations = jsonResponse["data"].toObject()["queryPopular"].toObject()["recommendations"].toArray();

        for (const QJsonValue &value : recommendations) {
            QJsonObject animeJson = value.toObject()["anyCard"].toObject();
            QString coverUrl = animeJson["thumbnail"].toString();
            coverUrl.replace("https:/", "https://wp.youtube-anime.com");
            if (coverUrl.startsWith("images3"))
                coverUrl = "https://wp.youtube-anime.com/aln.youtube-anime.com/" + coverUrl;

            QString title = animeJson["name"].toString();
            QString link = animeJson["_id"].toString();

            if (!title.isEmpty() && !link.isEmpty()) {
                animes.append(ShowData(title, link.toStdString (), coverUrl, this));
            }
        }

        return animes;


    };
    QList<ShowData> latest(int page, int type = 0) override{
        std::string urlString = "https://api.allanime.day/api?variables={%22search%22:{%22sortBy%22:%22Recent%22},%22limit%22:26,%22page%22:"+ std::to_string (page) +",%22translationType%22:%22sub%22,%22countryOrigin%22:%22JP%22}&extensions={%22persistedQuery%22:{%22version%22:1,%22sha256Hash%22:%2206327bc10dd682e1ee7e07b6db9c16e9ad2fd56c1b769e47513128cd5c9fc77a%22}}";
        QList<ShowData> animes;
        // qDebug () << QString::fromStdString (NetworkClient::get (url, headers).body);
        QJsonArray jsonResponse = NetworkClient::get(urlString, headers)
                                      .JSONOBJECT()["data"]
                                      .toObject()["shows"]
                                      .toObject()["edges"]
                                      .toArray();

        for (const QJsonValue& value : jsonResponse) {
            QJsonObject animeJson = value.toObject();
            QString coverUrl = animeJson.value("thumbnail").toString();
            coverUrl.replace("https:/", "https://wp.youtube-anime.com");
            if (coverUrl.startsWith("images3"))
                coverUrl = "https://wp.youtube-anime.com/aln.youtube-anime.com/" + coverUrl;

            QString title = animeJson.value("name").toString();
            QString link = animeJson.value("_id").toString();

            // Adding checks for empty values if necessary
            if (!title.isEmpty() && !link.isEmpty()) {
                animes.append(ShowData(title, link.toStdString (), coverUrl, this));
            }
        }
        return animes;
    };

    void loadDetails(ShowData& anime) const override {
        std::string url = "https://api.allanime.day/api?variables={%22_id%22:%22" + anime.link +"%22}&extensions={%22persistedQuery%22:{%22version%22:1,%22sha256Hash%22:%229d7439c90f203e534ca778c4901f9aa2d3ad42c06243ab2c5e6b79612af32028%22}}";
        auto jsonResponse = NetworkClient::get(url, headers).JSONOBJECT()["data"].toObject ()["show"].toObject ();
        anime.description =  jsonResponse["description"].toString ();
        anime.status = jsonResponse["status"].toString ();;
        anime.views =  jsonResponse["pageStatus"].toObject ()["views"].toString ();;

        QJsonValue scoreValue = jsonResponse["score"];
        if (!scoreValue.isUndefined() && !scoreValue.isNull()) {
            anime.score = QString::number(scoreValue.toDouble(), 'f', 1) + " (MAL)";
        }

        QJsonValue averageScoreValue = jsonResponse["averageScore"];
        if (!averageScoreValue.isUndefined() && !averageScoreValue.isNull()) {
            if (!anime.score.isEmpty()) anime.score += "; ";
            anime.score += QString::number(averageScoreValue.toInt()) + " (Anilist)";
        }

        QJsonArray genresArray = jsonResponse["genres"].toArray();
        for (const QJsonValue& genreValue : genresArray) {
            anime.genres.push_back(genreValue.toString());
        }

        QJsonObject airedStart = jsonResponse["airedStart"].toObject();
        int day = airedStart["date"].toInt(69);
        int month = airedStart["month"].toInt(69) + 1; // Adjusting month from 0-based to 1-based indexing
        int year = airedStart["year"].toInt(69);
        QDate airedStartDate(year, month, day);
        if (airedStartDate.isValid ()){
            // Convert dayOfWeek to a string representing the day
            anime.releaseDate = airedStartDate.toString("MMMM d, yyyy");
            anime.updateTime = airedStartDate.toString("Every dddd");
        }


        if (airedStart.contains("hour")) {
            int hour = airedStart["hour"].toInt();
            int minute = airedStart["minute"].toInt(0);
            anime.updateTime += QString(" at %1:%2").arg(hour, 2, 10, QLatin1Char('0')).arg(minute, 2, 10, QLatin1Char('0'));
        }

        QJsonArray episodesArray = jsonResponse["availableEpisodesDetail"].toObject()["sub"].toArray();
        for (int i = episodesArray.size() - 1; i >= 0; --i) {
            QString episodeString = episodesArray.at(i).toString();
            QString episodeUrl = QString("https://api.allanime.day/api?variables={\"showId\":\"%1\",\"translationType\":\"sub\",\"episodeString\":\"%2\"}&extensions={\"persistedQuery\":{\"version\":1,\"sha256Hash\":\"5f1a64b73793cc2234a389cf3a8f93ad82de7043017dd551f38f65b89daa65e0\"}}").arg(QString::fromStdString(anime.link), episodeString);
            anime.addEpisode(episodeString.toFloat(), episodeUrl.toStdString (), "");
        }


    };
    int getTotalEpisodes(const std::string& link) const override {

        return 0;
    };
    QList<VideoServer> loadServers(const PlaylistItem* episode) const override {
        QJsonObject jsonResponse = NetworkClient::get(episode->link, headers).JSONOBJECT();
        QList<VideoServer> servers;

        QJsonArray sourceUrls = jsonResponse["data"].toObject()["episode"].toObject()["sourceUrls"].toArray();
        for (const QJsonValue &value : sourceUrls) {
            QJsonObject server = value.toObject();
            QString name = server["sourceName"].toString();
            QString link = server["sourceUrl"].toString();
            servers.emplaceBack (name, link.toStdString ());

        }

        return servers;
    }

    QString extractSource(const VideoServer& server) const override {
        QString endPoint = NetworkClient::get(hostUrl + "getVersion").JSONOBJECT()["episodeIframeHead"].toString();
        auto decryptedLink = decryptSource(server.link);
        //qDebug().noquote() << "Log (AllAnime): Decrypted link" << decryptedLink;
        QString source;

        if (decryptedLink.starts_with("/apivtwo/")) {
            decryptedLink.insert (14,".json");
            QJsonObject jsonResponse = NetworkClient::get(endPoint.toStdString () + decryptedLink, headers).JSONOBJECT();
            QJsonArray links = jsonResponse["links"].toArray();

            // qDebug() .noquote()<< decryptedLink;
            // qDebug ().noquote() << endPoint.toStdString () + decryptedLink;
            //qDebug() .noquote() << "response json \n" << QJsonDocument(jsonResponse).toJson ();


            for (const QJsonValue& value : links) {
                QJsonObject linkObject = value.toObject();
                if (!linkObject["dash"].toBool ())
                {
                    QString linkString = linkObject["link"].toString();
                    return linkString;
                }

            }
        }

        return source;
    }

    std::string decryptSource(const std::string& input) const {
        if (input.empty() || input.front() != '-') {
            return input;
        }

        // Find the position of the last '-'
        size_t lastHyphenPos = input.find_last_of('-');
        if (lastHyphenPos == std::string::npos) {
            return input; // Return original if no hyphen is found
        }

        // Extract substring after the last '-'
        std::string encryptedPart = input.substr(lastHyphenPos + 1);
        std::string decrypted;
        for (size_t i = 0; i < encryptedPart.length(); i += 2) {
            // Extract two characters at a time
            std::string part = encryptedPart.substr(i, 2);

            // Convert from hex to int
            int value;
            std::istringstream(part) >> std::hex >> value;

            // XOR with 56 and convert to char
            char decryptedChar = static_cast<char>(value ^ 56);
            decrypted.push_back(decryptedChar);
        }

        return decrypted;
    }

};





