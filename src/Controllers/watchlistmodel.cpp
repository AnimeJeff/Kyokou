#include "watchlistmodel.h"
#include <QtGlobal>
#include "Data/playlistitem.h"
void WatchListModel::loadWatchList(QString filePath) {
    watchListFilePath = filePath.isEmpty() ? QDir::cleanPath(QCoreApplication::applicationDirPath() + QDir::separator() + ".watchlist") : filePath;
    QFile file(watchListFilePath);

    // Attempt to open the file, create and initialize with empty structure if it doesn't exist
    if (!file.open(QIODevice::ReadOnly)) {
        // File doesn't exist or cannot be opened for reading, create a new file with the empty structure
        if (file.open(QIODevice::WriteOnly)) {
            QJsonDocument doc(QJsonArray({QJsonArray(), QJsonArray(), QJsonArray(), QJsonArray(), QJsonArray()}));
            file.write(doc.toJson());
        }
        // Set m_watchListJson to the empty structure directly without reading from the file again
        m_watchListJson = QJsonArray({QJsonArray(), QJsonArray(), QJsonArray(), QJsonArray(), QJsonArray()});
    } else {
        // Read and parse the existing file
        QByteArray jsonData = file.readAll();
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
        if (error.error == QJsonParseError::NoError && doc.isArray()) {
            m_watchListJson = doc.array();
        } else {
            // In case of error, log it and use an empty array as a fallback
            qWarning() << "JSON parsing error:" << error.errorString();
            m_watchListJson = QJsonArray({QJsonArray(), QJsonArray(), QJsonArray(), QJsonArray(), QJsonArray()});
        }
    }
    file.close();

    // Populate the hash map from the loaded or initialized JSON array
    m_showHashmap.clear();
    for (int type = 0; type < m_watchListJson.size(); ++type) {
        const QJsonArray& array = m_watchListJson.at(type).toArray();
        for (int index = 0; index < array.size(); ++index) {
            const QJsonObject& show = array.at(index).toObject();
            QString link = show["link"].toString();
            m_showHashmap.insert(link, {type, index});
        }
    }
}

void WatchListModel::fetchUnwatchedEpisodes(int listType) {
    return;
    // int count = 0;
    // for (const auto& show : m_watchListJson[m_currentListType]) {
    //     QString providerName = QString::fromStdString (show["provider"].get<std::string>());
    //     // auto provider = ShowManagex).getProvider (providerName);
    //     // if (!provider) {
    //     //     qDebug()<<"Unable to find a provider for provider enum" << providerName;
    //     //     continue;
    //     // }
    //     // int totalEpisodes = provider->getTotalEpisodes(show["link"].get<std::string>());
    //     //totalEpisodeMap.insert ({show["link"].get<std::string>(), totalEpisodes)

    //     ++count;
    // }
    // emit layoutChanged();
}

QJsonObject WatchListModel::loadShow(int index) {
    // Validate the current list type and index
    const QJsonArray& currentList = m_watchListJson.at(m_currentListType).toArray();
    if (index < 0 || index >= currentList.size ()) {
        qWarning() << "Index out of bounds for the current list";
        return QJsonObject(); // Return an empty object for invalid index or list type
    }
    // Retrieve and return the show object at the given index
    const QJsonObject& show = currentList.at(index).toObject();
    return show;
}

void WatchListModel::updateProperty(const QString &showLink, QString propertyName, QVariant propertyValue, PropertyType propertyType){
    if (!m_showHashmap.contains(showLink)) return;

    QPair<int, int> listTypeAndIndex = m_showHashmap.value(showLink);
    int listType = listTypeAndIndex.first;
    int index = listTypeAndIndex.second;

    QJsonArray list = m_watchListJson[listType].toArray();
    QJsonObject show = list[index].toObject();

    if (propertyType == INT) {
        show.operator[](propertyName) = propertyValue.toInt ();
    } else {
        show.operator[](propertyName)= propertyValue.toString ();
    }


    list[index] = show; // Update the show in the list
    m_watchListJson[listType] = list; // Update the list in the model

    save(); // Save changes
}

void WatchListModel::add(ShowData& show, int listType)
{
    // Check if the show is already in the library, and if so, change its list type
    if (m_showHashmap.contains (show.link)) {
        changeShowListType (show, listType);
    } else {
        // Convert ShowData to QJsonObject
        QJsonObject showJson = show.toJson();

        // Append the new show to the appropriate list
        QJsonArray list = m_watchListJson.at(listType).toArray();
        list.append(showJson);
        m_watchListJson[listType] = list; // Update the list in m_jsonList

        // Update the hashmap
        m_showHashmap[show.link] = qMakePair(listType, list.count () - 1);

        // Model update signals
        if (m_currentListType == listType) {
            int newSize = list.size();
            beginInsertRows(QModelIndex(), newSize, newSize);
            endInsertRows();
        }
        show.setListType (listType);
    }
    save();
}

