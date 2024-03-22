#ifndef SERVERLISTMODEL_H
#define SERVERLISTMODEL_H

#include "Player/playlistitem.h"
#include <QAbstractListModel>



class ServerListModel : public QAbstractListModel {

public:
    ServerListModel() = default;
    ~ServerListModel() = default;
    void setServers(QList<VideoServer> servers) {
        m_servers = servers;
        emit layoutChanged();
        // testServers ();
    }
    void invalidateServer(int index) {
        // server is not working
        if (index > m_servers.size() || index < 0)
            return;
        m_servers[index].working = false;
    }

    QList<VideoServer> servers() const { return m_servers; }
    bool isEmpty() const {
        return m_servers.isEmpty ();
    }
     bool count() const {
        return m_servers.size ();
    }
    const VideoServer& at(int index) const
    {
        return m_servers.at (index);
    }
private:
    enum {
        NameRole = Qt::UserRole,
    };

    QList<VideoServer> m_servers;

public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return m_servers.size();
    };
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override {
        if (!index.isValid() || m_servers.isEmpty())
            return QVariant();

        const VideoServer &server = m_servers.at(index.row());
        switch (role) {
        case NameRole:
            return server.name;
            break;
        default:
            break;
        }
        return QVariant();
    };
    QHash<int, QByteArray> roleNames() const override{
        QHash<int, QByteArray> names;
        names[NameRole] = "name";
        return names;
    };
};



#endif // SERVERLISTMODEL_H
