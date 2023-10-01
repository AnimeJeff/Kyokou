#include "showmanager.h"



ShowManager::ShowManager()
{
    providers =
        {
            new Nivod,
            new Gogoanime,
            new Tangrenjie,
            new Haitu,
#ifdef QT_DEBUG
            new TestProvider
#endif
        };

    for (ShowProvider* provider : providers)
    {
        providersHashMap.insert(provider->name (), provider);

    }

#ifdef QT_DEBUG
    m_currentSearchProvider = providersHashMap["TestProvider"];
#else
    m_currentSearchProvider = providersHashMap["泥巴影院"];
#endif
    providers.move (providers.indexOf (m_currentSearchProvider),providers.size ()-1);
    m_timeoutTimer.setSingleShot (true);
    m_timeoutTimer.setInterval (5000);
    connect(&m_timeoutTimer, &QTimer::timeout, this,[this](){
        m_cancelReason = "Loading took too long";
        m_watcher.future ().cancel ();
    });
}

ShowManager::~ShowManager() {
    if (currentShow.playlist)
    {
        delete currentShow.playlist;
    }
    qDeleteAll (providers);
}

void ShowManager::setCurrentShow(const ShowData &show)
{
    if (m_watcher.isRunning ()) return;
    if (currentShow.link == show.link)
    {
        emit currentShowChanged();
        return;
    }
    if(currentShow.playlist && !currentShow.playlist->isInPlaylist)
    {
        delete currentShow.playlist;
    }
    setLoading(true);
    currentShow = show;
    m_watcher.setFuture(
        QtConcurrent::run (
            [show,this]()
            {
                if(show.provider.isEmpty ())
                    return;
                if (ShowProvider *provider = getProvider(show.provider))
                {
                    qDebug()<<"Loading details for" << show.title << "with" << provider->name () << "using the link:" << show.link;
                    provider->loadDetails (currentShow);
                    qDebug()<<"Successfully loaded details for" << show.title;
                    return;
                }
                ErrorHandler::instance ().show (QString("Error: Unable to find a provider instance for %1").arg(show.provider));
            }));
    m_watcher.future ()
        .then ([this](){
            m_timeoutTimer.stop ();
            setLoading(false);
            emit currentShowChanged();})
        .onFailed ([](const std::exception &e){
            qDebug() << e.what();
            ErrorHandler::instance().show(e.what());
        })
        .onCanceled ([this](){
            qDebug() << "Operation cancelled:" << m_cancelReason;
        });
    //m_timeoutTimer.start ();
}

void ShowManager::changeSearchProvider(int index)
{
    m_currentSearchProvider = providers.at (index);
    providers.move (index, providers.size () - 1);
    emit searchProviderChanged();
    emit layoutChanged();
}

void ShowManager::setLastWatchedIndex(int index)
{
    if (!currentShow.playlist) return;
    currentShow.playlist->currentIndex = index;
    emit lastWatchedIndexChanged ();
}

int ShowManager::getLastWatchedIndex() const
{
    if (!currentShow.playlist) return -1;
    return currentShow.playlist->currentIndex;
}

void ShowManager::setListType(int listType)
{
    currentShow.listType = listType;
    emit listTypeChanged ();
}

int ShowManager::getCurrentShowListType() const
{
    return currentShow.listType;
}

QString ShowManager::currentSearchProviderName(){
    return m_currentSearchProvider->name ();
}

int ShowManager::rowCount(const QModelIndex &parent) const{
    return providers.count ();
}

QVariant ShowManager::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();
    auto provider = providers.at(index.row ());
    switch (role) {
    case NameRole:
        return provider->name ();
        break;
    case IconRole:
        break;
    default:
        break;
    }
    return QVariant();
}

QHash<int, QByteArray> ShowManager::roleNames() const{
    QHash<int, QByteArray> names;
    names[NameRole] = "text";
    //        names[IconRole] = "icon";
    //        names[EnumRole] = "providerEnum";
    return names;
}
