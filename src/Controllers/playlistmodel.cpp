#include "playlistmodel.h"
#include "Mpv/mpvObject.h"
#include "Providers/showprovider.h"



QList<Video> PlaylistModel::loadOnlineSource(int playlistIndex, int itemIndex) {
    auto playlist = m_rootPlaylist->at(playlistIndex);
    auto episode = playlist->at(itemIndex);
    QString episodeName = episode->getFullName();
    qInfo() << QString("Log (Playlist): Fetching servers for episode %1 [%2/%3]")
                   .arg (episodeName).arg (itemIndex + 1).arg (playlist->size());

    ShowProvider *provider = playlist->getProvider();
    Q_ASSERT(provider);

    m_serverList.setServers(provider->loadServers(episode), provider);
    if (m_serverList.isEmpty()) {
        ErrorHandler::instance().show("Failed to load " + episodeName);
        return {};
    }
    qInfo() << "Log (Playlist): Successfully fetched servers for" << episodeName;
    return m_serverList.load(-1);
}

void PlaylistModel::appendPlaylist(PlaylistItem *playlist) {
    if (!playlist || playlistSet.contains(playlist->link))
        return;

    beginResetModel();
    registerPlaylist(playlist);
    // Playlist might also be used in the episodelistmodel and the downloader
    m_rootPlaylist->append(playlist);
    endResetModel();
    emit showNameChanged();
}

void PlaylistModel::appendPlaylist(const QUrl &path) {
    auto playlist = PlaylistItem::fromUrl(path.toString());
    if (playlist)
        appendPlaylist(playlist);
}

void PlaylistModel::replaceCurrentPlaylist(PlaylistItem *playlist) {
    if (!playlist) return;
    if (playlistSet.contains(playlist->link)) {
        int index = m_rootPlaylist->indexOf(playlist->link);
        Q_ASSERT(index != -1);
        m_rootPlaylist->currentIndex = index;
        return;
    }

    if (auto currentPlaylist = m_rootPlaylist->currentItem (); currentPlaylist) {
        // Deregister the current playlist
        deregisterPlaylist(currentPlaylist);

        // Register the new playlist
        registerPlaylist(playlist);

        // Replace the current playlist with the new playlist
        beginResetModel();
        m_rootPlaylist->replace (m_rootPlaylist->currentIndex, playlist);
        endResetModel();
        emit showNameChanged();

    } else {
        // Playlist is empty
        appendPlaylist (playlist);
    }


}

bool PlaylistModel::play(int playlistIndex, int itemIndex) {
    if (m_watcher.isRunning ()) return false;

    // Set to current playlist index if -1
    playlistIndex = playlistIndex == -1 ? m_rootPlaylist->currentIndex : playlistIndex;

    if (!m_rootPlaylist->isValidIndex (playlistIndex))
        return false;

    m_rootPlaylist->currentIndex = playlistIndex;
    auto playlist = m_rootPlaylist->currentItem ();

    setLoading(true);


    // Set to current playlist item index if -1
    itemIndex = itemIndex == -1 ? playlist->currentIndex : itemIndex;
    if (!playlist->isValidIndex (itemIndex))
        return false;
    playlist->currentIndex = itemIndex;
    emit currentIndexChanged();

    int seekTime = playlist->at(itemIndex)->lastPlayTime;
    qDebug() << "seek time" << itemIndex << seekTime;
    switch (playlist->at(itemIndex)->type) {
    case PlaylistItem::ONLINE: {
        m_watcher.setFuture(QtConcurrent::run([itemIndex, this]() {
            return loadOnlineSource(m_rootPlaylist->currentIndex, itemIndex);
        }));
        break;
    }
    case PlaylistItem::LOCAL: {
        m_watcher.setFuture(QtConcurrent::run([playlist, itemIndex]() {
            auto source = QList<Video>{Video(playlist->loadLocalSource(itemIndex))};
            return source;
        }));
        break;
    }
    case PlaylistItem::LIST: {
        qWarning() << "Attempted to play a list";
        return false;
    }
    }

    m_watcher.future()
        .then([this, itemIndex, playlist, seekTime](QList<Video> videos) {
            if (!videos.isEmpty()) {
                qInfo() << "Log (Playlist): Fetched source"
                        << videos.first().videoUrl;

                MpvObject::instance()->open(videos.first(), seekTime);
                emit sourceFetched();
                // Update current item index if videos are returned

            } else {
                ErrorHandler::instance().show(
                    "Failed to get a playable source for this episode");
            }
            setLoading(false);

        })
        .onFailed([this](const std::exception &e) { qDebug() << e.what();setLoading(false); })
        .onCanceled([this]() { qDebug() << "Operation Cancelled";setLoading(false); });
    return true;
}

void PlaylistModel::loadOffset(int offset) {
    auto currentPlaylist = m_rootPlaylist->currentItem ();
    int newIndex = currentPlaylist->currentIndex + offset;
    if (!currentPlaylist->isValidIndex (newIndex)) return;
    play(m_rootPlaylist->currentIndex, newIndex);
}

QModelIndex PlaylistModel::getCurrentIndex() {
    PlaylistItem *currentPlaylist = m_rootPlaylist->currentItem ();
    if (!currentPlaylist ||
        !currentPlaylist->isValidIndex (currentPlaylist->currentIndex))
        return QModelIndex();

    return index(currentPlaylist->currentIndex, 0, index(m_rootPlaylist->currentIndex, 0, QModelIndex()));
}

int PlaylistModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0)
        return 0;
    return parent.isValid()
               ? static_cast<PlaylistItem *>(parent.internalPointer())->size()
               : m_rootPlaylist->size();
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    PlaylistItem *parentItem = parent.isValid() ? static_cast<PlaylistItem *>(parent.internalPointer()) : m_rootPlaylist.get ();
    PlaylistItem *childItem = parentItem->at(row);
    return childItem ? createIndex(row, column, childItem) : QModelIndex();
}

QModelIndex PlaylistModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    PlaylistItem *childItem = static_cast<PlaylistItem *>(index.internalPointer());
    PlaylistItem *parentItem = childItem->parent();

    if (parentItem == m_rootPlaylist.get ())
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || m_rootPlaylist->isEmpty ())
        return QVariant();
    auto item = static_cast<PlaylistItem *>(index.internalPointer());
    switch (role) {
    case TitleRole:
        return item->name;
        break;
    case NumberRole:
        return item->number;
        break;
    case NumberTitleRole: {
        if (item->type == 0) {
            return item->name;
        }

        QString lastWatchedEpisodeName =
            item->number < 0 ? item->name + "\n"
                             : QString::number(item->number) + "\n" + item->name;
        return lastWatchedEpisodeName;
        break;
    }
    default:
        break;
    }
    return QVariant();
}

QHash<int, QByteArray> PlaylistModel::roleNames() const {
    QHash<int, QByteArray> names;
    names[TitleRole] = "title";
    names[NumberRole] = "number";
    names[NumberTitleRole] = "numberTitle";
    return names;
}
