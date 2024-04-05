#pragma once
#include <QAbstractListModel>
#include <QDir>
#include <QStandardItemModel>
#include <QtConcurrent>
#include "Data/showdata.h"
#include "Models/serverlistmodel.h"
#include "Data/playlistitem.h"

class PlaylistModel : public QAbstractItemModel {
    Q_OBJECT
    Q_PROPERTY(QModelIndex currentIndex READ getCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentItemName READ getCurrentItemName NOTIFY currentIndexChanged)
    Q_PROPERTY(ServerListModel *serverList READ getServerList CONSTANT)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)


private:
    bool m_isLoading = false;
    bool isLoading() { return m_isLoading; }
    void setIsLoading(bool value) {
        m_isLoading = value;
        emit isLoadingChanged();
    }

    QFileSystemWatcher m_folderWatcher;
    QFutureWatcher<void> m_watcher;
    ServerListModel m_serverList;
    std::unique_ptr<PlaylistItem> m_rootPlaylist = std::make_unique<PlaylistItem>("root", nullptr, "/");

    ServerListModel *getServerList() { return &m_serverList; }
    QString getCurrentItemName() const {
        auto currentPlaylist = m_rootPlaylist->getCurrentItem ();
        if (!currentPlaylist) return "";
        return currentPlaylist->getDisplayNameAt (currentPlaylist->currentIndex);
    }

    void play(int playlistIndex, int itemIndex);

    bool registerPlaylist(PlaylistItem *playlist);
    void deregisterPlaylist(PlaylistItem *playlist);
    QSet<QString> playlistSet;
public:
    explicit PlaylistModel(const QString &launchPath, QObject *parent = nullptr);

    ~PlaylistModel() = default;

    Q_INVOKABLE void loadIndex(QModelIndex index);
    Q_INVOKABLE void pasteOpen();
    QModelIndex getCurrentIndex();

    // Hashset containing all playlist links
    // prevents the same playlist being added


    //  Traversing the playlist
    Q_INVOKABLE bool tryPlay(int playlistIndex = -1, int itemIndex = -1);
    Q_INVOKABLE void loadOffset(int offset);
    Q_INVOKABLE void playNextItem() { loadOffset(1); }
    Q_INVOKABLE void playPrecedingItem() { loadOffset(-1); }

private slots:
    void onLocalDirectoryChanged(const QString &path);
public:
    void appendPlaylist(PlaylistItem *playlist);
    void replaceCurrentPlaylist(PlaylistItem *playlist);
    PlaylistItem *getCurrentPlaylist() const {
        // qDebug() << "size" << m_rootPlaylist->size () << "index" << m_rootPlaylist->currentIndex;
        return m_rootPlaylist->getCurrentItem ();
    }

public:
    Q_SIGNAL void isLoadingChanged(void);
    Q_SIGNAL void currentIndexChanged(void);

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