void WatchListModel::remove(ShowData &show)
{
    // QString showLink = QString::fromStdString(show.link);
    // Check if the show exists in the hashmap
    if (!m_showHashmap.contains(show.link)) return;

    // Extract list type and index from the hashmap
    QPair<int, int> listTypeAndIndex = m_showHashmap.value(show.link);
    int listType = listTypeAndIndex.first;
    int index = listTypeAndIndex.second;

    removeAt (index, listType);
    show.setListType (-1);
}

void WatchListModel::removeAt(int index, int listType) {
    if (listType < 0 || listType > 4) listType = m_currentListType;
    QJsonArray list = m_watchListJson[listType].toArray();
    if (index < 0 || index >= list.size ()) return;

    auto showLink = list[index].toObject ()["link"].toString ();

    // Remove the show from the list
    list.removeAt(index);
    m_watchListJson[listType] = list; // Update the list in the JSON structure

    // Remove the show from the hashmap
    m_showHashmap.remove(showLink);

    for (int i = index; i < list.size(); ++i) {
        QJsonObject show = list[i].toObject();
        QString showLink = show["link"].toString();
        m_showHashmap[showLink].second = i; // Update index
    }

    // If the current list type is being displayed, update the model accordingly
    if (m_currentListType == listType) {
        beginRemoveRows(QModelIndex(), index, index);
        endRemoveRows();
    }

    save(); // Save the changes to the JSON file
}

void WatchListModel::cycleDisplayingListType() {
    setDisplayingListType ((m_currentListType + 1) % 5);
}

void WatchListModel::move(int from, int to)
{
    QJsonArray currentList = m_watchListJson.at(m_currentListType).toArray();
    // Validate the 'from' and 'to' positions
    if (from < 0 || from >= currentList.size() || to < 0 || to >= currentList.size() || from == to) return;

    // Perform the move in the JSON array
    QJsonObject movingShow = currentList.takeAt (from).toObject();
    currentList.insert(to, movingShow);
    m_watchListJson[m_currentListType] = currentList; // Update the JSON structure

    for (int i = qMin(from, to); i <= qMax(from, to); ++i) {
        QJsonObject show = currentList.at(i).toObject();
        QString showLink = show["link"].toString();
        m_showHashmap[showLink].second = i; // Update index
    }

    beginMoveRows(QModelIndex(), from, from, QModelIndex(), to > from ? to + 1 : to);
    endMoveRows();
    save(); // Save changes
}

void WatchListModel::save()
{
    QMutexLocker locker(&mutex);

    QFile file(watchListFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open file for writing:" << watchListFilePath;
        return;
    }
    QJsonDocument doc(m_watchListJson); // Wrap the QJsonArray in a QJsonDocument
    file.write(doc.toJson(QJsonDocument::Indented)); // Write JSON data in a readable format
    file.close();
}

void WatchListModel::changeShowListType(ShowData &show, int newListType)
{
    // Check if the library has the show
    if (!m_showHashmap.contains (show.link)) return;

    // Retrieve the list type and the index of the show in the list
    QPair<int, int> listTypeAndIndex = m_showHashmap.value(show.link);
    int oldListType = listTypeAndIndex.first;
    int index = listTypeAndIndex.second;
    changeListTypeAt (index, newListType, oldListType);
    show.setListType (newListType);
}

void WatchListModel::changeListTypeAt(int index, int newListType, int oldListType) {
    if (oldListType == -1) oldListType = m_currentListType;
    if(oldListType == newListType) return;

    QJsonArray oldList = m_watchListJson[oldListType].toArray();
    QJsonArray newList = m_watchListJson[newListType].toArray();

    // Extract the show to move
    QJsonObject showToMove = oldList.takeAt(index).toObject();
    QString showLink = showToMove["link"].toString ();

    // Add to new list
    newList.append(showToMove);

    // Update the JSON structure
    m_watchListJson[oldListType] = oldList;
    m_watchListJson[newListType] = newList;

    // Update the hashmap
    m_showHashmap[showLink] = qMakePair (newListType, newList.count () - 1);

    for (int i = index; i < oldList.size(); ++i) {
        QJsonObject show = oldList.at(i).toObject();
        QString link = show["link"].toString();
        m_showHashmap[link].second = i; // Update index
    }

    // Emit model layout changes if necessary
    if (m_currentListType == oldListType) {
        beginRemoveRows (QModelIndex(), index, index);
        endRemoveRows();
    } else if (m_currentListType == newListType) {
        beginInsertRows(QModelIndex(), newList.size() - 1, newList.size());
        endInsertRows();
    }
    save(); // Save changes
}

QVariant WatchListModel::getChangeableListTypes() {
    QList<int> types = { WATCHING, PLANNED, ON_HOLD, DROPPED, COMPLETED };
    // QList<QString> types = { "Watching", "Planned", "On Hold", "Dropped", "Completed" };
    types.remove (m_currentListType);
    return QVariant::fromValue (types);
}

int WatchListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_watchListJson[m_currentListType].toArray ().size ();
}

QVariant WatchListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    try{
        auto show = m_watchListJson[m_currentListType].toArray ().at (index.row ());
        //    const auto show = m_list[m_currentListType][index.row ()];
        switch (role){
        case TitleRole:
            return show["title"].toString ();
            break;
        case CoverRole:
            return show["cover"].toString ();
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
