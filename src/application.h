#ifndef APPLICATION_H
#define APPLICATION_H

#include "models/downloadmodel.h"
#include "models/episodelistmodel.h"
#include "models/playlistmodel.h"
#include "models/searchresultsmodel.h"
#include "models/watchlistmodel.h"
#include "showmanager.h"
#include <QAbstractListModel>


class Application : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PlaylistModel* playlistModel READ playlistModel CONSTANT)
    Q_PROPERTY(EpisodeListModel* episodeListModel READ episodeListModel CONSTANT)
    Q_PROPERTY(SearchResultsModel* showExplorer READ searchResultsModel CONSTANT)
    Q_PROPERTY(WatchListModel* watchList READ watchListModel CONSTANT)
private:
    friend PlaylistModel;
    friend SearchResultsModel;
    friend EpisodeListModel;
    friend WatchListModel;
    EpisodeListModel m_episodeListModel{this};
    PlaylistModel m_playlistModel{this};
    SearchResultsModel m_searchResultsModel{this};
    WatchListModel m_watchListModel{this};
    DownloadModel m_downloadModel{this};

public:
    inline PlaylistModel* playlistModel(){return &m_playlistModel;}
    inline EpisodeListModel* episodeListModel(){return &m_episodeListModel;}
    inline SearchResultsModel* searchResultsModel(){return &m_searchResultsModel;}
    inline WatchListModel* watchListModel(){return &m_watchListModel;}
public:
    Q_INVOKABLE void loadSourceFromList(int index){
        m_playlistModel.syncList ();
        auto currentShow = ShowManager::instance().getCurrentShow();
        if(currentShow.isInWatchList ()){
            m_playlistModel.setWatchListShowItem (m_watchListModel.getShowJsonInList (currentShow));
        }
        m_playlistModel.loadSource (index);
    }

signals:
    void currentSearchProviderChanged();
    //Singleton implementation
public:
    static Application& instance()
    {
        static Application s_instance;
        return s_instance;
    }
private:
    explicit Application(QObject *parent = nullptr): QObject(parent){
        connect(&m_watchListModel,&WatchListModel::detailsRequested,&m_searchResultsModel,&SearchResultsModel::getDetails);
        connect(&m_playlistModel,&PlaylistModel::updatedLastWatchedIndex,&m_watchListModel,&WatchListModel::save);
        connect(&ShowManager::instance (), &ShowManager::currentShowChanged,&m_watchListModel,&WatchListModel::checkCurrentShowInList);
        connect(&ShowManager::instance (), &ShowManager::currentShowChanged,[&](){
            m_episodeListModel.setIsReversed(false);
            m_episodeListModel.updateLastWatchedName();
        });
        connect(&ShowManager::instance (), &ShowManager::lastWatchedIndexChanged,&m_episodeListModel,&EpisodeListModel::updateLastWatchedName);


        //        m_searchResultsModel.latest (1,0);
        //        m_downloadModel.downloadM3u8 ("lmao","D:\\TV\\temp\\尖峰时刻","https://ngzx.vizcloud.co/simple/EqPFIf8QWADtjDlGha7rC5QuqFxVu_T7SkR7rqk+wYMnU94US2El_Po4w1ynT_yP+tyVRt8p/br/H2/v.m3u8","https://ngzx.vizcloud.co");
    };
    ~Application() {} // Private destructor to prevent external deletion.
    Application(const Application&) = delete; // Disable copy constructor.
    Application& operator=(const Application&) = delete; // Disable copy assignment.

private:
};

#endif // APPLICATION_H
