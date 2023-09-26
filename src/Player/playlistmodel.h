#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include "Providers/showprovider.h"
#include <QDir>
#include <QAbstractListModel>
#include <QtConcurrent>
#include <QStandardItemModel>
#include "Player/mpv/mpvObject.h"
#include "Components/errorhandler.h"
#include "Explorer/showmanager.h"
#include "playlistitem.h"

class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString currentItemName READ getCurrentItemName NOTIFY currentIndexChanged)
    Q_PROPERTY(QString showName READ getTitle NOTIFY showNameChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString onLaunchFile READ getPlayOnLaunchFile CONSTANT)
    QString getTitle()
    {
        return "";
    }
    QString getCurrentItemName() const
    {
        if (rootItem->isEmpty ()) return "";
        return rootItem->first ()->getItemName (rootItem->currentIndex);
    }
    std::shared_ptr<PlaylistItem> rootItem;
public:
    explicit PlaylistModel(QObject *parent = nullptr) : QAbstractListModel(parent)
    {

        rootItem = std::make_shared<PlaylistItem> ("root",-1);
    };
    ~PlaylistModel(){}

    QString loadOnlineSource(int playlistIndex, int itemIndex);

    // opens the file to play immediately when application launches
    void setLaunchFile(const QString& path);
    void setLaunchFolder(const QString& path);
    QString getPlayOnLaunchFile()
    {
        return m_onLaunchFile;
    }

    std::set<std::string> playlistSet;
    /*
     * hashset containing all playlist links which prevents the same playlist from
     * being added to the list of playlists
    */

    Q_INVOKABLE void loadFolder(const QUrl& path); // loading playlist from folder

    void appendPlaylist(const ShowData& show,nlohmann::json* json);

    void appendPlaylist(const QUrl& path);

    void replaceCurrentPlaylist(const ShowData& show,nlohmann::json* json);

    Q_INVOKABLE void play(int playlistIndex, int itemIndex);
    int m_currentPlaylistIndex;

    //  Traversing the playlist
    Q_INVOKABLE void loadOffset(int offset);
    Q_INVOKABLE void playNextItem()
    {
        loadOffset (1);
    }
    Q_INVOKABLE void playPrecedingItem()
    {
        loadOffset(-1);
    }
private:
    inline bool isLoading()
    {
        return loading;
    }
    void setLoading(bool b)
    {
        loading = b;
        emit loadingChanged ();
    }
private:
    QFutureWatcher<void> m_watcher;
    QString m_onLaunchFile;
    bool loading = false;

signals:
    void loadingChanged(void);
    void sourceFetched();
    void currentIndexChanged(void);
    void updatedLastWatchedIndex();
    void showNameChanged(void);
    void encounteredError(QString error);
public slots:
    void syncList(const ShowData& show,nlohmann::json* json);
public:
    enum
    {
        TitleRole = Qt::UserRole,
        NumberRole,
        NumberTitleRole
    };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
};

class ServerListModel: public QAbstractListModel
{

public:
    ServerListModel() = default;
    ~ServerListModel() = default;
    void setServers(QVector<VideoServer> servers)
    {
        m_servers = servers;
        //testServers ();
    }
    void invalidateServer(int index)
    {
        //server is not working
        if (index > m_servers.size () || index < 0) return;
        m_servers[index].working = false;
    }

    void testServers()
    {
        QFuture<void> future = QtConcurrent::run ([this](){
            for (auto& server : m_servers)
            {
                // extract source first
                if(NetworkClient::get (server.source.toStdString ()).code != 200)
                {
                    server.working = false;
                };
            }
        });
    }

    QVector<VideoServer> servers() const
    {
        return m_servers;
    }

    //    VideoServer getFirstWorkingServer() const
    //    {
    //        for (const auto& server : m_servers)
    //        {
    //            if(server.working){
    //                return server;
    //            }
    //        }
    //        return "";
    //    }


private:
    enum{
        NameRole = Qt::UserRole,
    };

    QVector<VideoServer> m_servers {};
public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return m_servers.size();
    };
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid() || m_servers.isEmpty ())
            return QVariant();

        const VideoServer& server = m_servers.at (index.row());
        switch (role)
        {
        case NameRole:
            return server.name;
            break;
        default:
            break;
        }
        return QVariant();
    };
    QHash<int, QByteArray> roleNames() const override;
};

#endif // PLAYLISTMODEL_H
