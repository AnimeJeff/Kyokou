#pragma once
#include <set>
#include "Components/errorhandler.h"
#include "Explorer/Data/showdata.h"
#include <QDir>
#include <QAbstractListModel>
#include <QtConcurrent>
#include <QStandardItemModel>
#include "playlistitem.h"

class PlaylistModel : public QAbstractItemModel
{
//    *.cpp,*.h,
    Q_OBJECT

    Q_PROPERTY(int playlistIndex READ getPlaylistIndex NOTIFY showNameChanged)
    Q_PROPERTY(int playlistItemIndex READ getPlaylistItemIndex NOTIFY showNameChanged)
    Q_PROPERTY(QModelIndex currentIndex READ getCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentItemName READ getCurrentItemName NOTIFY currentIndexChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QUrl launchPath READ getLaunchPath CONSTANT)

    QString getCurrentItemName() const
    {
        if (m_playlists.isEmpty ()) return "";
        return m_playlists.first ()->getDisplayName (m_playlists.first ()->currentIndex);
    }
    int getPlaylistIndex()
    {
        return m_playlistIndex;
    }
    int getPlaylistItemIndex()
    {
        return m_playlists[m_playlistIndex]->currentIndex;
    }

    int m_playlistIndex = 0;
    QList<PlaylistItem*> m_playlists;
    QUrl loadOnlineSource(int playlistIndex, int itemIndex);
    QUrl getLaunchPath()
    {
        return m_launchPath;
    }

    QUrl nextVideoSource;
signals:
    void playlistIndexChanged();
    void playlistItemIndexChanged();

public:
    explicit PlaylistModel(QObject *parent = nullptr) {

    };
    ~PlaylistModel(){ qDeleteAll(m_playlists); }

    QModelIndex getCurrentIndex();
    Q_INVOKABLE bool load(QModelIndex index)
    {
        auto childItem = static_cast<PlaylistItem*>(index.internalPointer ());
        auto parentItem = childItem->parent ();

        if (!parentItem){
            return false;
        }
        int itemIndex = childItem->row ();
        int playlistIndex = m_playlists.indexOf (parentItem);
        play(playlistIndex, itemIndex);
        return true;
    }

    // opens the file to play immediately when application launches
    bool setLaunchPath(const QString& path);
    std::set<std::string> playlistSet;
    /*
     * hashset containing all playlist links which prevents the same playlist from
     * being added to the list of playlists
    */

    Q_INVOKABLE void loadFromEpisodeList(int index);
    Q_INVOKABLE void continueFromLastWatched();
    Q_INVOKABLE void replaceCurrentPlaylist(const QUrl& path);
    void appendPlaylist(const QUrl& path);
    void appendPlaylist(PlaylistItem *playlist);
    void replaceCurrentPlaylist(PlaylistItem *playlist);
    Q_INVOKABLE void play(int playlistIndex, int itemIndex);

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
    bool isLoading()
    {
        return loading;
    }
    void setLoading(bool b)
    {
        loading = b;
        emit loadingChanged ();
    }
    bool loading = false;
    QFutureWatcher<QUrl> m_watcher;
    QUrl m_launchPath;

signals:
    void loadingChanged(void);
    void sourceFetched();
    void currentIndexChanged(void);
    void updatedLastWatchedIndex();
    void showNameChanged(void);
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
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int columnCount(const QModelIndex &parent) const override {return 1;}
};

class ServerListModel: public QAbstractListModel
{

public:
    ServerListModel() = default;
    ~ServerListModel() = default;
    void setServers(QList<VideoServer> servers)
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



    QList<VideoServer> servers() const
    {
        return m_servers;
    }



private:
    enum{
        NameRole = Qt::UserRole,
    };

    QList<VideoServer> m_servers {};
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

