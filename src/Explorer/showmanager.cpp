#include "showmanager.h"

#include "Providers/testprovider.h"
#include "Providers/kimcartoon.h"
#include "Providers/gogoanime.h"
#include "Providers/nivod.h"
#include "Providers/haitu.h"
#include "Providers/allanime.h"
#include <QtConcurrent>

ShowManager::ShowManager(QString launchPath)
{
    m_providers =
        {
            new AllAnime,
            new Nivod,
            new Gogoanime,
            new Haitu,
            new Kimcartoon,
            // #ifdef QT_DEBUG
            //             new TestProvider
            // #endif
        };

    for (ShowProvider* provider : m_providers) {
        m_providersMap.insert(provider->name (), provider);
    }
    m_currentSearchProvider = m_providers.first ();

    // Parsing the arguments    if (argc == 1) return true;
    // m_playlistModel.setLaunchPath (QString::fromLocal8Bit (argv[1]))
    // return false;
    // ShowManager::instance ().setLaunchArgs(QString::fromLocal8Bit (argv[1]));
    if (!launchPath.isEmpty ())
        m_playlistModel.setLaunchPath(launchPath);
    // m_providers.move (m_providers.indexOf (m_currentSearchProvider),m_providers.size () - 1);



    // Cancels the task when it takes too long
    m_timeoutTimer.setSingleShot (true);
    m_timeoutTimer.setInterval (5000);
    connect(&m_timeoutTimer, &QTimer::timeout, this,[this](){
        m_cancelReason = "Loading took too long";
        m_watcher.future ().cancel ();
    });

    QObject::connect (&m_watcher, &QFutureWatcher<QList<ShowData>>::finished, this, [this](){

        if (!m_watcher.future().isValid()) {
            //future was cancelled
            ErrorHandler::instance().show ("Operation cancelled: " + m_cancelReason);
            setLoading (false);
            return;
        }

        try
        {
            if (currentShow.playlist != nullptr)
                ++currentShow.playlist->useCount;

            m_watchListModel.syncShow(currentShow);
            if (currentShow.lastWatchedIndex >= 0) {

                currentShow.playlist->currentIndex = currentShow.lastWatchedIndex;
                // If the last watched index is greater than the total number episodes
                // then set it to the index of the max episode
                if (currentShow.playlist->currentIndex >= currentShow.playlist->count ()){
                    currentShow.playlist->currentIndex = currentShow.playlist->count () - 1;
                    //TODO update in watch list?
                }
            }
            m_episodeListModel.setPlaylist(currentShow.playlist);
            m_episodeListModel.setIsReversed(true);

            emit listTypeChanged();
            emit currentShowChanged();
        }
        catch (QException& ex)
        {
            ErrorHandler::instance().show (ex.what ());
        }
        setLoading (false);
    });

    bool N_m3u8DLPathExists = QFile(QDir::cleanPath (QCoreApplication::applicationDirPath() + QDir::separator() + "N_m3u8DL-CLI_v3.0.2.exe")).exists ();
    if (N_m3u8DLPathExists)
    {
        m_downloadModel = new DownloadModel;
    }
    connect(&m_playlistModel, &PlaylistModel::currentIndexChanged, this, &ShowManager::checkUpdateLastWatchedIndex);
}

ShowManager::~ShowManager() {
    if (currentShow.playlist && --currentShow.playlist->useCount == 0) // might have already been deleted by playlistmodel
    {
        delete currentShow.playlist;
        currentShow.playlist = nullptr;
    }
    qDeleteAll (m_providers);
    if (m_downloadModel)
        delete m_downloadModel;
}

void ShowManager::setCurrentShow(const ShowData &show)
{
    if (m_watcher.isRunning ()) return;
    if (currentShow.link == show.link) {
        emit currentShowChanged();
        return;
    }
    // delete the previous show playlist that is being held in info tab
    if (currentShow.playlist && --currentShow.playlist->useCount == 0) // <0 means that it has already been deleted
    {
        delete currentShow.playlist;
    }

    setLoading(true);
    currentShow = show;
    m_watcher.setFuture(
        QtConcurrent::run (
            [this]()
            {
                if (currentShow.provider)
                {
                    qDebug()<<"Log (ShowManager)： Loading details for" << currentShow.title << "with" << currentShow.provider->name () << "using the link:" << currentShow.link;
                    currentShow.provider->loadDetails (currentShow);
                    qDebug()<<"Log (ShowManager)： Successfully loaded details for" << currentShow.title;
                } else {
                    ErrorHandler::instance ().show (QString("Error: Unable to find a provider for %1").arg(currentShow.title));
                }

            }));
}

void ShowManager::changeSearchProvider(int index)
{
    m_currentSearchProvider = m_providers.at (index);
    // m_providers.move (index, m_providers.size () - 1);
    emit searchProviderChanged();
    emit layoutChanged();
}

int ShowManager::getCurrentShowListType() const
{
    return currentShow.listType;
}

void ShowManager::cancel()
{
    if (m_watcher.isRunning ())
    {
        m_watcher.cancel ();
        setLoading (false);
    }
}

int ShowManager::rowCount(const QModelIndex &parent) const{
    return m_providers.count ();
}

QVariant ShowManager::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();
    auto provider = m_providers.at(index.row ());
    switch (role){
    case NameRole:
        return provider->name ();
        break;
    // case IconRole:
    // break;
    default:
        break;
    }
    return QVariant();
}

QHash<int, QByteArray> ShowManager::roleNames() const{
    QHash<int, QByteArray> names;
    names[NameRole] = "text";
    // names[IconRole] = "icon";
    return names;
}
