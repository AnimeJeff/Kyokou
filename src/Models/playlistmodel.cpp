#include "playlistmodel.h"
#include "Mpv/mpvObject.h"
#include "Providers/showprovider.h"


PlaylistModel::PlaylistModel(const QString &launchPath, QObject *parent) {
    // Opens the file to play immediately when application launches

    connect (&m_folderWatcher, &QFileSystemWatcher::directoryChanged, this, &PlaylistModel::onLocalDirectoryChanged);

    connect (&m_watcher, &QFutureWatcher<void>::finished, this, [this](){
        if (!m_watcher.future().isValid()) {
            //future was cancelled
            ErrorHandler::instance().show ("Operation cancelled");
        }
        try {
            m_watcher.waitForFinished ();
        } catch (QException& ex) {
            ErrorHandler::instance().show (ex.what ());
        }
        m_isLoading = false;
        emit isLoadingChanged ();
    });

    if (!launchPath.isEmpty ()) {
        auto url = QUrl::fromUserInput(launchPath);
        auto playlist = PlaylistItem::fromUrl(url);
        if (playlist) {
            replaceCurrentPlaylist(playlist);
            qDebug() << "Log (Playlist): Successfully opened launch path";
        }
    }
}

bool PlaylistModel::tryPlay(int playlistIndex, int itemIndex) {
    if (m_watcher.isRunning ()) return false;

    // Set to current playlist index if -1
    playlistIndex = playlistIndex == -1 ? (m_rootPlaylist->currentIndex == -1 ? 0 : m_rootPlaylist->currentIndex) : playlistIndex;

    if (!m_rootPlaylist->isValidIndex (playlistIndex))
        return false;
    auto playlist = m_rootPlaylist->at (playlistIndex);

    // Set to current playlist item index if -1
    if (playlist) {
        itemIndex = itemIndex == -1 ? (playlist->currentIndex == -1 ? 0 : playlist->currentIndex) : itemIndex;
        if (!playlist->isValidIndex (itemIndex) || playlist->at (itemIndex)->type == PlaylistItem::LIST) {
            qWarning() << "Index is invalid or attempted to play a list";
            return false;
        }
    } else return false;

    m_isLoading = true;
    emit isLoadingChanged();
    if (auto lastPlaylist = m_rootPlaylist->getCurrentItem (); lastPlaylist) {
        auto time = MpvObject::instance ()->time ();
        if (time > 0.9 * MpvObject::instance ()->duration ())
            time = 0;
        lastPlaylist->setLastPlayAt(lastPlaylist->currentIndex, time);
    }

    m_watcher.setFuture(QtConcurrent::run(&PlaylistModel::play, this, playlistIndex, itemIndex));
    return true;

}

void PlaylistModel::play(int playlistIndex, int itemIndex) {
    auto playlist = m_rootPlaylist->at(playlistIndex);
    auto episode = playlist->at(itemIndex);


    int seekTime = playlist->at(itemIndex)->timeStamp;
    qDebug() << "Log (Playlist): Seeking to" << seekTime;
    QString episodeName = episode->getFullName();

    QList<Video> videos;
    if (episode->type == PlaylistItem::LOCAL) {
        if (playlist->currentIndex != itemIndex){
            playlist->currentIndex = itemIndex;
            playlist->updateHistoryFile (0);
        }
        videos.emplaceBack (episode->link);
    } else {
        ShowProvider *provider = playlist->getProvider();
        Q_ASSERT(provider);
        qInfo() << QString("Log (Playlist): Fetching servers for episode %1 [%2/%3]")
                       .arg (episodeName).arg (itemIndex + 1).arg (playlist->size());
        auto servers = provider->loadServers(episode);
        if (servers.isEmpty()) {
            ErrorHandler::instance().show("No servers found for " + episodeName);
            return;
        }
        qInfo() << "Log (Playlist): Successfully fetched servers for" << episodeName;
        auto sourceAndIndex = m_serverList.autoSelectServer(servers, provider);
        videos = sourceAndIndex.first;
        if (!videos.isEmpty ()) {
            m_serverList.setServers(servers, provider, sourceAndIndex.second);
        }
    }

    if (!videos.isEmpty ()) {
        qInfo() << "Log (Playlist): Fetched source" << videos.first().videoUrl;
        MpvObject::instance()->open(videos.first(), episode->timeStamp);
        // Update current item index only if videos are returned
        m_rootPlaylist->currentIndex = playlistIndex;
        playlist->currentIndex = itemIndex;
        emit currentIndexChanged();
    }

}

void PlaylistModel::loadOffset(int offset) {
    auto currentPlaylist = m_rootPlaylist->getCurrentItem ();
    if (!currentPlaylist) return;
    int newIndex = currentPlaylist->currentIndex + offset;
    if (!currentPlaylist->isValidIndex (newIndex)) return;
    play(m_rootPlaylist->currentIndex, newIndex);
}

