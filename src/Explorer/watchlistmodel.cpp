#include "watchlistmodel.h"
#include "showmanager.h"
#include <tuple>
int WatchListModel::getCurrentListType()
{
    return m_currentListType;
}

void WatchListModel::setDisplayingListType(int listType)
{
    m_currentListType = listType;
    emit layoutChanged ();
}

void WatchListModel::loadWatchList(QString filePath)
{
    watchListFilePath = filePath.isEmpty () ? QDir::cleanPath (QCoreApplication::applicationDirPath() + QDir::separator() + ".watchlist") : filePath;
    QFile infile(watchListFilePath);
    const char* emptyFile = "[[],[],[],[],[]]";
    if (!infile.exists() || !infile.open(QIODevice::ReadWrite))
    {
        // File doesn't exist or is corrupted
        QFile outfile(watchListFilePath);
        if (outfile.open(QIODevice::WriteOnly))
        {
            outfile.write (emptyFile);
            outfile.close();
        }
        m_jsonList = nlohmann::json::parse (emptyFile);
    }
    else
    {
        if (infile.peek(1)!="[")
        {
            infile.write (emptyFile);
            m_jsonList = nlohmann::json::parse (emptyFile);
        }
        else{
            auto list = infile.readAll ().toStdString ();
            m_jsonList = nlohmann::json::parse (list);
        }
        infile.close ();
    }
    for (auto& [type, array] : m_jsonList.items())
    {
        for (auto& [index, show] : array.items())
        {
            jsonHashmap.insert ({show["link"].get<std::string>(), std::stoi(type)});
        }
    }
}

void WatchListModel::changeListType(const ShowData &show, int newListType)
{
    // check if the show is in the list
    if (!jsonHashmap.contains (show.link)) return;

    int oldListType = jsonHashmap[show.link];
    if(oldListType == newListType) return;

    // find the index of the show in the list
    int index = findShowInJsonList (oldListType, show.link);
    if(index == -1) return; // failed to find the index
    // add to the new list
    m_jsonList[newListType].push_back(m_jsonList[oldListType][index]);
    // update the hashmap
    jsonHashmap[show.link] = newListType;
    // remove the show from the old list
    m_jsonList[oldListType].erase (m_jsonList[oldListType].begin() + index);

    if (m_currentListType == oldListType) {
        beginRemoveRows (QModelIndex(), m_jsonList[oldListType].size(), m_jsonList[oldListType].size() - 1);
        endRemoveRows();
    } else if (m_currentListType == newListType) {
        beginInsertRows(QModelIndex(), m_jsonList[newListType].size(), m_jsonList[newListType].size() + 1);
        endInsertRows();
    }
    save();
}

void WatchListModel::add(const ShowData& show, int listType)
{
    m_jsonList[listType].push_back (show.toJson ());
    jsonHashmap[show.link] = listType;
    if (m_currentListType == listType) {
        beginInsertRows(QModelIndex(), m_jsonList[listType].size(), m_jsonList[listType].size() + 1);
        endInsertRows();
    }
    save();
}

void WatchListModel::addCurrentShow(int listType)
{
    const auto& currentShow = ShowManager::instance ().getCurrentShow ();
    if (currentShow.isInWatchList ())
    {
        changeListType (currentShow, listType);
    }
    else
    {
        add(currentShow, listType);
    }
    ShowManager::instance().setListType (listType);
}

void WatchListModel::removeCurrentShow()
{
    remove (ShowManager::instance().getCurrentShow ());
    ShowManager::instance ().setListType(-1);
}

void WatchListModel::remove(const ShowData &show)
{
    // check if the show is in the list
    if (!jsonHashmap.contains (show.link)) return;
    int listType = jsonHashmap[show.link];

    // find the index of the show in the list
    int index = findShowInJsonList (listType, show.link);
    if(index == -1) return; // failed to find the index
    // remove the show from the list
    m_jsonList[listType].erase (m_jsonList[listType].begin() + index);
    // remove the show from the hashmap
    jsonHashmap.erase(show.link);
    save();
}

