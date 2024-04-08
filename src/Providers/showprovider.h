#pragma once
#include "network/myexception.h"
#include "network/network.h"
#include "Providers/utils/functions.h"

#include "data/showdata.h"
#include "data/playlistitem.h"
#include "data/video.h"

#include <QString>


class ShowProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT);

    QString m_preferredServer;
public:
    ShowProvider(QObject *parent = nullptr) : QObject(parent){};
    virtual QString name() const = 0;
    QString hostUrl = "";
    virtual QList<int> getAvailableTypes() const = 0;

    virtual QList<ShowData> search(QString query, int page, int type) = 0;
    virtual QList<ShowData> popular(int page, int type) = 0;
    virtual QList<ShowData> latest(int page, int type) = 0;

    virtual void loadDetails(ShowData &show) const = 0;
    virtual int getTotalEpisodes(const QString &link) const = 0;
    virtual QList<VideoServer> loadServers(const PlaylistItem *episode) const = 0;
    virtual QList<Video> extractSource(const VideoServer &server) const = 0;

    void setPreferredServer(const QString &serverName) {
        m_preferredServer = serverName;
    }

    QString getPreferredServer() const {
        return m_preferredServer;
    }

};


