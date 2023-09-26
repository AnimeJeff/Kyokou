#include "application.h"

Application::Application(QObject *parent): QObject(parent)
{
    NetworkClient::init();
    connect(&m_watchListModel,&WatchListModel::detailsRequested,&m_searchResultsModel,&SearchResultsModel::getDetails);
    connect(&m_playlistModel,&PlaylistModel::updatedLastWatchedIndex,&m_watchListModel,&WatchListModel::save);
    connect(&ShowManager::instance (), &ShowManager::currentShowChanged,&m_watchListModel,&WatchListModel::checkCurrentShowInList);
    connect(&ShowManager::instance (), &ShowManager::currentShowChanged,[&](){
        m_episodeListModel.setIsReversed(false);
        m_episodeListModel.updateLastWatchedName();
    });
    connect(&ShowManager::instance (), &ShowManager::lastWatchedIndexChanged,&m_episodeListModel,&EpisodeListModel::updateLastWatchedName);

    QString N_m3u8DLPath = QDir::cleanPath (QCoreApplication::applicationDirPath() + QDir::separator() + "N_m3u8DL-CLI_v3.0.2.exe");;
    QString downloadDirectory = QDir::cleanPath (QCoreApplication::applicationDirPath() + QDir::separator() + "Downloads");;

    DownloadRunnable::setN_m3u8DLPath (N_m3u8DLPath);
    DownloadModel::setDownloadFolderPath (downloadDirectory);

    //        m_searchResultsModel.latest (1,4);
    //        m_downloadModel.downloadM3u8 ("lmao","D:\\TV\\temp\\尖峰时刻","https://ngzx.vizcloud.co/simple/EqPFIf8QWADtjDlGha7rC5QuqFxVu_T7SkR7rqk+wYMnU94US2El_Po4w1ynT_yP+tyVRt8p/br/H2/v.m3u8","https://ngzx.vizcloud.co");
}

void Application::loadSourceFromList(int index)
{
    try{
        auto currentShow = ShowManager::instance().getCurrentShow();
        auto watchListShowItem = m_watchListModel.getShowJsonInList (currentShow);
        currentShow.playlist->m_watchListShowItem = watchListShowItem;
        m_playlistModel.syncList (currentShow, watchListShowItem);
        m_playlistModel.play (0, index);
    }catch(QException& e){
        qDebug() << e.what ();
    }
}

int Application::parseArgs(int argc, char *argv[])
{
    if(argc == 1) return 0;
    if(strncmp("http", argv[1],4) == 0) // is an url
    {
        qDebug() << "Opening url" << argv[1];
        playlistModel ()->setLaunchFile (argv[1]);
        return 0;
    }
    std::filesystem::path path = std::filesystem::path(argv[1]).make_preferred ();
    if(path.is_relative ()) //handles relative path
        path = (std::filesystem::current_path()/std::filesystem::path(path));
    QString pathString = QString::fromStdString (path.string ());
    if (!std::filesystem::exists(path)){
        qDebug() << "Path" << pathString << "does not exist!";
        return -1;
    }
    qDebug() << "Opening file" << pathString;
    if(std::filesystem::is_directory (path)){
        playlistModel ()->setLaunchFolder (pathString);
    }else{
        std::set<std::string> validExtensions{".mp4", ".mkv", ".avi", ".mp3", ".flac", ".wav", ".ogg", ".webm"};
        if (!validExtensions.count(path.extension().string())){
            qDebug() << "File" << pathString << "does not have a valid extension";
            return -1;
        }
        playlistModel ()->setLaunchFile (pathString);
    }
    return 0;
}


