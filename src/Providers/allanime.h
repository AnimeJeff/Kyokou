#ifndef ALLANIME_H
#define ALLANIME_H

#pragma once
#include <QDebug>
#include "showprovider.h"

#include "Explorer/Data/showdata.h"

class AllAnime : public ShowProvider
{
    std::map<std::string, std::string> headers = {
                                                  {"authority", "api.allanime.day"},
                                                  {"accept-language", "en-GB,en;q=0.9,zh-CN;q=0.8,zh;q=0.7"},
                                                  {"origin", "https://allmanga.to"},
                                                  {"referer", "https://allmanga.to/"},
                                                  // {"user-agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"},
                                                  };
public:
    AllAnime() = default;
    QString name() const override { return "AllAnime"; }
    std::string hostUrl = "https://allmanga.to/";
    QList<int> getAvailableTypes() const override {
        return {ShowData::ANIME};
    };

    QList<ShowData> search(QString query, int page, int type = 0) override {
        std::string url = "https://api.allanime.day/api?variables={%22search%22:{%22query%22:%22" + query.toStdString () + "%22},%22limit%22:26,%22page%22:1,%22translationType%22:%22sub%22,%22countryOrigin%22:%22ALL%22}&extensions={%22persistedQuery%22:{%22version%22:1,%22sha256Hash%22:%2206327bc10dd682e1ee7e07b6db9c16e9ad2fd56c1b769e47513128cd5c9fc77a%22}}";
        QList<ShowData> animes;
        auto jsonResponse = NetworkClient::get (url,headers).json ()["data"]["shows"]["edges"];
        for (auto &el : jsonResponse.items()) {
            auto animeJson = el.value();
            QString coverUrl = QString::fromStdString (animeJson["thumbnail"].get<std::string> ());
            QString title = QString::fromStdString (animeJson["name"].get<std::string> ());
            std::string link = animeJson["_id"].get<std::string> ();
            QString latestEpisodeText = QString::fromStdString (animeJson["lastEpisodeInfo"]["sub"]["episodeString"].get<std::string> ());
            animes.emplaceBack (title, link, coverUrl, this, latestEpisodeText);
        }
        return animes;
    };
    QList<ShowData> popular(int page, int type = 0) override {
        //size = 25
        std::string url = "https://api.allanime.day/api?variables={%22type%22:%22anime%22,%22size%22:25,%22dateRange%22:7,%22page%22:" + std::to_string(page) + ",%22allowAdult%22:true,%22allowUnknown%22:false}&extensions={%22persistedQuery%22:{%22version%22:1,%22sha256Hash%22:%221fc9651b0d4c3b9dfd2fa6e1d50b8f4d11ce37f988c23b8ee20f82159f7c1147%22}}";
        QList<ShowData> animes;
        auto jsonResponse = NetworkClient::get (url, headers).json ()["data"]["queryPopular"]["recommendations"];
        for (auto &el : jsonResponse.items()) {
            auto animeJson = el.value()["anyCard"];
            QString coverUrl = QString::fromStdString (animeJson["thumbnail"].get<std::string> ());
            QString title = QString::fromStdString (animeJson["name"].get<std::string> ());
            std::string link = animeJson["_id"].get<std::string> ();
            // QString latestEpisodeText = QString::fromStdString (animeJson["lastEpisodeInfo"]["sub"]["episodeString"].get<std::string> ());
            animes.emplaceBack (title, link, coverUrl, this);
        }
        return animes;


    };
    QList<ShowData> latest(int page, int type = 0) override{
        std::string url = "https://api.allanime.day/api?variables={%22search%22:{%22sortBy%22:%22Recent%22},%22limit%22:26,%22page%22:"+ std::to_string (page) +",%22translationType%22:%22sub%22,%22countryOrigin%22:%22ALL%22}&extensions={%22persistedQuery%22:{%22version%22:1,%22sha256Hash%22:%2206327bc10dd682e1ee7e07b6db9c16e9ad2fd56c1b769e47513128cd5c9fc77a%22}}";
        QList<ShowData> animes;
        // qDebug () << QString::fromStdString (NetworkClient::get (url, headers).body);
        auto jsonResponse = NetworkClient::get (url, headers).json ()["data"]["shows"]["edges"];
        for (auto &el : jsonResponse.items()) {
            auto animeJson = el.value();
            QString coverUrl = QString::fromStdString (animeJson["thumbnail"].get<std::string> ());
            QString title = QString::fromStdString (animeJson["name"].get<std::string> ());
            std::string link = animeJson["_id"].get<std::string> ();
            QString latestEpisodeText = QString::fromStdString (animeJson["lastEpisodeInfo"]["sub"]["episodeString"].get<std::string> ());
            animes.emplaceBack (title, link, coverUrl, this, latestEpisodeText);
        }
        return animes;
    };

