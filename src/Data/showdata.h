#pragma once

#include <QObject>
#include <QList>
#include <QJsonObject>
#include <QDebug>
class PlaylistItem;
class ShowProvider;

struct ShowData
{
    ShowData(const QString& title, const QString& link, const QString& coverUrl,
             ShowProvider* provider, const QString& latestTxt = "", int type = 0)
        : title(title), link(link), coverUrl(coverUrl), provider(provider), latestTxt(latestTxt), type(type) {};


    QString title = "";
    QString link = "";
    QString coverUrl = "";
    QString latestTxt = "";
    ShowProvider* provider;
    int type = 0;
    QString description = "";
    QString releaseDate = "";
    QString status = "";
    QList<QString> genres;
    QString updateTime = "";
    QString score = "";
    QString views = "";

public:

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

    struct LastWatchInfo {
        int listType = -1;
        int lastWatchedIndex = -1;
        int timeStamp = 0;
    };
    inline PlaylistItem *getPlaylist() const { return playlist; }
    inline ShowProvider *getProvider() const { return provider; }


    friend class ShowManager;
    void setListType(int newListType) { listType = newListType; }

    void addEpisode(float number, const QString &link, const QString &name);
    QJsonObject toJson() const;
    QString toString() const;
private:
    int listType = -1;
    PlaylistItem* playlist = nullptr;

};


