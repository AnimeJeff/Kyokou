#pragma once
#include <QAbstractListModel>
#include <QObject>
#include <Components/cursor.h>
#include "Controllers/searchresultsmodel.h"
#include "Controllers/downloadmodel.h"

#include "Controllers/playlistmodel.h"
#include "Controllers/watchlistmodel.h"
#include "showmanager.h"

#include "Providers/showprovider.h"

class Application: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(ShowManager *currentShow READ getCurrentShow CONSTANT)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(int currentProviderIndex READ getCurrentProviderIndex WRITE setCurrentProviderIndex NOTIFY currentProviderIndexChanged)
    Q_PROPERTY(int currentSearchTypeIndex READ getCurrentShowTypeIndex WRITE setCurrentShowTypeIndex NOTIFY currentShowTypeIndexChanged)
    Q_PROPERTY(QVariant availableShowTypes READ getAvailableShowTypes NOTIFY currentProviderIndexChanged)

    Q_PROPERTY(Cursor *cursor READ cursor CONSTANT)
    Q_PROPERTY(PlaylistModel *playList READ playlistModel CONSTANT)
    Q_PROPERTY(WatchListModel *watchList READ watchListModel CONSTANT)
    Q_PROPERTY(DownloadModel *downloader READ downloadModel CONSTANT)
    Q_PROPERTY(SearchResultsModel *explorer READ searchResultsModel CONSTANT)
private:
    SearchResultsModel *searchResultsModel() { return &m_searchResultsModel; }
    SearchResultsModel m_searchResultsModel{this};
    DownloadModel *m_downloader = nullptr;
    DownloadModel *downloadModel() { return m_downloader; }
    WatchListModel m_watchListModel{this};
    WatchListModel *watchListModel() { return &m_watchListModel; }
    PlaylistModel m_playlist;
    PlaylistModel *playlistModel() { return &m_playlist; }
    Cursor m_cursor {this};
    Cursor *cursor() { return &m_cursor; }

    ShowManager m_showManager {this};
    ShowManager *getCurrentShow() { return &m_showManager; };

    QList<ShowProvider*> m_providers;
    QHash<QString, ShowProvider*> m_providersMap;
    ShowProvider *m_currentSearchProvider;

    int getCurrentProviderIndex() const { return m_currentProviderIndex; }
    void setCurrentProviderIndex(int index);;
    int m_currentProviderIndex = -1;

    void setCurrentShowTypeIndex(int index);;
    int getCurrentShowTypeIndex() const { return m_currentShowTypeIndex; }
    int m_currentShowTypeIndex = 0;

    bool m_loading = false;
    bool isLoading() { return m_loading; }
    void setLoading(bool loading) {
        m_loading = loading;
        emit loadingChanged();
    }
    QList<int> m_availableTypes;
    QVariant getAvailableShowTypes() {
        QStringList stringTypes = {"Movie", "Tv Series", "Variety", "Anime", "Documentary", "None"};
        QStringList availableTypes;
        for (auto type : m_availableTypes){
            availableTypes.push_back (stringTypes[type - 1]);
        }
        return QVariant::fromValue(availableTypes);
    }
private:
    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    QFutureWatcher<void> m_watcher;
    QTimer m_timeoutTimer{this};
    QString m_cancelReason;

public:
    Q_INVOKABLE void cycleProviders();
    Q_INVOKABLE void search(const QString& query, int page);
    Q_INVOKABLE void latest(int page);
    Q_INVOKABLE void popular(int page);
    Q_INVOKABLE void loadShow(int index, bool fromWatchList);
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void addCurrentShowToLibrary(int listType);
    Q_INVOKABLE void removeCurrentShowFromLibrary();
    Q_INVOKABLE void playFromEpisodeList(int index);
    Q_INVOKABLE void playFromFolder(const QUrl& pathUrl) {
        auto playlist = PlaylistItem::fromUrl (pathUrl);
        if (playlist) {
            m_playlist.replaceCurrentPlaylist (playlist);
            m_playlist.play();
        }
    }
    Q_INVOKABLE void continueWatching();
    Q_INVOKABLE void downloadCurrentShow(int startIndex, int count = 1);;
    Q_INVOKABLE void updateLastPlayTime() {
        // Update the last play time
        auto lastPlaylist = m_playlist.getCurrentPlaylist();
        if (!lastPlaylist ||
            lastPlaylist->hasSameLink(m_showManager.getPlaylist ())) return;

        qDebug() << "Updating last playlist" << lastPlaylist->link << MpvObject::instance ()->time ();
        auto playlistType = lastPlaylist->at (lastPlaylist->currentIndex)->type;

        if (lastPlaylist->isLoadedFromFolder ())
            lastPlaylist->updateHistoryFile (MpvObject::instance ()->time ());
        else {
            m_watchListModel.updateLastPlayTime (lastPlaylist->link, MpvObject::instance ()->time ());
        }
    }
signals:
    void loadingChanged(void);
    void currentShowTypeIndexChanged(void);
    void currentProviderIndexChanged(void);
private slots:
    void updateLastWatchedIndex();

public:
    Application(Application &&) = delete;
    Application &operator=(Application &&) = delete;
    explicit Application(const QString &launchPath);
    ~Application();

private:
    enum {
        NameRole = Qt::UserRole,
        // IconRole, //TODO add icons next to providers
    };
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
};
