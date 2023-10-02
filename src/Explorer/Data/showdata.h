#ifndef SHOWDATA_H
#define SHOWDATA_H

#include <QMetaType>
#include <QDebug>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>
#include <iostream>
#include "Player/playlistitem.h"
#include "nlohmann/json.hpp"

struct ShowData
{
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
    QString getDescription() const {return description.isEmpty () ? "???" : description;}
    QString getReleaseDate() const {return releaseDate.isEmpty () ? "???" : releaseDate;}
    QString getUpdateTime() const {return updateTime.isEmpty () ? "???" : updateTime;}
    QString getRating() const {return rating.isEmpty () ? "???" : rating;}
    QString getGenresString() const {return genres.isEmpty () ? "???" : genres.join (' ');}
    QString getViews() const {return views.isEmpty () ? "???" : views;}
    QString getStatus() const {return status.isEmpty () ? "???" : status;}
public:
    ShowData(const QString& title, const std::string& link, const QString& coverUrl, const QString& provider, const QString& latestTxt = "", int type = 0)
        : title(title), link(link), coverUrl(coverUrl), provider(provider), latestTxt(latestTxt), type(type) {};

    ShowData(){}

    QString title = "";
    std::string link = "";
    QString coverUrl = "";
    QString provider = "";
    QString latestTxt = "";
    int type = 0;

    QString description = "";
    QString releaseDate = "";
    QString status = "";
    QVector<QString> genres;
    QString updateTime = "";
    QString rating = "";
    QString views = "";
    int totalEpisodes = 0;

    const PlaylistItem *getPlaylist() const
    {
        return playlist;
    }

    friend class ShowManager;
    friend class WatchListModel;
    friend class PlaylistModel;
    friend class DownloadModel;
    void addEpisode(int number, std::string link, QString name, bool online = true)
    {
        if(!playlist)
        {
            playlist = new PlaylistItem (title, provider, this->link);
        }
        playlist->emplaceBack (number, link, name, playlist, online);
    }

    bool isInWatchList() const
    {
        return listType > -1;
    }
    nlohmann::json toJson() const{
        return nlohmann::json::object({
                                       {"title", title.toStdString ()},
                                       {"cover", coverUrl.toStdString ()},
                                       {"link", link},
                                       {"provider", provider.toStdString ()},
                                       {"lastWatchedIndex", playlist ? playlist->currentIndex : -1},
                                       });
    }

public:
    friend QDebug operator<<(QDebug debug, const ShowData& show)
    {
        debug << "Title: " << show.title << Qt::endl
              << "Link: " << show.link << Qt::endl
              << "Cover URL: " << show.coverUrl << Qt::endl
              << "Provider: " << show.provider << Qt::endl
              << "Latest Text: " << show.latestTxt << Qt::endl
              << "Type: " << show.type << Qt::endl
              //              << "Episodes: " << show.episodes.size ()<< Qt::endl
              << "Description: " << show.description << Qt::endl
              << "Year: " << show.releaseDate << Qt::endl
              << "Status: " << show.status << Qt::endl
              << "Genres: " << show.genres << Qt::endl
              << "Update Time: " << show.updateTime << Qt::endl
              << "Rating: " << show.rating << Qt::endl
              << "Views: " << show.views << Qt::endl;
        return debug;
    }
        friend QDebug operator<<(QDebug debug, const ShowData* show){
            debug << *show;
            return debug;
        }
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

private:
    PlaylistItem* playlist = nullptr;
    int listType = -1;
};

#endif // SHOWDATA_H
