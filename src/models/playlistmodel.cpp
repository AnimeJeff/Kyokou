#include "playlistmodel.h"


void PlaylistModel::setLaunchFolder(const QString& path){
    loadFolder (QUrl::fromLocalFile(path));
    //todo move to later in qml
    if(m_playlists.first().size () == 0){
        qWarning() << "Directory has no playable items.";
        return;
    }
    m_onLaunchFile = QString::fromStdString (m_playlists.first().get(this->m_currentIndex).link);
}

void PlaylistModel::setLaunchFile(const QString& path){
    m_onLaunchFile = QUrl::fromLocalFile(path).toString ();
}

void PlaylistModel::loadFolder(const QUrl &path){
    m_playlists.push_back (Playlist(path));
    emit showNameChanged();
    emit layoutChanged ();

}

void PlaylistModel::play(int index){
    setLoading(true);
    m_watcher.setFuture (QtConcurrent::run([index,this]() {
        auto playUrl = m_playlists.first().load (index);
        if(!playUrl.isEmpty ()){
            MpvObject::instance()->open (playUrl);
            emit sourceFetched ();
            if(m_playlists.first().m_watchListShowItem){
                m_playlists.first().m_watchListShowItem->at("lastWatchedIndex")= index;
                emit updatedLastWatchedIndex();
            }
            this->m_currentIndex = index;
            emit currentIndexChanged();
        }
        setLoading(false);
    }));
}

void PlaylistModel::syncList(const ShowData& show,nlohmann::json* json){

    if(!m_playlists.isEmpty () && show.link == m_playlists.first ().sourceLink
        && show.title == m_playlists.first().name)return;
    //m_playlists.removeFirst ();
    m_playlists.insert (0,Playlist(show,json));
    emit layoutChanged ();
    emit showNameChanged();
}

void PlaylistModel::loadOffset(int offset){
    auto newIndex = m_currentIndex+offset;
    play(newIndex);
    //    if(m_currentPlaylist->isValidIndex(newIndex)){
    //        qDebug()<<"Loading offset "<<newIndex;
    //        loadSource(newIndex);
    //    }
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || m_playlists.isEmpty ())
        return 0;
    return m_playlists.constFirst ().size ();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if ( !index.isValid() || m_playlists.isEmpty () )
        return QVariant();

    const Episode& episode = m_playlists.at (index.row()).get (index.column ());
    switch (role) {
    case TitleRole:
        return episode.title;
        break;
    case NumberRole:
        return episode.number;
        break;
    case NumberTitleRole:{
        QString lastWatchedEpisodeName = QString::number (episode.number);
        if(!(episode.title.isEmpty () || episode.title.toInt () == episode.number)){
            lastWatchedEpisodeName += "\n" + episode.title;
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
