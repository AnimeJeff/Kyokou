#ifndef SHOWRESPONSE_H
#define SHOWRESPONSE_H
#include "episode.h"
#include "nlohmann/json.hpp"

#include <QMetaType>

#include <QDebug>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>
#include <iostream>

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

enum class TvType {
    Movie,
    TvSeries,
    Reality,
    Anime,
    Documentary
};
enum class Status {
    Ongoing,
    Completed
};
enum Providers{
    e_Nivod,
    e_Consumet9anime,
    e_Gogoanime,
    e_NineAnime,
};
class ShowResponseObject;

struct ShowResponse
{
public:
    ShowResponse(QString title,QString link,QString coverUrl,int provider){
        this->title=title;
        this->link=link;
        this->coverUrl=coverUrl;
        this->provider=provider;
    };
    ShowResponse(nlohmann::json& jsonObject){
        this->jsonObject = &jsonObject;
        this->link = QString::fromStdString(jsonObject["link"].get<std::string>());
        this->title = QString::fromStdString(jsonObject["title"].get<std::string>());
        this->coverUrl = QString::fromStdString(jsonObject["cover"].get<std::string>());
        this->provider = jsonObject["provider"].get<int>();
        this->setLastWatchedIndex(jsonObject["lastWatchedIndex"].get<int>());
        this->isInWatchList = true;
        this->listType = jsonObject["listType"].get<int>();
        if(this->listType < 0 || this->listType > 3){
            qWarning()<<"Invalid list type.";
            this->listType = 0;
            jsonObject["listType"] = 0;
        }
    };
    ShowResponse(){};
    QString title = "";
    QString link = "";
    QString coverUrl = "";
    int provider = 0;
    QString latestTxt = "";
    int type = 0;
    QVector<Episode> episodes;
    QString description = "";
    QString releaseDate = "";
    QString status = "Unknown";
    QVector<QString> genres{};
    QString updateTime = "Unknown";
    QString rating = "0.0";
    int views = 0;
    friend QDebug operator<<(QDebug debug, const ShowResponse& show)
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
    friend QDebug operator<<(QDebug debug, const ShowResponse* show){
        debug << *show;
        return debug;
    }
    int getLastWatchedIndex() const {return lastWatchedIndex;}

    void setLastWatchedIndex(int index) {
        if(lastWatchedIndex==index)return;
        lastWatchedIndex = index;
        if(jsonObject){
            (*jsonObject)["lastWatchedIndex"] = index;
        }
    }

    void setListType(int listType){
        if(this->listType==listType)return;
        this->listType = listType;
        if(jsonObject){
            (*jsonObject)["listType"] = listType;
        }
    }

    friend class ShowResponseObject;
    friend class WatchListModel;
    bool operator==(const ShowResponse& other) const {
        return (title == other.title) && (link == other.link);
    }
    nlohmann::json* jsonObject;
private:

    ShowResponseObject* object;
    bool isInWatchList = false;
    int lastWatchedIndex = -1;
    int listType = -1;
};

class ShowResponseObject:public QObject{
    Q_OBJECT
    ShowResponse show;
    ShowResponse* watchListShow;
    Q_PROPERTY(QString title READ title NOTIFY showChanged);
    Q_PROPERTY(QString coverUrl READ coverUrl NOTIFY showChanged);
    Q_PROPERTY(QString desc READ desc NOTIFY showChanged);
    Q_PROPERTY(QString year READ year NOTIFY showChanged);
    Q_PROPERTY(QString status READ status NOTIFY showChanged);
    Q_PROPERTY(QString updateTime READ updateTime NOTIFY showChanged);
    Q_PROPERTY(QString rating READ rating NOTIFY showChanged);
    Q_PROPERTY(int views READ views NOTIFY showChanged);
    Q_PROPERTY(QString genresString READ genresString NOTIFY showChanged);
    Q_PROPERTY(bool isInWatchList READ isInWatchList NOTIFY showChanged NOTIFY showPropertyChanged);
    Q_PROPERTY(int lastWatchedIndex READ lastWatchedIndex NOTIFY showChanged NOTIFY showPropertyChanged NOTIFY lastWatchedIndexChanged);
    Q_PROPERTY(bool hasShow READ hasShow NOTIFY showChanged);
    Q_PROPERTY(int listType READ listType NOTIFY listTypeChanged);
public:
    ShowResponseObject(QObject* parent = nullptr) : QObject(parent) {}

    inline void setLastWatchedIndex(int index) {
        show.lastWatchedIndex = index;
        emit showPropertyChanged();
        emit lastWatchedIndexChanged();
    }

    inline void setIsInWatchList(bool isInWatchList) {
        show.isInWatchList = isInWatchList;
        emit showPropertyChanged();
    }

    inline void setListType(int listType) {
        show.setListType (listType);
        emit listTypeChanged();
    }

    inline void setShow(const ShowResponse& show) {
        this->show = show;
        this->show.object = this;
        emit showChanged();
    }

    inline void setWatchListShow(ShowResponse* show) {
        this->watchListShow = show;
    }

    inline ShowResponse getShow() const {return show;}

public:
    inline QString title() const {return show.title;}
    inline QString coverUrl() const {return show.coverUrl;}
    inline bool hasShow() const {return !show.title.isEmpty ();}
    inline QString desc() const {return show.description;}
    inline QString year() const {return show.releaseDate;}
    inline QString updateTime() const {return show.updateTime;}
    inline QString rating() const {return show.rating;}
    inline int views() const {return show.views;}
    inline QString status() const {return show.status;}
    inline QString genresString() const {return show.genres.join (' ');}
    inline int lastWatchedIndex() const {return show.lastWatchedIndex;}
    inline bool isInWatchList() const {return show.isInWatchList;}
    inline QString link() const {return show.link;}
    inline QVector<Episode> episodes() const {return show.episodes;}
    inline int listType() const {return show.listType;}
    inline int provider() const {return show.provider;}
signals:
    void showChanged(void);
    void showPropertyChanged(void);
    void lastWatchedIndexChanged(void);
    void listTypeChanged(void);
};

#endif // SHOWRESPONSE_H
