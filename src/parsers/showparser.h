#ifndef SHOWPARSER_H
#define SHOWPARSER_H

#include "episode.h"
#include "showresponse.h"

#include <QFutureWatcher>
#include <QNetworkReply>
#include <QString>
#include <network/client.h>


class ShowParser: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT);
    Q_PROPERTY(int providerEnum READ providerEnum CONSTANT);
protected:
    bool m_canFetchMore = false;
    int m_currentPage = 0;
    NetworkClient client;
    QFutureWatcher<void> watcher;
    QFutureWatcher<QVector<ShowResponse>> searchWatcher;
public:
    ShowParser(){};
    virtual int providerEnum() = 0;
    virtual QString name() = 0;
    virtual std::string hostUrl() = 0;
    virtual QVector<ShowResponse> search(QString query,int page,int type) = 0;
    virtual QVector<ShowResponse> popular(int page,int type) = 0;
    virtual QVector<ShowResponse> latest(int page,int type) = 0;
    virtual bool canFetchMore() final {return m_canFetchMore;};
    virtual QVector<ShowResponse> fetchMore() = 0;
    virtual ShowResponse loadDetails(ShowResponse show)  = 0;
    virtual QVector<VideoServer> loadServers(const Episode& episode) = 0;
    virtual void extractSource(VideoServer& server) = 0;
signals:
    void sourceFetched(QString link);
    void error(QString errorString);

};

#endif // SHOWPARSER_H
