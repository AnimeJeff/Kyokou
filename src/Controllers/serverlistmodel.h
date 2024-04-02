#pragma once

#include "Data/playlistitem.h"
#include <QAbstractListModel>
#include "Providers/showprovider.h"
#include "Mpv/mpvObject.h"

class ServerListModel : public QAbstractListModel {

    Q_OBJECT
    Q_PROPERTY(int currentIndex READ getCurrentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
public:
    ServerListModel() = default;
    ~ServerListModel() = default;
    void setServers(const QList<VideoServer>& servers, ShowProvider* provider) {
        if (!provider || servers.isEmpty ()) return;
        m_provider = provider;
        m_servers = servers;

        emit layoutChanged();
    }

    void invalidateServer(int index) {
        // server is not working
        if (index > m_servers.size() || index < 0)
            return;
        m_servers[index].working = false;
    }

    static QPair<QList<Video>, int> autoSelectServer(const QList<VideoServer> &servers, ShowProvider *provider, const QString &preferredServerName = "") {
        QString preferredServer = preferredServerName.isEmpty () ? provider->getPreferredServer () : preferredServerName;
        QList<Video> videos;
        int serverIndex = -1;

        if (!preferredServer.isEmpty ()) {
            for (int i = 0; i < servers.size (); i++) {
                if (servers[i].name == preferredServer) {
                    auto& server = servers.at (i);
                    qDebug() << "Log (ServerList): Using preferred server" << server.name;
                    videos = provider->extractSource(server);
                    if (!videos.isEmpty ()) {
                        serverIndex = i;
                        break;
                    }
                }
            }
            if (serverIndex == -1)
                qDebug() << "Log (ServerList): Preferred server" << preferredServer << "not available for this episode";
        }

        if (serverIndex == -1) {
            for (int i = 0; i < servers.size (); i++) {
                auto& server = servers.at (i);
                videos = provider->extractSource(server);
                if (!videos.isEmpty ()){
                    provider->setPreferredServer (server.name);
                    qDebug() << "Log (ServerList): Using server" << server.name;
                    serverIndex = i;
                    break;
                }
            }
        }

        return { videos, serverIndex};
    }

    QList<Video> load(int index = -1) {
        // auto select a working server
        QList<Video> videos;
        if (index == -1) {
            // videos = autoSelectServer ();
            auto sourceAndIndex = autoSelectServer(m_servers, m_provider);
            videos = sourceAndIndex.first;
            if (!videos.isEmpty ()) {
                m_currentIndex = sourceAndIndex.second;
                emit currentIndexChanged();
            } else {
                qInfo() << "Log (ServerList): Failed to find a working server";
            }
            return videos;
        } else if (index < 0 || index >= m_servers.size ()) return {};

        auto& server = m_servers.at (index);
        qInfo() << "Log (ServerList): Fetching source from server" << server.name;
        videos = m_provider->extractSource(server);
        m_currentIndex = index;
        emit currentIndexChanged();

        if (videos.isEmpty ()) {
            qInfo() << "Log (ServerList): Failed to fetch source from" << server.name;
        } else {
            m_provider->setPreferredServer (server.name);
        }
        return videos;
    }

    void setCurrentIndex(int index) {
        if (index == m_currentIndex) return;
        int previousIndex = m_currentIndex;
        auto videos = load(index);
        if (!videos.isEmpty ()) {
            qInfo() << "Log (ServerList): Fetched source" << videos.first ().videoUrl;
            MpvObject::instance()->open (videos.first ());
        } else {
            m_currentIndex = previousIndex;
            emit currentIndexChanged ();
        }
    }

    int getCurrentIndex() const { return m_currentIndex; }

    QList<VideoServer> servers() const { return m_servers; }
    bool isEmpty() const { return m_servers.isEmpty(); }
    int size() const { return m_servers.size(); }
signals:
    void currentIndexChanged();
private:
    QList<VideoServer> m_servers;
    int m_currentIndex = -1;
    ShowProvider* m_provider = nullptr;

    enum {
        NameRole = Qt::UserRole,
        LinkRole
    };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return m_servers.size();
    };
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || m_servers.isEmpty())
            return QVariant();

        const VideoServer &server = m_servers.at(index.row());
        switch (role) {
        case NameRole:
            return server.name;
            break;
        case LinkRole:
            return QString::fromStdString (server.link.substr (0,22)) + "...";
            break;
        default:
            break;
        }
        return QVariant();
    };
    QHash<int, QByteArray> roleNames() const override{
        QHash<int, QByteArray> names;
        names[NameRole] = "name";
        names[LinkRole] = "link";
        return names;
    };
};



