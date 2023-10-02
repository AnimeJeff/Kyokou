#include "playlistmodel.h"
#include "Providers/showprovider.h"
#include "Player/mpv/mpvObject.h"
#include "Components/ErrorHandler.h"
#include "Explorer/showmanager.h"

bool PlaylistModel::setLaunchPath(const QString& pathString)
{
    if (pathString.startsWith ("http"))
    {
        m_launchPath = pathString;
        return true;
    }
    QFileInfo path = QFileInfo(pathString);
    qDebug()<<"File" << pathString << "exists: " << path.exists ();
    if (!path.exists ()) return false;
    path.makeAbsolute ();
    if (path.isDir ())
    {
        auto playlist = PlaylistItem::fromLocalDir (path.filePath ());
        if(!playlist) return false;
        replaceCurrentPlaylist (playlist);
        m_launchPath = playlist->loadLocalSource (playlist->currentIndex);
    }
    else
    {
        auto validSuffixes = QStringList{"mp4", "mkv", "avi", "mp3", "flac", "wav", "ogg", "webm"};
        if (validSuffixes.contains (path.suffix ()))
        {
            m_launchPath = QUrl::fromLocalFile (path.filePath ());
        }
    }
    qDebug()<<m_launchPath;
    return true;
}

void PlaylistModel::loadFromEpisodeList(int index)
{
    replaceCurrentPlaylist (ShowManager::instance().getCurrentShow().playlist);
    play (0, index);
}

void PlaylistModel::continueFromLastWatched()
{
    replaceCurrentPlaylist (ShowManager::instance().getCurrentShow().playlist);
    int continueIndex = m_playlists[m_playlistIndex]->currentIndex;
    if (continueIndex < 0) continueIndex = 0;
    else if (continueIndex == m_playlists[m_playlistIndex]->count () - 2) ++continueIndex;
    play (0, continueIndex);
}

QUrl PlaylistModel::loadOnlineSource(int playlistIndex, int itemIndex)
{
    //lazy show in watchlist
    auto playlist = m_playlists.at (playlistIndex);
    if(playlist->link.ends_with (".m3u8"))
    {
        return QString::fromStdString (playlist->link);
    }
    auto episode = playlist->at (itemIndex);
    QString episodeName = episode->fullName;
    qDebug()<<"Fetching servers for episode" << episodeName;
    qDebug()<<"Playlist index:" << itemIndex + 1 << "/" << playlist->count ();
    auto provider = ShowManager::instance ().getProvider (playlist->provider);
    QVector<VideoServer> servers =  provider->loadServers (episode);
    if(!servers.empty ())
    {
        qDebug()<<"Successfully fetched servers for" << episodeName;
        QUrl source = provider->extractSource(servers[0]);
        if(playlist->link == ShowManager::instance().getCurrentShow ().link)
        {
            ShowManager::instance().setLastWatchedIndex (itemIndex);
        }
        playlist->currentIndex = itemIndex;
        //            emit currentIndexChanged();
        return source;
    }
    ErrorHandler::instance ().show ("Failed to load " + episodeName);
    return QUrl();
}

void PlaylistModel::appendPlaylist(PlaylistItem *playlist)
{
    if (!playlist || playlistSet.contains (playlist->link)) return;
    beginResetModel();
    ++playlist->useCount;
    qDebug() <<"playlist model" << playlist->useCount;
    m_playlists.push_back (playlist);
    playlistSet.insert(playlist->link);
    endResetModel();
    emit showNameChanged();
}

void PlaylistModel::appendPlaylist(const QUrl &path)
{
    auto playlist = PlaylistItem::fromLocalDir (path.toString ());
    appendPlaylist (playlist);
}

void PlaylistModel::replaceCurrentPlaylist(PlaylistItem *playlist)
{
    if (!playlist) return;
    if(playlistSet.contains (playlist->link))
    {
        beginResetModel();
        m_playlists.move (m_playlists.indexOf (playlist), 0);
        endResetModel();
        return;
    }
    if(!m_playlists.isEmpty ())
    {
        playlistSet.erase (m_playlists.first()->link);
        if(--m_playlists.first ()->useCount == 0)
        {
            delete m_playlists.first ();
        }

        m_playlists.removeFirst ();
    }
    appendPlaylist (playlist);
    m_playlistIndex = 0;
}