    void loadDetails(ShowData& anime) const override {
        std::string url = "https://api.allanime.day/api?variables={%22_id%22:%22" + anime.link +"%22}&extensions={%22persistedQuery%22:{%22version%22:1,%22sha256Hash%22:%229d7439c90f203e534ca778c4901f9aa2d3ad42c06243ab2c5e6b79612af32028%22}}";
        auto jsonResponse = NetworkClient::get (url,headers).json ()["data"]["show"];
        anime.description =  QString::fromStdString (jsonResponse["description"].get<std::string> ());
        anime.status = QString::fromStdString (jsonResponse["status"].get<std::string> ());;
        anime.views =  QString::fromStdString (jsonResponse["pageStatus"]["views"].get<std::string> ());;
        if (!jsonResponse["score"].is_null ()){
            anime.rating = QString::number(jsonResponse["score"].get<float> (), 'f', 1) + "(MAL)";
        }
        if (!jsonResponse["averageScore"].is_null ()){
            if(!anime.rating.isEmpty ()) anime.rating += "; ";
            anime.rating += QString::fromStdString (std::to_string (jsonResponse["averageScore"].get<int> ()) + "(Anilist)");
        }

        for (const auto& genre : jsonResponse["genres"]) {
            anime.genres.push_back (QString::fromStdString (genre.get <std::string>()));
        }
        int day = jsonResponse["airedStart"]["date"].get<int>();
        int month = jsonResponse["airedStart"]["month"].get<int>() + 1;
        int year = jsonResponse["airedStart"]["year"].get<int>();
        QDate airedStartDate(year, month, day);
        // Get the day of the week (1 = Monday, 7 = Sunday)
        int dayOfWeek = airedStartDate.dayOfWeek();
        // Convert dayOfWeek to a string representing the day
        QString dayOfWeekStr;
        switch(dayOfWeek) {
        case 1: dayOfWeekStr = "Monday"; break;
        case 2: dayOfWeekStr = "Tuesday"; break;
        case 3: dayOfWeekStr = "Wednesday"; break;
        case 4: dayOfWeekStr = "Thursday"; break;
        case 5: dayOfWeekStr = "Friday"; break;
        case 6: dayOfWeekStr = "Saturday"; break;
        case 7: dayOfWeekStr = "Sunday"; break;
        default: dayOfWeekStr = "Unknown";
        }
        anime.releaseDate = airedStartDate.toString("MMMM d, yyyy");
        anime.updateTime = QString("Every %1").arg (dayOfWeekStr);
        if (jsonResponse["airedStart"].contains("hour")){
            int hour = jsonResponse["airedStart"]["hour"].get<int>();
            int minute = jsonResponse["airedStart"]["minute"].get<int>();
            anime.updateTime += QString(" at %1:%2").arg(hour, 2, 10, QLatin1Char('0')).arg(minute, 2, 10, QLatin1Char('0'));
        }
        for (int i = jsonResponse["availableEpisodesDetail"]["sub"].size () - 1; i > -1; i--) {
            try {
                // qDebug() << QString::fromStdString (episode.get<std::string>());
                auto episodeString = jsonResponse["availableEpisodesDetail"]["sub"][i].get<std::string>();
                auto url = "https://api.allanime.day/api?variables=%7B%22showId%22%3A%22" + anime.link + "%22%2C%22translationType%22%3A%22sub%22%2C%22episodeString%22%3A%22" + episodeString + "%22%7D&extensions=%7B%22persistedQuery%22%3A%7B%22version%22%3A1%2C%22sha256Hash%22%3A%225f1a64b73793cc2234a389cf3a8f93ad82de7043017dd551f38f65b89daa65e0%22%7D%7D";
                anime.addEpisode(std::stoi(episodeString), url, "");
            } catch (const std::exception& e) {
                qDebug() << "Conversion error: " << e.what();
            }
        }


    };
    int getTotalEpisodes(const std::string& link) const override {

        return 0;
    };
    QList<VideoServer> loadServers(const PlaylistItem* episode) const override {
        auto jsonResponse = NetworkClient::get (episode->link, headers).json ();
        QList<VideoServer> servers;
        for (const auto& server : jsonResponse["data"]["episode"]["sourceUrls"]) {
            try {
                auto name = QString::fromStdString (server["sourceName"].get<std::string>());
                std::string link = server["sourceUrl"].get<std::string>();
                qDebug() << name;
                servers.emplaceBack (name, link);
            } catch (const std::exception& e) {
                qDebug() << "Error: Failed to read server json." << e.what();
            }
        }

        return servers;
    };
    QString extractSource(const VideoServer& server) const override
    {
        std::string endPoint = NetworkClient::get (hostUrl + "getVersion").json ()["episodeIframeHead"].get <std::string>();
        auto decryptedLink = decryptSource(server.link);
        qDebug() << QString::fromStdString (endPoint+decryptedLink);
        qDebug() << NetworkClient::get (endPoint + decryptedLink, headers);

        QString source = "";
        if (decryptedLink.starts_with ("/apivtwo/")){
            decryptedLink.replace (8, 7, "/clock.json?");
            qDebug() << decryptedLink;
            auto jsonResponse = NetworkClient::get (endPoint + decryptedLink, headers).json ();

            for (const auto& link : jsonResponse["links"]) {
                try {
                    auto linkString = QString::fromStdString (link["link"].get<std::string>());
                    qDebug() << linkString;
                    return linkString;
                    if (decryptedLink.starts_with ("/apivtwo")){
                        source = linkString;
                    }

                    // qDebug() << QString::fromStdString (decryptSource(link));

                } catch (const std::exception& e) {
                    qDebug() << "Error: Failed to read link json." << e.what();
                }
            }

        }

        return source;
    };

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




#endif // ALLANIME_H