void PlaylistModel::onLocalDirectoryChanged(const QString &path) {
    // TODO
    int index = m_rootPlaylist->indexOf (path);
    qInfo() << "Log (Playlist): Path" << path << "changed" << index;

    if (index > -1) {
        beginResetModel();
        QString prevlink = index == m_rootPlaylist->currentIndex ? m_rootPlaylist->getCurrentItem ()->getCurrentItem ()->link : "";
        if (!m_rootPlaylist->at (index)->reloadFromFolder ()) {
            // Folder is empty, deleted, can't open history file etc.
            m_rootPlaylist->removeAt (index);
            m_rootPlaylist->currentIndex = m_rootPlaylist->isEmpty () ? -1 : 0;
            qWarning() << "Failed to reload folder" << m_rootPlaylist->at (index)->link;
        }
        endResetModel();
        emit currentIndexChanged ();
        QString newLink = index == m_rootPlaylist->currentIndex ? m_rootPlaylist->getCurrentItem ()->getCurrentItem ()->link : "";
        if (prevlink != newLink) {
            tryPlay ();
        }
    }
}

void PlaylistModel::loadIndex(QModelIndex index) {
    if (!index.isValid ()) return;
    auto childItem = static_cast<PlaylistItem *>(index.internalPointer());
    auto parentItem = childItem->parent();
    if (parentItem == m_rootPlaylist.get ()) return;
    int itemIndex = childItem->row();
    int playlistIndex = m_rootPlaylist->indexOf(parentItem);
    play(playlistIndex, itemIndex);
}

void PlaylistModel::pasteOpen() {
    QString clipboardText = QGuiApplication::clipboard()->text();
    qInfo() << "Log (mpv): Pasting" << clipboardText;
    MpvObject::instance ()->showText (QByteArray("Pasting ") + clipboardText.toUtf8 ());
    if (clipboardText.endsWith(".vtt")) {
        MpvObject::instance ()->addSubtitle(clipboardText);
        MpvObject::instance ()->setSubVisible(true);
    } else {
        auto playlist = PlaylistItem::fromUrl(QUrl::fromUserInput (clipboardText));
        if (playlist) {
            MpvObject::instance ()->stop ();
            replaceCurrentPlaylist(playlist);
            tryPlay ();
        }
    }
}

bool PlaylistModel::registerPlaylist(PlaylistItem *playlist) {
    if (!playlist || playlistSet.contains (playlist->link)) return false;
    playlistSet.insert(playlist->link);

    // Watch playlist path if local folder
    if (playlist->isLoadedFromFolder ()) {
        m_folderWatcher.addPath (playlist->link);
    }
    return true;
}

void PlaylistModel::deregisterPlaylist(PlaylistItem *playlist) {
    if (!playlist || !playlistSet.contains (playlist->link)) return;
    playlistSet.remove(playlist->link);

    // Unwatch playlist path if local folder
    if (playlist->isLoadedFromFolder ()) {
        m_folderWatcher.removePath (playlist->link);
    }
}

void PlaylistModel::appendPlaylist(PlaylistItem *playlist) {
    if (!registerPlaylist (playlist)) return;
    beginResetModel();
    m_rootPlaylist->append(playlist);
    endResetModel();
}

void PlaylistModel::replaceCurrentPlaylist(PlaylistItem *playlist) {
    if (!playlist) return;
    if (playlistSet.contains(playlist->link)) {
        int index = m_rootPlaylist->indexOf(playlist->link);
        Q_ASSERT(index != -1);
        m_rootPlaylist->currentIndex = index;
        return;
    }

    // Register the new playlist
    registerPlaylist(playlist);
    auto currentPlaylist = m_rootPlaylist->getCurrentItem ();

    beginResetModel();

    if (currentPlaylist) {
        // Deregister the current playlist
        deregisterPlaylist(currentPlaylist);
        // Replace the current playlist with the new playlist
        if (m_rootPlaylist->replace (m_rootPlaylist->currentIndex, playlist)) {
            qDebug() << "Log (Playlist): Replacing current index" << m_rootPlaylist->currentIndex << "with" << playlist->link;
            endResetModel();
            return;
        }
    }

    // Playlist is empty or replacing failed so we append the playlist
    if (!m_rootPlaylist->isEmpty ())
        m_rootPlaylist->currentIndex = m_rootPlaylist->size ();
    m_rootPlaylist->append(playlist);
    qDebug() << m_rootPlaylist->currentIndex;
    endResetModel();
}

QModelIndex PlaylistModel::getCurrentIndex() {
    PlaylistItem *currentPlaylist = m_rootPlaylist->getCurrentItem ();
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
