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
    Q_PROPERTY(QModelIndex currentIndex READ getCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentItemName READ getCurrentItemName NOTIFY currentIndexChanged)
    Q_PROPERTY(ServerListModel *serverList READ getServerList CONSTANT)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)

    QString getCurrentItemName() const {
        auto currentPlaylist = m_rootPlaylist->currentItem ();
        if (!currentPlaylist) return "";
        return currentPlaylist->getDisplayNameAt (currentPlaylist->currentIndex);
    }

private:
    bool loading = false;
    bool isLoading() { return loading; }
    void setLoading(bool value) {
        loading = value;
        emit loadingChanged();
    }

    QFileSystemWatcher m_folderWatcher;
    QFutureWatcher<QList<Video>> m_watcher;
    ServerListModel m_serverList;
    std::unique_ptr<PlaylistItem> m_rootPlaylist = std::make_unique<PlaylistItem>("root", nullptr, "", nullptr);

   ServerListModel *getServerList() { return &m_serverList; }

    QList<Video> loadOnlineSource(int playlistIndex, int itemIndex);

    void registerPlaylist(PlaylistItem *playlist) {
        playlistSet.insert(playlist->link);

        // Watch playlist path if local folder
        if (playlist->isLoadedFromFolder ()) {
            m_folderWatcher.addPath (playlist->link);
        }
    }
    void deregisterPlaylist(PlaylistItem *playlist) {
        playlistSet.remove(playlist->link);

        // Unwatch playlist path if local folder
        if (playlist->isLoadedFromFolder ()) {
            m_folderWatcher.removePath (playlist->link);
        }
    }
public:
    explicit PlaylistModel(const QString &launchPath, QObject *parent = nullptr) {
        // Opens the file to play immediately when application launches

        connect (&m_folderWatcher, &QFileSystemWatcher::directoryChanged, this, &PlaylistModel::onLocalDirectoryChanged);

        if (!launchPath.isEmpty ()) {
            auto url = QUrl::fromUserInput(launchPath);
            auto playlist = PlaylistItem::fromUrl(url);
            if (playlist) {
                replaceCurrentPlaylist(playlist);
                qDebug() << "Log (Playlist): Successfully opened launch path";
            }
        }
    };

    ~PlaylistModel() = default;


    Q_INVOKABLE void load(QModelIndex index) {
        auto childItem = static_cast<PlaylistItem *>(index.internalPointer());
        auto parentItem = childItem->parent();
        if (parentItem == m_rootPlaylist.get ()) return;
        int itemIndex = childItem->row();
        int playlistIndex = m_rootPlaylist->indexOf(parentItem);
        play(playlistIndex, itemIndex);
    }
    Q_INVOKABLE void pasteOpen() {
        QString clipboardText = QGuiApplication::clipboard()->text();
        qInfo() << "Log (mpv): Pasting" << clipboardText;
        MpvObject::instance ()->showText (QByteArray("Pasting ") + clipboardText.toUtf8 ());
        if (clipboardText.endsWith(".vtt")) {
            MpvObject::instance ()->addSubtitle(clipboardText);
            MpvObject::instance ()->setSubVisible(true);
        } else {
            auto playlist = PlaylistItem::fromUrl(QUrl::fromUserInput (clipboardText));
            if (playlist) {
                replaceCurrentPlaylist(playlist);
                play ();
            }
        }
    }
    QModelIndex getCurrentIndex();

    // Hashset containing all playlist links
    // prevents the same playlist being added
    QSet<QString> playlistSet;
    Q_INVOKABLE bool play(int playlistIndex = -1, int itemIndex = -1);
    //  Traversing the playlist
    Q_INVOKABLE void loadOffset(int offset);
    Q_INVOKABLE void playNextItem() { loadOffset(1); }
    Q_INVOKABLE void playPrecedingItem() { loadOffset(-1); }



private slots:
    void onLocalDirectoryChanged(const QString &path) {
        // TODO
        qInfo() << "Log (Playlist): Path" << path << "changed";

    }
public:
    void appendPlaylist(const QUrl &path);
    void appendPlaylist(PlaylistItem *playlist);
    void replaceCurrentPlaylist(PlaylistItem *playlist);
    PlaylistItem *getCurrentPlaylist() const { return m_rootPlaylist->currentItem (); }

public:
    Q_SIGNAL void loadingChanged(void);
    Q_SIGNAL void currentIndexChanged(void);
    Q_SIGNAL void sourceFetched(void);
    Q_SIGNAL void updatedLastWatchedIndex(void);
    Q_SIGNAL void showNameChanged(void);

private:
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

