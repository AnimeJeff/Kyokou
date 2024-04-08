#include "providermanager.h"
#include "Providers/iyf.h"
// #include "Providers/testprovider.h"
#include "Providers/kimcartoon.h"
#include "Providers/gogoanime.h"
#include "Providers/nivod.h"
#include "Providers/haitu.h"
#include "Providers/allanime.h"

ProviderManager::ProviderManager(QObject *parent)
    : QAbstractListModel(parent)
{
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
            new Nivod,
            new AllAnime,
            new Haitu,
            new Kimcartoon,
            new Gogoanime,
            new IyfProvider,


            // #ifdef QT_DEBUG
            //             new TestProvider
            // #endif
        };

    for (ShowProvider* provider : m_providers) {
        m_providersMap.insert(provider->name (), provider);
    }

    setCurrentProviderIndex(0);


}

void ProviderManager::setCurrentProviderIndex(int index) {
    if (index == m_currentProviderIndex) return;
    m_currentSearchType = m_availableTypes.isEmpty () ? -1 : m_availableTypes[m_currentShowTypeIndex];
    m_currentProviderIndex = index;
    m_currentSearchProvider = m_providers.at (index);
    m_availableTypes = m_currentSearchProvider->getAvailableTypes ();
    emit currentProviderIndexChanged();
    int searchTypeIndex =  m_availableTypes.indexOf (m_currentSearchType);
    m_currentShowTypeIndex = searchTypeIndex == -1 ? 0 : searchTypeIndex;
    m_currentSearchType = m_availableTypes[m_currentShowTypeIndex];
    emit currentSearchTypeIndexChanged();
}

void ProviderManager::setCurrentSearchTypeIndex(int index) {
    if (index == m_currentShowTypeIndex) return;
    m_currentShowTypeIndex = index;
    emit currentSearchTypeIndexChanged ();
}

void ProviderManager::cycleProviders() {
    setCurrentProviderIndex((m_currentProviderIndex + 1) % m_providers.count ());
}

int ProviderManager::rowCount(const QModelIndex &parent) const{
    return m_providers.count ();
}

QVariant ProviderManager::data(const QModelIndex &index, int role) const{
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

QHash<int, QByteArray> ProviderManager::roleNames() const{
    QHash<int, QByteArray> names;
    names[NameRole] = "text";
    // names[IconRole] = "icon";
    return names;
}
