#include "watchlistmodel.h"
#include "showmanager.h"

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
        if(infile.peek(1)!="[")
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
            std::tuple<nlohmann::json*, int, int> d = std::tuple<nlohmann::json*, int, int>(&show, std::stoi(type), std::stoi(index));
            jsonHashmap.insert ({show["link"].get<std::string>(), d});
        }
    }
}

void WatchListModel::changeListType(const ShowData &show, int newListType)
{
    const auto [json,type,index] = jsonHashmap.at (show.link);
    auto& oldList = m_jsonList[show.listType];
    auto& newList = m_jsonList[newListType];
    newList.push_back(oldList[index]);
    oldList.erase (oldList.begin() + index);

    std::get<1>(jsonHashmap[show.link]) = newListType;
    std::get<2>(jsonHashmap[show.link]) = newList.size () - 1;
    emit layoutChanged();
    save();
}

void WatchListModel::add(const ShowData& show, int listType)
{
    m_jsonList[listType].push_back (show.toJson ());
    jsonHashmap[show.link] = std::tuple<nlohmann::json*, int, int>(&m_jsonList[listType].back (),listType, m_jsonList[listType].size () - 1);
    if (m_currentListType == listType)
        emit layoutChanged ();
    save();
}

void WatchListModel::addCurrentShow(int listType)
{
    const auto& currentShow = ShowManager::instance ().getCurrentShow ();
    if(currentShow.isInWatchList ())
    {
        //change the list type
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
    const auto& [json,type,index] = jsonHashmap[show.link];
    Q_ASSERT(show.listType == type);
    auto& list = m_jsonList[type];
    list.erase (list.begin() + index);
    emit layoutChanged();
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
    if(ShowManager::instance ().isInWatchList ())
    {
        emit syncedCurrentShow();
    }
    std::string link = ShowManager::instance ().getCurrentShow ().link;
    if(jsonHashmap.contains (link))
    {
        const auto& [json,type,index] = jsonHashmap[link];
        ShowManager::instance ().setLastWatchedIndex(json->operator[]("lastWatchedIndex").get<int> ());
        ShowManager::instance().setListType(type);
        ShowManager::instance().getCurrentShow().playlist->m_watchListShowItem = json;
        emit syncedCurrentShow();
    }
    emit syncedCurrentShow();
}

void WatchListModel::fetchUnwatchedEpisodes(){
    return;
    int count = 0;
//    for(const auto& show:m_list[WATCHING]){
//        auto provider = ShowManager::instance ().getProvider (show->provider);
//        if(!provider) {
//            qDebug()<<"Unable to find a provider for provider enum" << show->provider;
//            continue;
//        }
//        //            qDebug()<<"Getting the total episodes for" << show->title << "with" << provider->name ();
//        auto totalEpisodes = provider->getTotalEpisodes(*show);
//        show->totalEpisodes = totalEpisodes;
//        //            qDebug()<<totalEpisodes;
//        //            emit dataChanged(index(count,0),index(count,0));
//        count++;
//    }
    emit layoutChanged();
}


void WatchListModel::loadShow(int index)
{
    auto showJson = m_jsonList[m_currentListType][index];
    std::string link = showJson["link"].get<std::string>();
    qDebug() << link;
    QString title = QString::fromStdString(showJson["title"].get<std::string>());
     qDebug() << title;
    QString coverUrl = QString::fromStdString(showJson["cover"].get<std::string>());
      qDebug() << coverUrl;
    QString provider = QString::fromStdString (showJson["provider"].get<std::string>());
       qDebug() << provider;
    int lastWatchedIndex = showJson["lastWatchedIndex"].get<int>();
       qDebug() << lastWatchedIndex;
    ShowData show(title, link, coverUrl, provider);
    ShowManager::instance ().setCurrentShow (show);
    ShowManager::instance ().setLastWatchedIndex (lastWatchedIndex);
}


int WatchListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_jsonList[m_currentListType].size ();
    //    return m_jsonList[m_currentListType].count ();
}

QVariant WatchListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    try{
        auto show = m_jsonList[m_currentListType][index.row ()];
        //    const auto show = m_list[m_currentListType][index.row ()];
        switch (role) {
        case TitleRole:
            return QString::fromStdString (show["title"].get<std::string> ());
            break;
        case CoverRole:
            return QString::fromStdString (show["cover"].get<std::string> ());
            break;
        case UnwatchedEpisodesRole:
            //        if(show->totalEpisodes > show->lastWatchedIndex)
            //        {
            //            if(show->lastWatchedIndex<0){
            //                return show->totalEpisodes;
            //            }
            //            return show->totalEpisodes - show->lastWatchedIndex - 1;
            //        }
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
