#pragma once
#include <QAbstractListModel>
#include <QObject>
#include <Download/downloadmodel.h>
#include "Explorer/Data/showdata.h"
#include "Explorer/searchresultsmodel.h"
#include "Providers/showprovider.h"
#include "Explorer/episodelistmodel.h"
#include "Player/playlistmodel.h"
#include "Explorer/watchlistmodel.h"

class ShowProvider;



class ShowManager: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(ShowData currentShow READ getCurrentShow NOTIFY currentShowChanged)
    Q_PROPERTY(bool hasCurrentShow READ hasCurrentShow NOTIFY currentShowChanged) //info page cover
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)

    // ShowData is not a QObject so these dynamic data which are mutable must be provided by this proxy class
    Q_PROPERTY(int currentShowListType READ getCurrentShowListType NOTIFY listTypeChanged)
    Q_PROPERTY(bool currentShowIsInWatchList READ isInWatchList NOTIFY listTypeChanged)


    Q_PROPERTY(PlaylistModel *playList READ playlistModel CONSTANT)
    PlaylistModel m_playlistModel{this};
    PlaylistModel *playlistModel() { return &m_playlistModel; }

    Q_PROPERTY(EpisodeListModel *episodeList READ episodeListModel CONSTANT)
    EpisodeListModel m_episodeListModel{this};
    EpisodeListModel *episodeListModel() { return &m_episodeListModel; }

    Q_PROPERTY(WatchListModel *watchList READ watchListModel CONSTANT)
    WatchListModel m_watchListModel{this};
    WatchListModel *watchListModel() { return &m_watchListModel; }

    Q_PROPERTY(SearchResultsModel *explorer READ searchResultsModel CONSTANT)
    SearchResultsModel *searchResultsModel() { return &m_searchResultsModel; }
    SearchResultsModel m_searchResultsModel{this};

    Q_PROPERTY(DownloadModel *downloader READ downloadModel CONSTANT)
    DownloadModel *m_downloadModel = nullptr;
    DownloadModel *downloadModel() { return m_downloadModel; }

    ShowData currentShow{"Undefined", "", "", nullptr};
    Q_PROPERTY(int currentProviderIndex READ getCurrentProviderIndex WRITE setCurrentProviderIndex NOTIFY currentProviderIndexChanged)
    int getCurrentProviderIndex() const {
        return m_currentProviderIndex;
    }
    void setCurrentProviderIndex(int index) {
        if (index == m_currentProviderIndex) return;
        int currentType = m_currentProviderIndex != -1 ? m_currentSearchProvider->getAvailableTypes ()[m_currentSearchTypeIndex] : -1;
        m_currentProviderIndex = index;
        m_currentSearchProvider = m_providers.at (index);
        emit currentProviderIndexChanged();
        int searchTypeIndex = m_currentSearchProvider->getAvailableTypes ().indexOf (currentType);
        m_currentSearchTypeIndex = searchTypeIndex == -1 ? 0 : searchTypeIndex;
        emit currentSearchTypeIndexChanged();


    };
    int m_currentProviderIndex = -1;

    Q_PROPERTY(int currentSearchTypeIndex READ getCurrentSearchTypeIndex WRITE setCurrentSearchTypeIndex NOTIFY currentSearchTypeIndexChanged)
    void setCurrentSearchTypeIndex(int index) {
        if (index == m_currentSearchTypeIndex) return;
        m_currentSearchTypeIndex = index;
        emit currentSearchTypeIndexChanged ();
    };
    int getCurrentSearchTypeIndex() const {
        return m_currentSearchTypeIndex;
    }
    int m_currentSearchTypeIndex = ShowData::ShowType::MOVIE;

    // Q_PROPERTY(int availableListTypes READ getCurrentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QVariant availableShowTypes READ getAvailableShowTypes NOTIFY currentProviderIndexChanged)
    QVariant getAvailableShowTypes() {
        QStringList stringTypes = {"Movie", "Tv Series", "Variety", "Anime", "Documentary", "None"};
        QStringList availableTypes;
        for (auto type : m_currentSearchProvider->getAvailableTypes()){
            availableTypes.push_back (stringTypes[type - 1]);
        }
        return QVariant::fromValue(availableTypes);
    }

