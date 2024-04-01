
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
    setLoading (true);
    if (fromWatchList) {
        auto showJson = m_watchListModel.loadShow(index);
        if (showJson.isEmpty ()) return;
        QString providerName = showJson["provider"].toString ();
        if (!m_providersMap.contains(providerName)) return;
        auto provider = m_providersMap[providerName];
        std::string link = showJson["link"].toString ().toStdString ();
        QString title = showJson["title"].toString ();
        QString coverUrl = showJson["cover"].toString ();
        int lastWatchedIndex = showJson["lastWatchedIndex"].toInt ();
        int type = showJson["type"].toInt ();
        auto show = ShowData(title, link, coverUrl, provider, "", type, lastWatchedIndex);
        m_showManager.setShow(show);
        m_showManager.setListType (m_watchListModel.getCurrentListType());
    } else {
        auto show = m_searchResultsModel.at(index);
        m_watchListModel.syncShow (show);
        m_showManager.setShow(show);
    }
}

void Application::addCurrentShowToLibrary(int listType)
{
    m_watchListModel.add (m_showManager.getShow (), listType); //either changes the list type or adds to library
    m_showManager.setListType(listType);
}

void Application::removeCurrentShowFromLibrary()
{
    m_watchListModel.remove (m_showManager.getShow ());
    m_showManager.setListType(-1);
}

void Application::playFromEpisodeList(int index)
{
    m_playlist.replaceCurrentPlaylist (m_showManager.getPlaylist ());
    m_playlist.play (0, m_showManager.correctIndex(index));
}

void Application::continueWatching()
{
    int index = m_showManager.getContinueIndex();
    if (index < 0) {
        qDebug() << "Error: Invalid continue index";
        return;
    }
    playFromEpisodeList(index);
}

void Application::downloadCurrentShow(int startIndex, int count)
{
    m_downloader->downloadShow (m_showManager.getShow (), m_showManager.correctIndex(startIndex), count); //TODO
}

void Application::updateLastWatchedIndex() {
    PlaylistItem *currentPlaylist = m_playlist.getCurrentPlaylist();
    auto showPlaylist = m_showManager.getPlaylist ();
    if (!showPlaylist || !currentPlaylist) {//TODO local file
        qDebug() << "error: playlist returned nullptr";
        return;
    }

    if (showPlaylist->link == currentPlaylist->link) {
        m_showManager.updateLastWatchedIndex (currentPlaylist->currentIndex);
    }

    //todo change playlist link to qstring
    m_watchListModel.updateLastWatchedIndex (QString::fromStdString (currentPlaylist->link), currentPlaylist->currentIndex);

}

void Application::cancel()
{
    if (m_watcher.isRunning ())
    {
        m_watcher.cancel ();
        setLoading (false);
    }
}

void Application::setCurrentProviderIndex(int index) {
    if (index == m_currentProviderIndex) return;
    int currentType = m_currentProviderIndex != -1 ? m_availableTypes[m_currentShowTypeIndex] : -1;
    m_currentProviderIndex = index;
    m_currentSearchProvider = m_providers.at (index);
    m_availableTypes = m_currentSearchProvider->getAvailableTypes ();
    emit currentProviderIndexChanged();
    int searchTypeIndex = searchTypeIndex == -1 ? -1 : m_availableTypes.indexOf (currentType);
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
