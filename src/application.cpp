
#include "application.h"
#include "Providers/testprovider.h"
#include "Providers/kimcartoon.h"
#include "Providers/gogoanime.h"
#include "Providers/nivod.h"
#include "Providers/haitu.h"
#include "Providers/allanime.h"
#include <QtConcurrent>

Application::Application(QString launchPath)
{
    NetworkClient::init ();

    m_providers =
        {
            new AllAnime,
            new Haitu,
            new Nivod,
            new Gogoanime,
            new Kimcartoon,
            // #ifdef QT_DEBUG
            //             new TestProvider
            // #endif
        };

    for (ShowProvider* provider : m_providers) {
        m_providersMap.insert(provider->name (), provider);
    }
    setCurrentProviderIndex(0);

    if (!launchPath.isEmpty ())
        m_playlist.setLaunchPath(launchPath);

    // Cancels the task when it takes too long
    m_timeoutTimer.setSingleShot (true);
    m_timeoutTimer.setInterval (5000);
    connect(&m_timeoutTimer, &QTimer::timeout, this,[this](){
        m_cancelReason = "Loading took too long";
        m_watcher.future ().cancel ();
    });


    connect (&m_showManager, &ShowManager::showChanged, this, [this](){
        setLoading (false);
        });



    bool N_m3u8DLPathExists = QFile(QDir::cleanPath (QCoreApplication::applicationDirPath() + QDir::separator() + "N_m3u8DL-CLI_v3.0.2.exe")).exists ();
    if (N_m3u8DLPathExists)
    {
        m_downloader = new DownloadModel;
    }
    connect(&m_playlist, &PlaylistModel::currentIndexChanged, this, &Application::updateLastWatchedIndex);
}

Application::~Application() {
    NetworkClient::shutdown ();
    qDeleteAll (m_providers);
    if (m_downloader)
        delete m_downloader;
}


void Application::cancel()
{
    if (m_watcher.isRunning ())
    {
        m_watcher.cancel ();
        setLoading (false);
    }
}

int Application::rowCount(const QModelIndex &parent) const{
    return m_providers.count ();
}

QVariant Application::data(const QModelIndex &index, int role) const{
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

QHash<int, QByteArray> Application::roleNames() const{
    QHash<int, QByteArray> names;
    names[NameRole] = "text";
    // names[IconRole] = "icon";
    return names;
}