void PlaylistModel::replaceCurrentPlaylist(const QUrl &path)
{
    auto playlist = PlaylistItem::fromLocalDir (path.toString ());
    replaceCurrentPlaylist (playlist);
}

void PlaylistModel::play(int playlistIndex, int itemIndex)
{
    if (playlistIndex < 0 || playlistIndex >= m_playlists.count ()) return;
    if (itemIndex < 0 || itemIndex >= m_playlists[playlistIndex]->count ()) return;
    setLoading(true);
    auto playlist = m_playlists.at (playlistIndex);
    auto item = playlist->at (itemIndex);
    m_playlistIndex = playlistIndex;
    switch(item->type)
    {
    case PlaylistItem::ONLINE:
    {
        m_watcher.setFuture (QtConcurrent::run([playlistIndex, itemIndex, this]() {
            return QUrl(loadOnlineSource (playlistIndex, itemIndex));
        }));
        break;
    }
    case PlaylistItem::LOCAL:
    {
        qDebug() << playlist->name << playlist->count ();
        auto url = playlist->loadLocalSource (itemIndex);
        m_watcher.setFuture (QtConcurrent::run([url]() {
            return url;
        }));
        break;
    }
    case PlaylistItem::LIST:
    {
        qDebug() << "Trying to play a list";
        return;
    }
    }

    m_watcher.future ()
        .then([this,playlistIndex,itemIndex,playlist](QUrl playUrl)
              {
                  if(!playUrl.isEmpty ())
                  {
                      MpvObject::instance()->open (playUrl);
                      emit sourceFetched ();
                      if (m_playlists[playlistIndex]->m_watchListShowItem)
                      {

                          playlist->m_watchListShowItem->at("lastWatchedIndex") = itemIndex;
                          if (ShowManager::instance().getCurrentShow().link == playlist->link)
                          {
                              ShowManager::instance().setLastWatchedIndex (itemIndex);
                          }
                          emit updatedLastWatchedIndex();
                      }
                      bool emitted = false;
                      emit currentIndexChanged ();
                      if (m_playlistIndex != playlistIndex)
                      {
                          m_playlistIndex = playlistIndex;
                          emitted = true;
                          emit playlistIndexChanged();
                      }
                      if (playlist->currentIndex != itemIndex)
                      {
                          playlist->currentIndex = itemIndex;
                          if (!emitted) emit currentIndexChanged ();
                          emit playlistItemIndexChanged();
                      }
                  }
                  setLoading(false);
              })
        .onFailed ([](const std::exception& e){
            qDebug() << e.what ();
        })
        .onCanceled ([](){
            qDebug() << "Operation Cancelled";
        })
        ;
}

void PlaylistModel::loadOffset(int offset)
{
    play(m_playlistIndex, m_playlists[m_playlistIndex]->currentIndex + offset); //todo check bound
}

QModelIndex PlaylistModel::getCurrentIndex()
{
    if(m_playlists.isEmpty () || m_playlists[m_playlistIndex]->currentIndex < 0) return QModelIndex();
    return createIndex(m_playlists[m_playlistIndex]->currentIndex, 0, m_playlists[m_playlistIndex]->at (m_playlists[m_playlistIndex]->currentIndex));
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
    {
        return m_playlists.count ();
    }
    else
        return static_cast<PlaylistItem*>(parent.internalPointer())->count ();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_playlists.isEmpty ())
        return QVariant();
    auto item = static_cast<PlaylistItem*>(index.internalPointer());
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
        if (item->type == 0)
        {
            return item->name;
        }

        QString lastWatchedEpisodeName = item->number < 0 ? item->name + "\n" : QString::number (item->number) + "\n" + item->name;
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

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column, m_playlists.at (row));
    else
    {
        PlaylistItem *parentItem;
        parentItem = static_cast<PlaylistItem*>(parent.internalPointer());
        const PlaylistItem *childItem = parentItem->at (row);
        if (childItem)
        {
            return createIndex(row, column, childItem);
        }
    }

    return QModelIndex();
}

QModelIndex PlaylistModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    auto childItem = static_cast<PlaylistItem*>(index.internalPointer());
    auto parentItem = childItem->parent ();
    if (parentItem == nullptr)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}


