#include "playlistmodel.h"
#include "Providers/showprovider.h"
#include "Player/mpv/mpvObject.h"
#include "Components/errorhandler.h"
#include "Explorer/showmanager.h"

bool PlaylistModel::setLaunchPath(const QString& pathString)
{
    if (pathString.startsWith ("http")) {
        m_launchPath = pathString;
        return true;
    }
    QFileInfo path = QFileInfo(pathString);
    auto playlist = PlaylistItem::fromLocalDir (path.filePath ());
    if (!playlist) return false;
    replaceCurrentPlaylist (playlist);
    m_launchPath = playlist->loadLocalSource (playlist->currentIndex);
    return true;
}

QUrl PlaylistModel::loadOnlineSource(int playlistIndex, int itemIndex)
{
    auto playlist = m_playlists.at (playlistIndex);
    auto episode = playlist->at (itemIndex);
    QString episodeName = episode->getFullName ();
    qDebug()<<"Log (Playlist): Fetching servers for episode" << episodeName;
    qDebug()<<"Log (Playlist): Playlist index:" << itemIndex + 1 << "/" << playlist->count ();
    // if(playlist->getProviderName ().isEmpty ()) return QUrl(QString::fromStdString (episode->link));

    ShowProvider *provider = playlist->getProvider();
    Q_ASSERT(provider);

    m_serverList.setServers (provider->loadServers (episode));
    if (!m_serverList.isEmpty ())
    {
        qDebug()<<"Log (Playlist): Successfully fetched servers for" << episodeName;
        QUrl source = provider->extractSource(m_serverList.at(0));
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
    //used in the info and the playlist model
    m_playlists.push_back (playlist);
    playlistSet.insert(playlist->link);
    endResetModel();
    emit showNameChanged();
}

void PlaylistModel::appendPlaylist(const QUrl &path)
{
    auto playlist = PlaylistItem::fromLocalDir (path.toString ());
    if(playlist)
        appendPlaylist (playlist);
}

void PlaylistModel::replaceCurrentPlaylist(PlaylistItem *playlist)
{
    if (!playlist) return;
    if (playlistSet.contains (playlist->link) && playlistSet.size () > 1)
    {
        beginResetModel();
        m_playlists.move (m_playlists.indexOf (playlist), 0);
        endResetModel();
        return;
    }
    if (!m_playlists.isEmpty ()) {
        playlistSet.erase (m_playlists.first()->link);
        if (--m_playlists.first()->useCount == 0) {
            delete m_playlists.first();
        }
        m_playlists.removeFirst ();
    }
    appendPlaylist (playlist);
    m_playlistIndex = 0;
}

void PlaylistModel::replaceCurrentPlaylist(const QUrl &path)
{
    auto playlist = PlaylistItem::fromLocalDir (path.toLocalFile ());
    replaceCurrentPlaylist (playlist);
}

void PlaylistModel::play(int playlistIndex, int itemIndex = -1)
{
    // check if the indices are valid
    if (playlistIndex < 0 || playlistIndex >= m_playlists.count ()) return;
    if (itemIndex < -1 || itemIndex >= m_playlists[playlistIndex]->count ()) return;

    m_playlistIndex = playlistIndex;
    emit playlistIndexChanged();

    setLoading(true);
    auto playlist = m_playlists.at (m_playlistIndex);
    itemIndex = itemIndex == -1 ? playlist->currentIndex : itemIndex;
    auto item = playlist->at (itemIndex);

    switch (item->type) {
    case PlaylistItem::ONLINE: {
        m_watcher.setFuture(QtConcurrent::run([itemIndex, this]() {
            return loadOnlineSource(m_playlistIndex, itemIndex);
        }));

        break;
    }
    case PlaylistItem::LOCAL: {

        m_watcher.setFuture(QtConcurrent::run([playlist, itemIndex]() { return playlist->loadLocalSource(itemIndex); }));
        break;
    }
    case PlaylistItem::LIST: {
        qDebug() << "Attempted to play a list";
        return;
    }
    }

    m_watcher.future ()
        .then([this, itemIndex, playlist](QUrl playUrl)
              {
                  if (!playUrl.isEmpty ())
                  {
                      // open the url in with mpv
                      // qDebug() << "Attempting to play" << playUrl;
                      MpvObject::instance()->open (playUrl);
                      emit sourceFetched ();
                      // update the lastwatchedindex in watch list
                      playlist->currentIndex = itemIndex;
                      emit currentIndexChanged ();
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
    if(m_playlistIndex >= m_playlists.count ()) return;
    if(nextVideoSource.isValid ())
    {
        MpvObject::instance()->open (nextVideoSource);
        emit sourceFetched ();
        m_playlists[m_playlistIndex]->currentIndex += offset;

        emit playlistItemIndexChanged();
    }
    play(m_playlistIndex, m_playlists[m_playlistIndex]->currentIndex + offset);
}

QModelIndex PlaylistModel::getCurrentIndex() {
    if (m_playlists.isEmpty() || m_playlistIndex < 0 || m_playlistIndex >= m_playlists.size())
        return QModelIndex();

    PlaylistItem* currentPlaylist = m_playlists.at(m_playlistIndex);
    if (currentPlaylist->currentIndex < 0 || currentPlaylist->currentIndex >= currentPlaylist->count())
        return QModelIndex();

    // Assuming 'currentPlaylist' is a top-level item and items within it are its children
    return index(currentPlaylist->currentIndex, 0, index(m_playlistIndex, 0, QModelIndex()));
}


int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;
    return parent.isValid() ? static_cast<PlaylistItem*>(parent.internalPointer())->count () : m_playlists.count ();
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid()) {
        // Top-level items
        return createIndex(row, column, m_playlists.at(row));
    } else {
        // Child items
        PlaylistItem* parentItem = static_cast<PlaylistItem*>(parent.internalPointer());
        PlaylistItem* childItem = parentItem->at(row);
        return childItem ? createIndex(row, column, childItem) : QModelIndex();
    }
}

QModelIndex PlaylistModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    PlaylistItem* childItem = static_cast<PlaylistItem*>(index.internalPointer());
    PlaylistItem* parentItem = childItem->parent();

    if (parentItem == nullptr)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
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

