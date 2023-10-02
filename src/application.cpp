#include "application.h"
#include <QString>
#include <QTextCodec>
Application::Application(QObject *parent): QObject(parent)
{
    NetworkClient::init();

    connect(&m_playlistModel,&PlaylistModel::updatedLastWatchedIndex,&m_watchListModel,&WatchListModel::save);
    connect(&ShowManager::instance (), &ShowManager::currentShowChanged, &m_watchListModel, &WatchListModel::syncCurrentShow);

    connect(&m_watchListModel, &WatchListModel::syncedCurrentShow, this, [this]()
            {
                m_episodeListModel.setIsReversed(true);
                m_episodeListModel.updateLastWatchedName();
            });
    connect(&ShowManager::instance (), &ShowManager::lastWatchedIndexChanged, &m_episodeListModel, &EpisodeListModel::updateLastWatchedName);
    N_m3u8DLPathExists = QFile(QDir::cleanPath (QCoreApplication::applicationDirPath() + QDir::separator() + "N_m3u8DL-CLI_v3.0.2.exe")).exists ();
//    ;;
//    if(QFile(N_m3u8DLPath).exists ())
//    {
//        DownloadRunnable::setN_m3u8DLPath (N_m3u8DLPath);
//        DownloadModel::setDownloadFolderPath (downloadDirectory);
//    }
}

bool Application::parseArgs(int argc, char *argv[])
{
    if(argc == 1) return true;
    return m_playlistModel.setLaunchPath (QString::fromLocal8Bit (argv[1]));
}


