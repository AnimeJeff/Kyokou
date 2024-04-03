#pragma once
#include <QAbstractListModel>
#include <QDir>
#include <QStandardItemModel>
#include <QtConcurrent>
#include "Data/showdata.h"
#include "Controllers/serverlistmodel.h"
#include "Data/playlistitem.h"

class PlaylistModel : public QAbstractItemModel {
    Q_OBJECT
    // Q_PROPERTY(int playlistIndex READ getPlaylistIndex NOTIFY showNameChanged)
    // Q_PROPERTY(int playlistItemIndex READ getPlaylistItemIndex NOTIFY showNameChanged)
    Q_PROPERTY(QModelIndex currentIndex READ getCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentItemName READ getCurrentItemName NOTIFY currentIndexChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    // Q_PROPERTY(QUrl launchPath READ getLaunchPath CONSTANT)
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

    QFutureWatcher<QList<Video>> m_watcher;

    int m_playlistIndex = 0;
    QList<PlaylistItem *> m_playlists;

    QList<Video> loadOnlineSource(int playlistIndex, int itemIndex);

public:
    explicit PlaylistModel(const QString &launchPath, QObject *parent = nullptr) {

        // Opens the file to play immediately when application launches
        if (!launchPath.isEmpty ()) {
            auto playlist = PlaylistItem::fromUrl(QUrl::fromUserInput(launchPath));
            if (playlist) {
                replaceCurrentPlaylist(playlist);
                qDebug() << "Log (Playlist): Successfully opened launch path";
            }
        }
    };

    ~PlaylistModel() {
        for (auto &playlist : m_playlists) {
            if (--playlist->useCount == 0)
                delete playlist;
        }

    }

    Q_INVOKABLE QModelIndex getCurrentIndex();
    ServerListModel *getServerList() { return &m_serverList; }

    Q_INVOKABLE void load(QModelIndex index) {
        auto childItem = static_cast<PlaylistItem *>(index.internalPointer());
        auto parentItem = childItem->parent();

        if (!parentItem) return;
        int itemIndex = childItem->row();
        int playlistIndex = m_playlists.indexOf(parentItem);
        play(playlistIndex, itemIndex);
    }
    Q_INVOKABLE void pasteOpen() {
        QString clipboardText = QGuiApplication::clipboard()->text();
        qInfo() << "Log (mpv): Pasting" << clipboardText;
        MpvObject::instance ()->showText (QByteArray("Pasting ") + clipboardText.toUtf8 ());
        // addSubtitle ()
        if (clipboardText.endsWith(".vtt")) {
            MpvObject::instance ()->addSubtitle(clipboardText);
            MpvObject::instance ()->setSubVisible(true);
        } else {
            auto playlist = PlaylistItem::fromUrl(clipboardText);
            if (playlist) {
                replaceCurrentPlaylist(playlist);
                play ();
            }
        }
    }

    // Hashset containing all playlist links
    // prevents the same playlist being added
    QSet<QString> playlistSet;

public slots:
    // void replaceCurrentPlaylist(const QUrl &path);
    bool play(int playlistIndex = -1, int itemIndex = -1);

    //  Traversing the playlist
    void loadOffset(int offset);
    void playNextItem() { loadOffset(1); }
    void playPrecedingItem() { loadOffset(-1); }

public:
    void appendPlaylist(const QUrl &path);
    void appendPlaylist(PlaylistItem *playlist);
    void replaceCurrentPlaylist(PlaylistItem *playlist);
    PlaylistItem *getCurrentPlaylist() const {
        if (m_playlists.isEmpty () || m_playlistIndex < 0 ||
            m_playlistIndex > m_playlists[m_playlistIndex]->size ())
            return nullptr;

        return m_playlists[m_playlistIndex];
    }




public:
    Q_SIGNAL void loadingChanged(void);
    Q_SIGNAL void currentIndexChanged(void);
    Q_SIGNAL void sourceFetched(void);
    Q_SIGNAL void updatedLastWatchedIndex(void);
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

