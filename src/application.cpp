
#include "application.h"
#include "Components/errorhandler.h"
#include "Providers/iyf.h"
#include "Providers/testprovider.h"
#include "Providers/kimcartoon.h"
#include "Providers/gogoanime.h"
#include "Providers/nivod.h"
#include "Providers/haitu.h"
#include "Providers/allanime.h"
#include <QtConcurrent>

Application::Application(const QString &launchPath) : m_playlist(launchPath, this)
{
    NetworkClient::init ();

    // QDir pluginsDir(qApp->applicationDirPath());
    // pluginsDir.cd("plugins");
    // foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
    //     QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
    //     QObject *plugin = loader.instance();
    //     if (plugin) {
    //         ShowProvider *provider = qobject_cast<ShowProvider *>(plugin);
    //         if (provider) {
    //             m_providers.push_back (provider);
    //         }
    //     } else {
    //         qDebug() << "Could not load plugin:" << loader.errorString();
    //     }
    // }


    m_providers =
        {
            new IyfProvider,
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

    QString N_m3u8DLPath = QDir::cleanPath(QCoreApplication::applicationDirPath() + QDir::separator() + "N_m3u8DL-CLI_v3.0.2.exe");
    if (QFileInfo::exists (N_m3u8DLPath)) {
        m_downloader = new DownloadModel;
    }
    connect(&m_playlist, &PlaylistModel::currentIndexChanged, this, &Application::updateLastWatchedIndex);

}

Application::~Application() {
    NetworkClient::cleanUp ();
    qDeleteAll (m_providers);
    if (m_downloader)
        delete m_downloader;
}

void Application::search(const QString &query, int page) {
    int type = m_availableTypes[m_currentShowTypeIndex];
    m_searchResultsModel.search (query, page, type, m_currentSearchProvider);
}

void Application::latest(int page) {
    int type = m_availableTypes[m_currentShowTypeIndex];
    m_searchResultsModel.latest (page, type, m_currentSearchProvider);
}

void Application::popular(int page) {
    int type = m_availableTypes[m_currentShowTypeIndex];
    m_searchResultsModel.popular (page,type,m_currentSearchProvider);
}

void Application::loadShow(int index, bool fromWatchList) {

    if (fromWatchList) {
        auto showJson = m_libraryModel.getShowJsonAt(index);
        if (showJson.isEmpty ()) return;
        QString providerName = showJson["provider"].toString ();
        if (!m_providersMap.contains(providerName)) {
            ErrorHandler::instance().showWarning (providerName + " does not exist", "Show Error");
            return;
        }
        auto provider = m_providersMap[providerName];

        QString title = showJson["title"].toString ();
        QString link = showJson["link"].toString ();
        QString coverUrl = showJson["cover"].toString ();
        int lastWatchedIndex = showJson["lastWatchedIndex"].toInt ();
        int type = showJson["type"].toInt ();
        auto show = ShowData(title, link, coverUrl, provider, "", type);
        auto timeStamp = showJson["timeStamp"].toInt (0);
        int listType = m_libraryModel.getCurrentListType();
        m_showManager.setShow(show, {listType, lastWatchedIndex, timeStamp});
    } else {
        auto show = m_searchResultsModel.at(index);
        auto lastWatchedInfo = m_libraryModel.getLastWatchInfo (show.link);
        m_showManager.setShow(show, lastWatchedInfo);
    }
}

void Application::addCurrentShowToLibrary(int listType)
{
    m_libraryModel.add (m_showManager.getShow (), listType); //either changes the list type or adds to library
    m_showManager.setListType(listType);
}

void Application::removeCurrentShowFromLibrary() {
    m_libraryModel.remove (m_showManager.getShow ());
    m_showManager.setListType(-1);
}

void Application::playFromEpisodeList(int index) {
    updateTimeStamp();
    auto showPlaylist = m_showManager.getPlaylist ();
    index = m_showManager.correctIndex(index);


    m_playlist.replaceCurrentPlaylist (showPlaylist);
    m_playlist.tryPlay(-1, index);
}

void Application::continueWatching() {
    int index = m_showManager.getContinueIndex();
    if (index < 0) index = 0;
    playFromEpisodeList(index);
}

void Application::downloadCurrentShow(int startIndex, int count) {
    startIndex = m_showManager.correctIndex(startIndex);
    m_downloader->downloadShow (m_showManager.getShow (), startIndex, count); //TODO
}

void Application::updateTimeStamp() {
    // Update the last play time
    auto lastPlaylist = m_playlist.getCurrentPlaylist();
    if (!lastPlaylist) return;
    auto time = MpvObject::instance ()->time ();
    qDebug() << "Log (App): Attempting to updating time stamp for" << lastPlaylist->link << "to" << time;

    if (lastPlaylist->isLoadedFromFolder ()){
        lastPlaylist->updateHistoryFile (time);
    } else {
        if (time > 0.85 * MpvObject::instance ()->duration () && lastPlaylist->currentIndex + 1 < lastPlaylist->size ()) {
            qDebug() << "Log (App): Setting to next episode" << lastPlaylist->link;
            m_libraryModel.updateLastWatchedIndex (lastPlaylist->link, lastPlaylist->currentIndex + 1);
        }
        else {
            m_libraryModel.updateTimeStamp (lastPlaylist->link, time);
        }
    }
}

void Application::updateLastWatchedIndex() {
    PlaylistItem *currentPlaylist = m_playlist.getCurrentPlaylist();
    auto showPlaylist = m_showManager.getPlaylist ();
    if (!showPlaylist || !currentPlaylist) return;

    if (showPlaylist->link == currentPlaylist->link)
        m_showManager.updateLastWatchedIndex ();

    m_libraryModel.updateLastWatchedIndex (currentPlaylist->link, currentPlaylist->currentIndex);
}



void Application::setCurrentProviderIndex(int index) {
    if (index == m_currentProviderIndex) return;
    int currentType = m_availableTypes.isEmpty () ? -1 : m_availableTypes[m_currentShowTypeIndex];
    m_currentProviderIndex = index;
    m_currentSearchProvider = m_providers.at (index);
    m_availableTypes = m_currentSearchProvider->getAvailableTypes ();
    emit currentProviderIndexChanged();
    int searchTypeIndex =  m_availableTypes.indexOf (currentType);
    m_currentShowTypeIndex = searchTypeIndex == -1 ? 0 : searchTypeIndex;
    emit currentShowTypeIndexChanged();
}

void Application::setCurrentShowTypeIndex(int index) {
    if (index == m_currentShowTypeIndex) return;
    m_currentShowTypeIndex = index;
    emit currentShowTypeIndexChanged ();
}

void Application::cycleProviders() {
    setCurrentProviderIndex((m_currentProviderIndex + 1) % m_providers.count ());
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
