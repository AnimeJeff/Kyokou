#pragma once
#include "Components/errorhandler.h"
#include "Explorer/Data/showdata.h"
#include "Explorer/serverlistmodel.h"
#include "playlistitem.h"
#include <QAbstractListModel>
#include <QDir>
#include <QStandardItemModel>
#include <QtConcurrent>
#include <set>


class PlaylistModel : public QAbstractItemModel {
    Q_OBJECT
    Q_PROPERTY(int playlistIndex READ getPlaylistIndex NOTIFY showNameChanged)
    Q_PROPERTY(int playlistItemIndex READ getPlaylistItemIndex NOTIFY showNameChanged)
    Q_PROPERTY(QModelIndex currentIndex READ getCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentItemName READ getCurrentItemName NOTIFY currentIndexChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QUrl launchPath READ getLaunchPath CONSTANT)
    Q_PROPERTY(ServerListModel *serverList READ getServerList CONSTANT)

    QString getCurrentItemName() const {
        if (m_playlists.isEmpty()) return "";
        return m_playlists.first()->getDisplayName(m_playlists.first()->currentIndex);
    }
    ServerListModel m_serverList;
private:
    bool loading = false;
    bool isLoading() { return loading; }
    void setLoading(bool b) {
        loading = b;
        emit loadingChanged();
    }

    QFutureWatcher<QUrl> m_watcher;
    QUrl m_launchPath;

    int m_playlistIndex = 0;
    QList<PlaylistItem *> m_playlists;

    QUrl loadOnlineSource(int playlistIndex, int itemIndex);
    QUrl getLaunchPath() { return m_launchPath; }
    QUrl nextVideoSource; // TODO

signals:
    void playlistIndexChanged();
    void playlistItemIndexChanged();

public:
    explicit PlaylistModel(QObject *parent = nullptr){  };
    ~PlaylistModel() {
        for (auto &playlist : m_playlists) {
            if (--playlist->useCount == 0)
                delete playlist;
        }

    }

    Q_INVOKABLE QModelIndex getCurrentIndex();
    ServerListModel *getServerList() { return &m_serverList; }

    Q_INVOKABLE bool load(QModelIndex index) {
        auto childItem = static_cast<PlaylistItem *>(index.internalPointer());
        auto parentItem = childItem->parent();

        if (!parentItem) return false;
        int itemIndex = childItem->row();
        int playlistIndex = m_playlists.indexOf(parentItem);
        play(playlistIndex, itemIndex);
        return true;
    }

    // opens the file to play immediately when application launches
    bool setLaunchPath(const QString &path);
    std::set<std::string> playlistSet;
    /*
   * hashset containing all playlist links which prevents the same playlist from
   * being added to the list of playlists
   */

public slots:
    void replaceCurrentPlaylist(const QUrl &path);
    void play(int playlistIndex, int itemIndex);

    //  Traversing the playlist
    void loadOffset(int offset);
    void playNextItem() { loadOffset(1); }
    void playPrecedingItem() { loadOffset(-1); }

    //not a slot?
    int getPlaylistIndex() { return m_playlistIndex; }
    int getPlaylistItemIndex() {
        return m_playlists[m_playlistIndex]->currentIndex;
    }

public:
    void appendPlaylist(const QUrl &path);
    void appendPlaylist(PlaylistItem *playlist);
    void replaceCurrentPlaylist(PlaylistItem *playlist);
    PlaylistItem *getCurrentPlaylist() const {
        return m_playlists[m_playlistIndex];
    }




public:
    Q_SIGNAL void loadingChanged(void);
    Q_SIGNAL void currentIndexChanged(void);
    Q_SIGNAL void sourceFetched();
    Q_SIGNAL void updatedLastWatchedIndex();
    Q_SIGNAL void showNameChanged(void);

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
    int columnCount(const QModelIndex &parent) const override { return 1; }
};

