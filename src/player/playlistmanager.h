#pragma once
#include <QDir>
#include <QStandardItemModel>
#include <QtConcurrent>
#include "data/showdata.h"
#include "player/serverlistmodel.h"
#include "data/playlistitem.h"

class PlaylistManager : public QAbstractItemModel {
    Q_OBJECT
    Q_PROPERTY(QModelIndex currentIndex READ getCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentItemName READ getCurrentItemName NOTIFY currentIndexChanged)
    Q_PROPERTY(ServerListModel *serverList READ getServerList CONSTANT)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

private:
    bool m_isLoading = false;
    bool isLoading() { return m_isLoading; }
    void setIsLoading(bool value);

    QFileSystemWatcher m_folderWatcher;
    QFutureWatcher<void> m_watcher;
    ServerListModel m_serverList;
    PlaylistItem *m_rootPlaylist = new PlaylistItem("root", nullptr, "/");
    QSet<QString> playlistSet; // Prevents the playlist with the same being added

    ServerListModel *getServerList() { return &m_serverList; }
    QString getCurrentItemName() const;
    QModelIndex getCurrentIndex() const;

    bool registerPlaylist(PlaylistItem *playlist);
    void deregisterPlaylist(PlaylistItem *playlist);

    void play(int playlistIndex, int itemIndex);
    Q_SLOT void onLocalDirectoryChanged(const QString &path);
public:
    explicit PlaylistManager(QObject *parent = nullptr);

    ~PlaylistManager() { delete m_rootPlaylist; }
    void appendPlaylist(PlaylistItem *playlist);
    void replaceCurrentPlaylist(PlaylistItem *playlist);
    PlaylistItem *getCurrentPlaylist() const { return m_rootPlaylist->getCurrentItem (); }

    Q_INVOKABLE void openUrl(const QUrl &url, bool playUrl);
    Q_INVOKABLE void loadIndex(QModelIndex index);
    Q_INVOKABLE void pasteOpen();
    //  Traversing the playlist
    Q_INVOKABLE bool tryPlay(int playlistIndex = -1, int itemIndex = -1);
    Q_INVOKABLE void loadOffset(int offset);
    Q_INVOKABLE void playNextItem() { loadOffset(1); }
    Q_INVOKABLE void playPrecedingItem() { loadOffset(-1); }

    Q_SIGNAL void isLoadingChanged(void);
    Q_SIGNAL void currentIndexChanged(void);
    Q_SIGNAL void aboutToPlay(void);


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

