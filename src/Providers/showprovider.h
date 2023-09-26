#ifndef SHOWPROVIDER_H
#define SHOWPROVIDER_H

#include <QFutureWatcher>
#include <QNetworkReply>
#include <QString>
#include "Explorer/Data/videoserver.h"
#include "Explorer/Data/episode.h"
#include "Explorer/Data/showdata.h"
#include "Components/network.h"
#include "Components/ErrorHandler.h"
#include "Components/MyException.h"
#include "Components/functions.h"



class ShowProvider: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT);
    Q_PROPERTY(int providerEnum READ providerEnum CONSTANT);
protected:
    bool m_canFetchMore = false;
    int m_currentPage = 0;
    std::function<QVector<ShowData>()> lastSearch;
public:
    ShowProvider(QObject* parent = nullptr): QObject(parent) {};
    enum Providers{
        NIVOD,
        NINEANIME,
        GOGOANIME,
        ALLANIME,
        TANGRENJIE,
        ANIMEDAO,
        ANIMEPAHE,
        HAITU,
#ifdef QT_DEBUG
        TEST = 9999,
#endif
    };

    virtual int providerEnum() = 0;

    virtual QString name() = 0;

    std::string hostUrl = "";

    virtual QVector<ShowData> search(QString query,int page,int type) = 0;

    virtual QVector<ShowData> popular(int page,int type) = 0;

    virtual QVector<ShowData> latest(int page,int type) = 0;

    virtual bool canFetchMore() final { return m_canFetchMore; }

    virtual QVector<ShowData> fetchMore() final { return lastSearch(); }

    virtual ShowData loadDetails(ShowData show)  = 0;

    virtual int getTotalEpisodes(const ShowData& show) = 0;

    virtual QVector<VideoServer> loadServers(const PlaylistItem& episode) = 0; //todo check if list

    virtual QString extractSource(VideoServer& server) = 0;

signals:
    void fetchedResults(QVector<ShowData> results);
};

#endif // SHOWPROVIDER_H