void WatchListModel::move(int from, int to)
{
    auto& list = m_jsonList[m_currentListType];
    auto element_to_move = list[from];
    list.erase(list.begin() + from);
    list.insert(list.begin() + to, element_to_move);
}

void WatchListModel::moveEnded()
{
    emit layoutChanged ();
    save();
}

void WatchListModel::syncCurrentShow()
{
    if (std::string link = ShowManager::instance ().getCurrentShow ().link; !ShowManager::instance ().isInWatchList () && jsonHashmap.contains (link))
    {
        int listType = jsonHashmap[link];
        // find the index of the show in the list
        int index = findShowInJsonList (listType, link);
        if(index != -1) {
            int lastWatchedIndex = m_jsonList[listType][index]["lastWatchedIndex"].get<int> ();
            ShowManager::instance().setLastWatchedIndex(lastWatchedIndex);
            ShowManager::instance().setListType(listType);
            ShowManager::instance().getCurrentShow().playlist->m_watchListShowItem = &m_jsonList[listType][index];
        }
    }
    emit syncedCurrentShow();
}

void WatchListModel::save()
{
    QMutexLocker locker(&mutex);
    std::ofstream output_file(watchListFilePath.toStdString ());
    output_file << m_jsonList.dump (4);
    output_file.close();
}

void WatchListModel::fetchUnwatchedEpisodes(int listType) {
    return;
    int count = 0;
    for (const auto& show : m_jsonList[m_currentListType]) {
        QString providerName = QString::fromStdString (show["provider"].get<std::string>());
        auto provider = ShowManager::instance ().getProvider (providerName);
        if (!provider) {
            qDebug()<<"Unable to find a provider for provider enum" << providerName;
            continue;
        }
        int totalEpisodes = provider->getTotalEpisodes(show["link"].get<std::string>());
        //totalEpisodeMap.insert ({show["link"].get<std::string>(), totalEpisodes)

        ++count;
    }
    emit layoutChanged();
}

void WatchListModel::loadShow(int index)
{
    auto showJson = m_jsonList[m_currentListType][index];
    std::string link = showJson["link"].get<std::string>();
    QString title = QString::fromStdString(showJson["title"].get<std::string>());
    QString coverUrl = QString::fromStdString(showJson["cover"].get<std::string>());
    QString provider = QString::fromStdString (showJson["provider"].get<std::string>());
    int lastWatchedIndex = showJson["lastWatchedIndex"].get<int>();
    ShowData show(title, link, coverUrl, provider, "", m_currentListType);
    ShowManager::instance ().setCurrentShow (show);
    ShowManager::instance ().setLastWatchedIndex (lastWatchedIndex);
}

int WatchListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_jsonList[m_currentListType].size ();
}

QVariant WatchListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    try{
        auto show = m_jsonList[m_currentListType][index.row ()];
        //    const auto show = m_list[m_currentListType][index.row ()];
        switch (role){
        case TitleRole:
            return QString::fromStdString (show["title"].get<std::string> ());
            break;
        case CoverRole:
            return QString::fromStdString (show["cover"].get<std::string> ());
            break;
        case UnwatchedEpisodesRole:
            //                if (show->totalEpisodes > show->lastWatchedIndex)
            //                {
            //                    if (show->lastWatchedIndex < 0){
            //                        return show->totalEpisodes;
            //                    }
            //                    return show->totalEpisodes - show->lastWatchedIndex - 1;
            //                }
            break;
        default:
            break;
        }

    }catch(...)
    {
        return {};
    }


    return QVariant();
}

QHash<int, QByteArray> WatchListModel::roleNames() const{
    QHash<int, QByteArray> names;
    names[TitleRole] = "title";
    names[CoverRole] = "cover";
    names[UnwatchedEpisodesRole] = "unwatchedEpisodes";
    return names;
}
