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
    NetworkClient client;
//    QFutureWatcher<void> watcher;
//    QFutureWatcher<QVector<ShowData>> searchWatcher;
    std::function<QVector<ShowData>()> lastSearch;
public:
    ShowProvider(){};
    enum Providers{
        Nivod,
        NineAnimeHQ,
        Gogoanime,
        NtDongMan,
        Tangrenjie,
    };
    virtual int providerEnum() = 0;
    virtual QString name() = 0;
    virtual std::string hostUrl() = 0;
    virtual QVector<ShowData> search(QString query,int page,int type) = 0;
    virtual QVector<ShowData> popular(int page,int type) = 0;
    virtual QVector<ShowData> latest(int page,int type) = 0;
    virtual bool canFetchMore() final {return m_canFetchMore;};
    virtual QVector<ShowData> fetchMore() = 0;
    virtual ShowData loadDetails(ShowData media)  = 0;
    virtual QVector<VideoServer> loadServers(const Episode& episode) = 0;
    virtual QString extractSource(VideoServer& server) = 0;
signals:
    void error(QString errorString);
};

#endif // SHOWPROVIDER_H
