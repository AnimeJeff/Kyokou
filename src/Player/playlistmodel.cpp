#include "playlistmodel.h"


void PlaylistModel::setLaunchFolder(const QString& path){
    loadFolder (QUrl::fromLocalFile(path));
    //todo move to later in qml
    if(rootItem->first ()->count () == 0)
    {
        qWarning() << "Directory has no playable items.";
        return;
    }
    m_onLaunchFile = QString::fromStdString (rootItem->first()->at (rootItem->first()->currentIndex)->link);
}

QString PlaylistModel::loadOnlineSource(int playlistIndex, int itemIndex){
    //lazy show in watchlist
    auto playlist = rootItem->at (playlistIndex);
    if(playlist->sourceLink.ends_with (".m3u8"))
    {
        return QString::fromStdString (playlist->sourceLink);
    }

    auto episode = playlist->at (itemIndex);
    qDebug() << episode->fullName;
    QString episodeName = episode->fullName;
    qDebug()<<"Fetching servers for episode" << episodeName;
    qDebug()<<"Playlist index:" << itemIndex + 1 << "/" << playlist->count ();

    auto provider = ShowManager::instance ().getProvider (playlist->provider);
    QVector<VideoServer> servers =  provider->loadServers (*episode);

    if(!servers.empty ())
    {
        qDebug()<<"Successfully fetched servers for" << episodeName;
        QString source = provider->extractSource(servers[0]);
        if(playlist->sourceLink == ShowManager::instance().getCurrentShow ().link)
        { //todo add source link
            ShowManager::instance().setLastWatchedIndex (itemIndex);
        }
        playlist->currentIndex = itemIndex;
        //            emit currentIndexChanged();
        return source;
    }
    return "";
}

void PlaylistModel::setLaunchFile(const QString& path)
{
    m_onLaunchFile = QUrl::fromLocalFile(path).toString ();
}

void PlaylistModel::loadFolder(const QUrl &path)
{
    beginInsertRows (QModelIndex(), 0, 0);
    //    PlaylistItem("",-1,rootItem);
    rootItem->getChildren ()->emplaceBack (std::make_shared<PlaylistItem>("folder", -1, rootItem.get ()));
    rootItem->getChildren ()->last ()->loadFromLocalDir (path,rootItem->getChildren ()->last ().get ());
    //    rootItem->getChildren ()->emplaceBack (path, rootItem);
    endInsertRows();
    emit showNameChanged();
    emit layoutChanged ();

}

void PlaylistModel::appendPlaylist(const ShowData &show, nlohmann::json *json)
{
    if(playlistSet.contains (show.link)) return; // prevents duplicate playlists from being added
    //        root.emplaceBack (PlaylistItem(show,json));
    playlistSet.insert (show.link);
}

void PlaylistModel::appendPlaylist(const QUrl &path)
{
    //    rootItem->emplaceBack (path); todo
    emit showNameChanged();
    emit layoutChanged ();
}

void PlaylistModel::replaceCurrentPlaylist(const ShowData &show, nlohmann::json *json)
{
    if( !rootItem->isEmpty () &&
        show.link == rootItem->getChildren ()->first ()->sourceLink &&
        show.title ==rootItem->getChildren ()->first ()->name)
    {
        return;
    }

    playlistSet.insert (playlistSet.begin (),show.link);
    if(!rootItem->getChildren ()->isEmpty ()) rootItem->getChildren ()->removeFirst ();
    //        rootItem->getChildren ()->insert (0,PlaylistItem2(show,json));
    emit layoutChanged ();
    emit showNameChanged();
}

void PlaylistModel::play(int playlistIndex, int itemIndex)
{
    if (playlistIndex < 0 || playlistIndex >= rootItem->count ()) return;
    qDebug() << "playlist ok" << rootItem.get()[playlistIndex].count ();
    qDebug() << "play row" << rootItem->first ()->count () << rootItem->at (playlistIndex)->count ();
    if (itemIndex < 0 || itemIndex >= rootItem->at (playlistIndex)->count ()) return;
    qDebug() << "itemIndex ok";
    setLoading(true);
    rootItem->currentIndex = playlistIndex;
    qDebug() << "starting to play";

    qDebug() << "root count 2 : " << rootItem->count ();
    qDebug() << "first Item count 2 : " << rootItem->first ()->count ();

    m_watcher.setFuture (QtConcurrent::run([playlistIndex, itemIndex,this]() {
        QString playUrl;
        auto playlist = rootItem->at (playlistIndex);
        auto item = playlist->at (itemIndex);
        qDebug() << "playlist type" << playlist->type;
        qDebug() << "item type" << item->type;
        if(item->type == PlaylistItem::ONLINE)
        {
            playUrl = loadOnlineSource (playlistIndex, itemIndex);
        }
        else if (item->type == PlaylistItem::LOCAL)
        {
            playUrl = playlist->loadLocalSource (itemIndex);
        }
        else
        {
            return setLoading (false);
        }
        qDebug() << playUrl;
        if(!playUrl.isEmpty ())
        {
            MpvObject::instance()->open (playUrl);
            emit sourceFetched ();
            if(rootItem.get ()[playlistIndex].m_watchListShowItem)
            {
                qDebug() << "has json";
                playlist->m_watchListShowItem->at("lastWatchedIndex") = itemIndex;
                emit updatedLastWatchedIndex();
            }
            playlist->currentIndex = itemIndex;
            //todo emit data changed
        }
        setLoading(false);
    }));
}

void PlaylistModel::syncList(const ShowData& show, nlohmann::json* json)
{
    if(!rootItem->isEmpty ())
    {
        rootItem->getChildren ()->removeAt (0);
    }
    rootItem->getChildren ()->insert (0, show.playlist);
    rootItem->currentIndex = 0;
    qDebug() << "sync row" << rootItem->first ()->count ();
    emit layoutChanged ();
    emit showNameChanged();
}

void PlaylistModel::loadOffset(int offset){
    play(rootItem->currentIndex, rootItem.get ()[rootItem->currentIndex].currentIndex + offset); //todo check bound
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || rootItem->isEmpty ())
        return 0;
    return rootItem->at (rootItem->currentIndex)->count ();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || rootItem->isEmpty ())
        return QVariant();
    //qDebug() <<"data " << index.row () << index.column () << index.isValid ();
    //    return "lol";
    auto item = rootItem->at (rootItem->currentIndex)->at(index.row ());
    //qDebug() << item->name;
    switch (role)
    {
    case TitleRole:
        return item->name;
        break;
    case NumberRole:
        return item->number;
        break;
    case NumberTitleRole:
    {
        if (item->number < 0)
        {
            return item->name;
        }
        QString lastWatchedEpisodeName = QString::number (item->number);
        if(!item->name.isEmpty ()){
            lastWatchedEpisodeName += "\n" + item->name;
        }
        return lastWatchedEpisodeName;
        break;
    }
    default:
        break;
    }
    return QVariant();
}

QHash<int, QByteArray> PlaylistModel::roleNames() const{
    QHash<int, QByteArray> names;
    names[TitleRole] = "title";
    names[NumberRole] = "number";
    names[NumberTitleRole] = "numberTitle";
    return names;
}
