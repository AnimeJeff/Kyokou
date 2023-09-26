#ifndef SHOWDATA_H
#define SHOWDATA_H

#include <QMetaType>
#include <QDebug>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>
#include <iostream>
#include "episode.h"
#include "Player/playlistitem.h"
#include "nlohmann/json.hpp"

struct ShowData
{
    enum ShowType
    {
        MOVIE = 1,
        TVSERIES,
        VARIETY,
        ANIME,
        DOCUMENTARY,
        NONE
    };
    enum Status
    {
        Ongoing,
        Completed
    };
    Q_GADGET
    Q_PROPERTY(QString title READ getTitle);
    Q_PROPERTY(QString coverUrl READ getCoverUrl);
    Q_PROPERTY(QString description READ getDescription);
    Q_PROPERTY(QString releaseDate READ getReleaseDate);
    Q_PROPERTY(QString status READ getStatus);
    Q_PROPERTY(QString updateTime READ getUpdateTime);
    Q_PROPERTY(QString rating READ getRating);
    Q_PROPERTY(QString views READ getViews);
    Q_PROPERTY(QString genresString READ getGenresString);

    QString getTitle() const {return title;}
    QString getCoverUrl() const {return coverUrl;}
    QString getDescription() const {return description;}
    QString getReleaseDate() const {return releaseDate;}
    QString getUpdateTime() const {return updateTime;}
    QString getRating() const {return rating;}
    QString getGenresString() const {return genres.join (' ');}
    QString getViews() const {return views;}
    QString getStatus() const {return status;}
public:
    ShowData(QString title, std::string link, QString coverUrl, int provider, QString latestTxt = "", int type = 0)
        : title(title), link(link), coverUrl(coverUrl), provider(provider), latestTxt(latestTxt), type(type)

        {

        };

    ShowData(nlohmann::json& jsonObject)
    {
//        this->jsonObject = &jsonObject; //todo
        this->link = jsonObject["link"].get<std::string>();
        this->title = QString::fromStdString(jsonObject["title"].get<std::string>());
        this->coverUrl = QString::fromStdString(jsonObject["cover"].get<std::string>());
        this->provider = jsonObject["provider"].get<int>();
        this->setLastWatchedIndex(jsonObject["lastWatchedIndex"].get<int>());

    };
    ShowData(){}


    QString title = "";
    std::string link = "null";
    QString coverUrl = "";
    int provider = 0;
    QString latestTxt = "";
    int type = 0;

    QString description = "";
    QString releaseDate = "Unknown";
    QString status = "Unknown";
    QVector<QString> genres{};
    QString updateTime = "Unknown";
    QString rating = "Unknown";
    QString views = 0;
    int totalEpisodes = 0;
    std::shared_ptr<PlaylistItem> playlist = nullptr; //todo parent

    void addEpisode(int number, std::string link, QString name, bool online = true)
    {
        //PlaylistItem(number,link,name,playlist);
        if(!playlist) {
            playlist = std::make_shared<PlaylistItem> (title,provider);
            playlist->sourceLink = link;
        }
        playlist->emplaceBack (number,link,name,playlist.get (),online);
    }
    void addEpisode(const QUrl &path)
    {
        if(!playlist) {
            playlist = std::make_shared<PlaylistItem> (title,provider);
            playlist->sourceLink = link;
        }
        playlist->loadFromLocalDir (path, playlist.get ());
    }

    int getLastWatchedIndex() const {
        return lastWatchedIndex;}
    void setLastWatchedIndex(int index)
    {
        if(lastWatchedIndex==index)return;
        lastWatchedIndex = index;
    }

    int getListType() const { return listType; }
    void setListType(int listType)
    {
        if(this->listType==listType)return;
        this->listType = listType;
    }
    bool isInWatchList() const {
        return listType>-1;
    }

    friend class WatchListModel;
    bool operator==(const ShowData& other) const {
        return ( (title == other.title) && (link == other.link) );
    }
    bool operator <=>(const ShowData& other){
        //todo deep check
        if(!(*this==other))return false;
        return false;
    }

    nlohmann::json toJson() const{
        return nlohmann::json::object({
                                       {"title", title.toStdString ()},
                                       {"cover", coverUrl.toStdString ()},
                                       {"link", link},
                                       {"provider", provider},
                                       {"listType",listType},
                                       {"lastWatchedIndex", lastWatchedIndex},
                                       });
    }

public:
//    friend QDebug operator<<(QDebug debug, const ShowData& show)
//    {
//        debug << "Title: " << show.title << Qt::endl
//              << "Link: " << show.link << Qt::endl
//              << "Cover URL: " << show.coverUrl << Qt::endl
//              << "Provider: " << show.provider << Qt::endl
//              << "Latest Text: " << show.latestTxt << Qt::endl
//              << "Type: " << show.type << Qt::endl
//              //              << "Episodes: " << show.episodes.size ()<< Qt::endl
//              << "Description: " << show.description << Qt::endl
//              << "Year: " << show.releaseDate << Qt::endl
//              << "Status: " << show.status << Qt::endl
//              << "Genres: " << show.genres << Qt::endl
//              << "Update Time: " << show.updateTime << Qt::endl
//              << "Rating: " << show.rating << Qt::endl
//              << "Views: " << show.views << Qt::endl;
//        return debug;
//    }
    //    friend QDebug operator<<(QDebug debug, const ShowData* show){
    //        debug << *show;
    //        return debug;
    //    }
private:
    int lastWatchedIndex = -1;
    int listType = -1;
};

#endif // SHOWDATA_H
