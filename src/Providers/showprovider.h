#pragma once
#include "Components/ErrorHandler.h"
#include "Components/MyException.h"
#include "Components/functions.h"
#include "Components/network.h"
#include "Data/showdata.h"
#include "Data/playlistitem.h"
#include <QFutureWatcher>
#include <QNetworkReply>
#include <QString>
#include "Data/video.h"

class ShowProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT);

    QString m_preferredServer;
public:
    ShowProvider(QObject *parent = nullptr) : QObject(parent){};
    virtual QString name() const = 0;
    std::string hostUrl = "";
    virtual QList<int> getAvailableTypes() const = 0;

    virtual QList<ShowData> search(QString query, int page, int type) = 0;
    virtual QList<ShowData> popular(int page, int type) = 0;
    virtual QList<ShowData> latest(int page, int type) = 0;

    virtual void loadDetails(ShowData &show) const = 0;
    virtual int getTotalEpisodes(const std::string &link) const = 0;
    virtual QList<VideoServer> loadServers(const PlaylistItem *episode) const = 0;
    virtual QList<Video> extractSource(const VideoServer &server) const = 0;

    void setPreferredServer(const QString &serverName) {
        m_preferredServer = serverName;
    }

    QString getPreferredServer() const {
        return m_preferredServer;
    }

};
