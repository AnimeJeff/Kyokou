#ifndef SHOWPROVIDER_H
#define SHOWPROVIDER_H

#include <QFutureWatcher>
#include <QNetworkReply>
#include <QString>
#include "network/client.h"
#include "parsers/data/videoserver.h"
#include "data/episode.h"
#include "data/showdata.h"

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
    ShowProvider(QObject* parent = nullptr): QObject(parent){

    };
    enum Providers{
        NIVOD,
        NINEANIME,
        GOGOANIME,
        NtDongMan,
        TANGRENJIE,
        ANIMEDAO,
        ANIMEPAHE,
        HAITU
    };

    virtual int providerEnum() = 0;

    virtual QString name() = 0;

    virtual std::string hostUrl() = 0;

    virtual QVector<ShowData> search(QString query,int page,int type) = 0;

    virtual QVector<ShowData> popular(int page,int type) = 0;

    virtual QVector<ShowData> latest(int page,int type) = 0;

    virtual bool canFetchMore() final { return m_canFetchMore; }

    virtual QVector<ShowData> fetchMore() final { return lastSearch(); }

    virtual ShowData loadDetails(ShowData show)  = 0;

    virtual int getTotalEpisodes(const ShowData& show) = 0;

    virtual QVector<VideoServer> loadServers(const Episode& episode) = 0;

    virtual QString extractSource(VideoServer& server) = 0;

signals:
    void fetchedResults(QVector<ShowData> results);
};

#endif // SHOWPROVIDER_H