private:
    ShowManager(const ShowManager &) = delete;
    ShowManager &operator=(const ShowManager &) = delete;

    QFutureWatcher<void> m_watcher{};

    bool m_loading = false;
    bool isLoading() { return m_loading; }
    void setLoading(bool loading) {
        m_loading = loading;
        emit loadingChanged();
    }
    QTimer m_timeoutTimer{this};
    QString m_cancelReason;

    QList<ShowProvider*> m_providers;
    QHash<QString, ShowProvider *> m_providersMap;

    ShowProvider *m_currentSearchProvider;

public:
    bool hasCurrentShow() { return !currentShow.link.empty(); }
    const ShowData &getCurrentShow() { return currentShow; };

    void setCurrentShow(const ShowData &show);
    bool isInWatchList() { return currentShow.isInWatchList(); } //TODO remove?

    ShowProvider *getProvider(const QString &provider) {
        if (m_providersMap.contains(provider))
            return m_providersMap[provider];
        return nullptr;
    }

    int getCurrentShowListType() const;

signals:
    void currentShowChanged(void);
    void listTypeChanged(void);
    void loadingChanged(void);
    void searchProviderChanged(void);
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
        if (fromWatchList) {
            auto showJson = m_watchListModel.loadShow(index);
            std::string link = showJson["link"].get<std::string>();
            QString title = QString::fromStdString(showJson["title"].get<std::string>());
            QString coverUrl = QString::fromStdString(showJson["cover"].get<std::string>());
            ShowProvider *provider = getProvider(QString::fromStdString(showJson["provider"].get<std::string>()));
            int lastWatchedIndex = showJson["lastWatchedIndex"].get<int>();
            ShowData show(title, link, coverUrl, provider, "", m_watchListModel.getCurrentListType(), lastWatchedIndex);
            setCurrentShow (show);
        } else {
            setCurrentShow(m_searchResultsModel.at(index));
        }
    }
    void cancel();
    void addCurrentShowToLibrary(int listType)
    {
        //todo logic
        m_watchListModel.add (currentShow, listType); //either changes the list type or adds to library
        emit listTypeChanged ();
    }
    void removeCurrentShowFromLibrary()
    {
        m_watchListModel.remove (currentShow);
        emit listTypeChanged ();
    }
    void checkUpdateLastWatchedIndex() {
        PlaylistItem *currentPlaylist = m_playlistModel.getCurrentPlaylist();
        if (!currentShow.playlist || !currentPlaylist) {//TODO local file
            qDebug() << "error: playlist returned nullptr";
            return;
        }

        if (currentShow.playlist->link == currentPlaylist->link) {
            currentShow.playlist->currentIndex = currentPlaylist->currentIndex;
            m_episodeListModel.updateLastWatchedIndex();
        }
        if (nlohmann::json *libraryJsonPtr = currentPlaylist->getJsonPtr()) {
            libraryJsonPtr->at("lastWatchedIndex") = currentPlaylist->currentIndex;
            m_watchListModel.save ();
        }



    }
    void playFromEpisodeList(int index)
    {
        index = m_episodeListModel.getIsReversed () ? currentShow.playlist->count () - index - 1: index;
        Q_ASSERT(index > -1 && index < currentShow.playlist->count ());

        m_playlistModel.replaceCurrentPlaylist (currentShow.playlist);
        m_playlistModel.play (0, index);
    }
    void continueWatching()
    {
        int index = m_episodeListModel.getContinueIndex();
        if (index < 0 || index >= currentShow.playlist->count ()) {
            qDebug() << "Error: Invalid continue index";
            return;
        }
        playFromEpisodeList(index);
    }
    void downloadCurrentShow(int startIndex, int count = 1)
    {
        startIndex = m_episodeListModel.getIsReversed () ? currentShow.playlist->count () - startIndex - 1: startIndex;
        Q_ASSERT(startIndex > 0 && startIndex < currentShow.playlist->count ());
        m_downloadModel->downloadShow (currentShow, currentShow.provider, startIndex, count); //TODO
    };

public:
    ShowManager(ShowManager &&) = delete;
    ShowManager &operator=(ShowManager &&) = delete;
    explicit ShowManager(QString launchPath);
    ~ShowManager();



private:
    enum {
        NameRole = Qt::UserRole,
        // IconRole, //TODO add icons next to providers
    };
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
};
