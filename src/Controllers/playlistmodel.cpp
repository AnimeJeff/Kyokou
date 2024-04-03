#include "playlistmodel.h"
#include "Mpv/mpvObject.h"
#include "Providers/showprovider.h"



QList<Video> PlaylistModel::loadOnlineSource(int playlistIndex, int itemIndex) {
    auto playlist = m_playlists.at(playlistIndex);
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
    ++playlist->useCount;
    // playlist might also be used in the episodelistmodel and the downloader
    m_playlists.push_back(playlist);
    playlistSet.insert(playlist->link);
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

    if (playlistSet.contains(playlist->link) && playlistSet.size() > 1) {
        m_playlistIndex = m_playlists.indexOf(playlist);
    } else {
        if (!m_playlists.isEmpty()) {
            playlistSet.remove(m_playlists.first()->link);
            if (--m_playlists.first()->useCount == 0) {
                delete m_playlists.first();
            }
            m_playlists.removeFirst();
        }
        appendPlaylist(playlist);
    }

    m_playlistIndex = 0;
}


bool PlaylistModel::play(int playlistIndex, int itemIndex) {
    // Check if the indices are valid
    if (playlistIndex < -1 || playlistIndex >= m_playlists.count())
        return false;
    setLoading(true);
    // Set to current playlist index if -1
    m_playlistIndex = playlistIndex == -1 ? m_playlistIndex : playlistIndex;
    auto playlist = m_playlists.at(m_playlistIndex);

    // Set to current playlist item index if -1
    itemIndex = itemIndex == -1 ? playlist->currentIndex : itemIndex;
    if (!m_playlists[m_playlistIndex]->isValidIndex (itemIndex))
        return false;

    int seekTime = playlist->at(itemIndex)->lastPlayTime;
    // qDebug() << "seek time" << itemIndex << seekTime;
    switch (playlist->at(itemIndex)->type) {
    case PlaylistItem::ONLINE: {
        m_watcher.setFuture(QtConcurrent::run([itemIndex, this]() {
            return loadOnlineSource(m_playlistIndex, itemIndex);
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
                playlist->currentIndex = itemIndex;
                emit currentIndexChanged();
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
    if (m_playlistIndex >= m_playlists.count())
        return;
    play(m_playlistIndex, m_playlists[m_playlistIndex]->currentIndex + offset);
}

QModelIndex PlaylistModel::getCurrentIndex() {
    if (m_playlists.isEmpty() || m_playlistIndex < 0 ||
        m_playlistIndex >= m_playlists.size())
        return QModelIndex();

    PlaylistItem *currentPlaylist = m_playlists.at(m_playlistIndex);
    if (!currentPlaylist->isValidIndex (currentPlaylist->currentIndex))
        return QModelIndex();

    // Assuming 'currentPlaylist' is a top-level item and items within it are its
    return index(currentPlaylist->currentIndex, 0, index(m_playlistIndex, 0, QModelIndex()));
}

int PlaylistModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0)
        return 0;
    return parent.isValid()
               ? static_cast<PlaylistItem *>(parent.internalPointer())->size()
               : m_playlists.count();
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid()) {
        // Top-level items
        return createIndex(row, column, m_playlists.at(row));
    } else {
        // Child items
        PlaylistItem *parentItem =
            static_cast<PlaylistItem *>(parent.internalPointer());
        PlaylistItem *childItem = parentItem->at(row);
        return childItem ? createIndex(row, column, childItem) : QModelIndex();
    }
}

QModelIndex PlaylistModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    PlaylistItem *childItem =
        static_cast<PlaylistItem *>(index.internalPointer());
    PlaylistItem *parentItem = childItem->parent();

    if (parentItem == nullptr)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || m_playlists.isEmpty())
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
