#include "application.h"

Application::Application(QObject *parent): QObject(parent)
{
    NetworkClient::init();

    connect(&m_playlistModel,&PlaylistModel::updatedLastWatchedIndex,&m_watchListModel,&WatchListModel::save);
    connect(&ShowManager::instance (), &ShowManager::currentShowChanged,&m_watchListModel,&WatchListModel::syncCurrentShow);
    connect(&ShowManager::instance (), &ShowManager::currentShowChanged,[this]()
            {
                m_episodeListModel.setIsReversed(true);
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

bool Application::parseArgs(int argc, char *argv[])
{
    if(argc == 1) return true;
    return m_playlistModel.setLaunchPath (argv[1]);
}


