#ifndef MEDIAPROVIDER_H
#define MEDIAPROVIDER_H

#include <QFutureWatcher>
#include <QNetworkReply>
#include <QString>
#include "network/client.h"
#include "parsers/data/videoserver.h"
#include "data/episode.h"
#include "data/mediadata.h"

class MediaProvider: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT);
    Q_PROPERTY(int providerEnum READ providerEnum CONSTANT);
protected:
    bool m_canFetchMore = false;
    int m_currentPage = 0;
    NetworkClient client;
    QFutureWatcher<void> watcher;
    QFutureWatcher<QVector<MediaData>> searchWatcher;
public:
    MediaProvider(){};
    enum Providers{
        Nivod,
        NineAnimeHQ,
        Gogoanime,
        NtDongMan,
        HuaLe,
    };
    virtual int providerEnum() = 0;
    virtual QString name() = 0;
    virtual std::string hostUrl() = 0;
    virtual QVector<MediaData> search(QString query,int page,int type) = 0;
    virtual QVector<MediaData> popular(int page,int type) = 0;
    virtual QVector<MediaData> latest(int page,int type) = 0;
    virtual bool canFetchMore() final {return m_canFetchMore;};
    virtual QVector<MediaData> fetchMore() = 0;
    virtual MediaData loadDetails(MediaData media)  = 0;
    virtual QVector<VideoServer> loadServers(const Episode& episode) = 0;
    virtual QString extractSource(VideoServer& server) = 0;
signals:
    void error(QString errorString);
};

#endif // MEDIAPROVIDER_H
