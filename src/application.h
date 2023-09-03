#ifndef APPLICATION_H
#define APPLICATION_H

#include "models/downloadmodel.h"
#include "models/episodelistmodel.h"
#include "models/playlistmodel.h"
#include "models/searchresultsmodel.h"
#include "models/watchlistmodel.h"
#include "mpv/mpvObject.h"
#include "showmanager.h"
#include <QAbstractListModel>


class Application : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PlaylistModel* playlist READ playlistModel CONSTANT)
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
    int parseArgs(int argc, char *argv[]);
public:
    static Application& instance()
    {
        static Application s_instance;
        return s_instance;
    }
private:
    explicit Application(QObject *parent = nullptr);;
    ~Application() {
        NetworkClient::shutdown();
    }
    Application(const Application&) = delete; // Disable copy constructor.
    Application& operator=(const Application&) = delete; // Disable copy assignment.

private:
};

#endif // APPLICATION_H
