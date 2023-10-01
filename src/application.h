#ifndef APPLICATION_H
#define APPLICATION_H

#include "Download/downloadmodel.h"
#include "Explorer/episodelistmodel.h"
#include "Player/playlistmodel.h"
#include "Player/mpv/mpvObject.h"
#include "Explorer/searchresultsmodel.h"
#include "Explorer/watchlistmodel.h"

#include "Explorer/showmanager.h"
#include "Components/cursor.h"
#include <QAbstractListModel>


class Application : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PlaylistModel* playlist READ playlistModel CONSTANT)
    Q_PROPERTY(EpisodeListModel* episodeListModel READ episodeListModel CONSTANT)
    Q_PROPERTY(SearchResultsModel* showExplorer READ searchResultsModel CONSTANT)
    Q_PROPERTY(WatchListModel* watchList READ watchListModel CONSTANT)
    Q_PROPERTY(DownloadModel* downloader READ downloadModel CONSTANT)
    Q_PROPERTY(Cursor* cursor READ cursor CONSTANT)
private:
    friend PlaylistModel;
    friend SearchResultsModel;
    friend EpisodeListModel;
    friend WatchListModel;
    Cursor m_cursor {}; //todo
    EpisodeListModel m_episodeListModel {this};
    PlaylistModel m_playlistModel {this};
    SearchResultsModel m_searchResultsModel {this};
    WatchListModel m_watchListModel {this};
    DownloadModel m_downloadModel {this};
public:
    PlaylistModel* playlistModel() { return &m_playlistModel; }
    EpisodeListModel* episodeListModel() { return &m_episodeListModel; }
    SearchResultsModel* searchResultsModel() { return &m_searchResultsModel; }
    WatchListModel* watchListModel() { return &m_watchListModel; }
    DownloadModel* downloadModel() { return &m_downloadModel; }
    Cursor* cursor() { return &m_cursor; }
public:
    bool parseArgs(int argc, char *argv[]);
public:
    static Application& instance()
    {
        static Application s_instance;
        return s_instance;
    }
private:
    explicit Application(QObject *parent = nullptr);
    ~Application() {
        NetworkClient::shutdown();
    }
    Application(const Application&) = delete; // Disable copy constructor.
    Application& operator=(const Application&) = delete; // Disable copy assignment.

private:
};

#endif // APPLICATION_H
