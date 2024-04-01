#pragma once

#include <QObject>
#include <QList>
#include <QJsonObject>
#include <QDebug>
class PlaylistItem;
class ShowProvider;

struct ShowData
{
private:
    ShowProvider* provider;
public:
    ShowData(const QString& title, const std::string& link, const QString& coverUrl,
             ShowProvider* provider, const QString& latestTxt = "", int type = 0, int lastWatchedIndex = -1)
        : title(title), link(link), coverUrl(coverUrl), provider(provider), latestTxt(latestTxt), type(type), lastWatchedIndex(lastWatchedIndex) {};


    QString title = "";
    std::string link = "";
    QString coverUrl = "";
    QString latestTxt = "";
    int type = 0;
    QString description = "";
    QString releaseDate = "";
    QString status = "";
    QList<QString> genres;
    QString updateTime = "";
    QString score = "";
    QString views = "";
    int lastWatchedIndex = -1;

    friend class ShowManager;

    void addEpisode(float number, std::string link, QString name);
    QJsonObject toJson() const;

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

    inline PlaylistItem *getPlaylist() const { return playlist; }
    inline ShowProvider *getProvider() const { return provider; }
    void setListType(int newListType) { listType = newListType; }

    QString toString() const;
private:
    PlaylistItem* playlist = nullptr;
    int listType = -1;
};


