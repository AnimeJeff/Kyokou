#ifndef MEDIADATA_H
#define MEDIADATA_H
#include "episode.h"
#include "nlohmann/json.hpp"

#include <QMetaType>

#include <QDebug>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>
#include <iostream>



enum class Status {
    Ongoing,
    Completed
};



struct MediaData
{

public:
    MediaData(QString title,QString link,QString coverUrl,int provider){
        this->title=title;
        this->link=link;
        this->coverUrl=coverUrl;
        this->provider=provider;
    };
    MediaData(nlohmann::json& jsonObject){
        this->jsonObject = &jsonObject;
        this->link = QString::fromStdString(jsonObject["link"].get<std::string>());
        this->title = QString::fromStdString(jsonObject["title"].get<std::string>());
        this->coverUrl = QString::fromStdString(jsonObject["cover"].get<std::string>());
        this->provider = jsonObject["provider"].get<int>();
        this->setLastWatchedIndex(jsonObject["lastWatchedIndex"].get<int>());
    };
    MediaData(){};
    QString title = "";
    QString link = "";
    QString coverUrl = "";
    int provider = 0;
    QString latestTxt = "";
    int type = 0;
    QVector<Episode> episodes;
    QString description = "";
    QString releaseDate = "Unknown";
    QString status = "Unknown";
    QVector<QString> genres{};
    QString updateTime = "Unknown";
    QString rating = "Unknown";
    QString views = 0;

    int getLastWatchedIndex() const {return lastWatchedIndex;}
    void setLastWatchedIndex(int index) {
        if(lastWatchedIndex==index)return;
        lastWatchedIndex = index;
    }

    int getListType() const { return listType; }
    void setListType(int listType){
        if(this->listType==listType)return;
        this->listType = listType;
    }
    bool isInWatchList() const {
        return listType>-1;
    }

    friend class MediaDataObject;
    friend class WatchListModel;
    bool operator==(const MediaData& other) const {
        return ( (title == other.title) && (link == other.link) );
    }
    bool operator <=>(const MediaData& other){
        //todo deep check
        if(!(*this==other))return false;
        return false;
    }
    nlohmann::json* jsonObject = nullptr;
    nlohmann::json toJson() const{
        return nlohmann::json::object({
             {"title", title.toStdString ()},
             {"cover", coverUrl.toStdString ()},
             {"link", link.toStdString ()},
             {"provider", provider},
             {"listType",listType},
             {"lastWatchedIndex", lastWatchedIndex},
             });
    }
    void setJsonObject(nlohmann::json& jsonobj){
        this->jsonObject = &jsonobj;
    }
public:
    friend QDebug operator<<(QDebug debug, const MediaData& show)
    {
        debug << "Title: " << show.title << Qt::endl
              << "Link: " << show.link << Qt::endl
              << "Cover URL: " << show.coverUrl << Qt::endl
              << "Provider: " << show.provider << Qt::endl
              << "Latest Text: " << show.latestTxt << Qt::endl
              << "Type: " << show.type << Qt::endl
              << "Episodes: " << show.episodes.size ()<< Qt::endl
              << "Description: " << show.description << Qt::endl
              << "Year: " << show.releaseDate << Qt::endl
              << "Status: " << show.status << Qt::endl
              << "Genres: " << show.genres << Qt::endl
              << "Update Time: " << show.updateTime << Qt::endl
              << "Rating: " << show.rating << Qt::endl
              << "Views: " << show.views << Qt::endl;
        return debug;
    }
    friend QDebug operator<<(QDebug debug, const MediaData* show){
        debug << *show;
        return debug;
    }
private:
    int lastWatchedIndex = -1;
    int listType = -1;
};


#endif // MEDIADATA_H
