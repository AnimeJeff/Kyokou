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
// class ShowProvider;

class Application: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(ShowManager *currentShow READ getCurrentShow CONSTANT)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(int currentProviderIndex READ getCurrentProviderIndex WRITE setCurrentProviderIndex NOTIFY currentProviderIndexChanged)
    Q_PROPERTY(int currentSearchTypeIndex READ getCurrentSearchTypeIndex WRITE setCurrentSearchTypeIndex NOTIFY currentSearchTypeIndexChanged)
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
    PlaylistModel m_playlist{this};
    PlaylistModel *playlistModel() { return &m_playlist; }
    Cursor *cursor() { return &m_cursor; }
    Cursor m_cursor{}; // todo

    ShowManager m_showManager;
    ShowManager *getCurrentShow() { return &m_showManager; };

    QList<ShowProvider*> m_providers;
    QHash<QString, ShowProvider*> m_providersMap;
    ShowProvider *m_currentSearchProvider;


    int getCurrentProviderIndex() const {
        return m_currentProviderIndex;
    }
    void setCurrentProviderIndex(int index) {
        if (index == m_currentProviderIndex) return;
        int currentType = m_currentProviderIndex != -1 ? m_currentSearchProvider->getAvailableTypes ()[m_currentSearchTypeIndex] : -1;
        m_currentProviderIndex = index;
        m_currentSearchProvider = m_providers.at (index);

        emit currentProviderIndexChanged();
        int searchTypeIndex = searchTypeIndex == -1 ? -1 : m_currentSearchProvider->getAvailableTypes ().indexOf (currentType);
        m_currentSearchTypeIndex = searchTypeIndex == -1 ? 0 : searchTypeIndex;
        emit currentSearchTypeIndexChanged();
    };
    int m_currentProviderIndex = -1;

    void setCurrentSearchTypeIndex(int index) {
        if (index == m_currentSearchTypeIndex) return;
        m_currentSearchTypeIndex = index;
        emit currentSearchTypeIndexChanged ();
    };
    int getCurrentSearchTypeIndex() const {
        return m_currentSearchTypeIndex;
    }
    int m_currentSearchTypeIndex = ShowData::ShowType::MOVIE;

    QVariant getAvailableShowTypes() {
        QStringList stringTypes = {"Movie", "Tv Series", "Variety", "Anime", "Documentary", "None"};
        QStringList availableTypes;
        for (auto type : m_currentSearchProvider->getAvailableTypes()){
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


    bool m_loading = false;
    bool isLoading() { return m_loading; }
    void setLoading(bool loading) {
        m_loading = loading;
        emit loadingChanged();
    }


signals:
    void loadingChanged(void);
    void currentSearchTypeIndexChanged(void);
    void currentProviderIndexChanged(void);
public slots:
    void search(const QString& query, int page){
        int type = m_currentSearchProvider->getAvailableTypes()[m_currentSearchTypeIndex];
        m_searchResultsModel.search (query, page, type, m_currentSearchProvider);
    }
    void latest(int page) {
        int type = m_currentSearchProvider->getAvailableTypes()[m_currentSearchTypeIndex];
        m_searchResultsModel.latest (page, type, m_currentSearchProvider);
    }
    void popular(int page){
        int type = m_currentSearchProvider->getAvailableTypes()[m_currentSearchTypeIndex];
        m_searchResultsModel.popular (page,type,m_currentSearchProvider);
    }
    void loadShow(int index, bool fromWatchList) {
        setLoading (true);
        if (fromWatchList) {
            auto showJson = m_watchListModel.loadShow(index);
            if (showJson.isEmpty ()) return;
            QString providerName = showJson["provider"].toString ();
            if (!m_providersMap.contains(providerName)) return;
            auto provider = m_providersMap[providerName];
            std::string link = showJson["link"].toString ().toStdString ();
            QString title = showJson["title"].toString ();
            QString coverUrl = showJson["cover"].toString ();
            int lastWatchedIndex = showJson["lastWatchedIndex"].toInt ();
            auto show = ShowData(title, link, coverUrl, provider, "", -1, lastWatchedIndex);
            m_showManager.setShow(show);
            m_showManager.setListType (m_watchListModel.getCurrentListType());
        } else {
            auto show = m_searchResultsModel.at(index);
            m_watchListModel.syncShow (show);
            m_showManager.setShow(show);
        }
    }
    void cancel();
    void addCurrentShowToLibrary(int listType)
    {
        //todo logic
        m_watchListModel.add (m_showManager.getShow (), listType); //either changes the list type or adds to library
        m_showManager.setListType(listType);
    }
    void removeCurrentShowFromLibrary()
    {
        m_watchListModel.remove (m_showManager.getShow ());
        m_showManager.setListType(-1);
    }
    void updateLastWatchedIndex() {
        PlaylistItem *currentPlaylist = m_playlist.getCurrentPlaylist();
        auto showPlaylist = m_showManager.getPlaylist ();
        if (!showPlaylist || !currentPlaylist) {//TODO local file
            qDebug() << "error: playlist returned nullptr";
            return;
        }

        if (showPlaylist->link == currentPlaylist->link) {
            m_showManager.updateLastWatchedIndex (currentPlaylist->currentIndex);
        }

        //todo change playlist link to qstring
        m_watchListModel.updateLastWatchedIndex (QString::fromStdString (currentPlaylist->link), currentPlaylist->currentIndex);

    }
    void playFromEpisodeList(int index)
    {
        m_playlist.replaceCurrentPlaylist (m_showManager.getPlaylist ());
        m_playlist.play (0, m_showManager.correctIndex(index));
    }
    void continueWatching()
    {
        int index = m_showManager.getContinueIndex();
        if (index < 0) {
            qDebug() << "Error: Invalid continue index";
            return;
        }
        playFromEpisodeList(index);
    }
    void downloadCurrentShow(int startIndex, int count = 1)
    {
        m_downloader->downloadShow (m_showManager.getShow (), m_showManager.correctIndex(startIndex), count); //TODO
    };

public:
    Application(Application &&) = delete;
    Application &operator=(Application &&) = delete;
    explicit Application(QString launchPath);
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
