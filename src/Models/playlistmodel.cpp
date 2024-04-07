#include "playlistmodel.h"
#include "Mpv/mpvObject.h"
#include "Providers/showprovider.h"
#include "Components/errorhandler.h"

PlaylistModel::PlaylistModel(const QString &launchPath, QObject *parent) {
    // Opens the file to play immediately when application launches

    connect (&m_folderWatcher, &QFileSystemWatcher::directoryChanged, this, &PlaylistModel::onLocalDirectoryChanged);

    connect (&m_watcher, &QFutureWatcher<void>::finished, this, [this](){
        if (!m_watcher.future().isValid()) {
            //future was cancelled
            ErrorHandler::instance().show ("Operation cancelled", "");
        }
        try {
            m_watcher.waitForFinished ();
        } catch (MyException& ex) {
            ErrorHandler::instance().show (ex.what (), "Playlist Error");
        } catch(const std::runtime_error& ex) {
            ErrorHandler::instance().show (ex.what (), "Playlist Error");
        } catch (...) {
            ErrorHandler::instance().show ("Something went wrong", "Playlist Error");
        }

        m_isLoading = false;
        emit isLoadingChanged ();
        m_errorMessage.clear ();
    });

    if (!launchPath.isEmpty ()) {
        QUrl url = QUrl::fromUserInput(launchPath);
        openUrl (url, false);
    }
}

void PlaylistModel::openUrl(const QUrl &url, bool playUrl) {
    if (!url.isValid ()) return;
    PlaylistItem *playlist = nullptr;
    QString urlString = url.toString ();

    if (url.isLocalFile ()) {
        playlist = PlaylistItem::fromLocalUrl (url);
        replaceCurrentPlaylist (playlist);
    } else {
        // Get the playlist for pasted online videos
        auto pastePlaylistIndex = m_rootPlaylist->indexOf ("videos");
        if (pastePlaylistIndex == -1) {
            // Create a new one
            playlist = new PlaylistItem("Videos", nullptr, "videos");
            m_rootPlaylist->currentIndex = m_rootPlaylist->size ();
            appendPlaylist (playlist);
        } else {
            // Set the index to that index
            m_rootPlaylist->currentIndex = pastePlaylistIndex;
            playlist = m_rootPlaylist->getCurrentItem ();
        }
        // Add the url to the playlist
        playlist->currentIndex = playlist->size ();
        playlist->emplaceBack (playlist->size () + 1, urlString, urlString, true);
    }

    if (playUrl && playlist) {
        tryPlay ();
    }

}

void PlaylistModel::pasteOpen() {
    QString clipboardText = QGuiApplication::clipboard()->text().trimmed ();
    auto url = QUrl::fromUserInput (clipboardText);
    if (!url.isValid ()) return;

    MpvObject::instance ()->showText (QByteArrayLiteral("Pasting ") + clipboardText.toUtf8 ());
    QString extension = QFileInfo(url.path()).suffix();

    qDebug() << "Extension:" << extension;
    QStringList subtitleExtensions = {
        "srt", "sub", "ssa", "ass", "idx", "vtt",
    };
    if (subtitleExtensions.contains(extension)) {
        MpvObject::instance ()->addSubtitle(url);
        MpvObject::instance ()->setSubVisible(true);
    } else {
        static QRegularExpression urlPattern(R"(https?:\/\/(?:www\.|(?!www))[a-zA-Z0-9][a-zA-Z0-9-]+[a-zA-Z0-9]\.[^\s]{2,}|www\.[a-zA-Z0-9][a-zA-Z0-9-]+[a-zA-Z0-9]\.[^\s]{2,}|https?:\/\/(?:www\.|(?!www))[a-zA-Z0-9]+\.[^\s]{2,}|www\.[a-zA-Z0-9]+\.[^\s]{2,})");
        QRegularExpressionMatch match = urlPattern.match(clipboardText);
        if (!match.hasMatch () || MpvObject::instance ()->getCurrentVideoUrl() == url)
            return;

        openUrl(url, true);
        qDebug() << match.captured(0);

    }

}

bool PlaylistModel::tryPlay(int playlistIndex, int itemIndex) {
    if (m_watcher.isRunning ()) return false;

    // Set to current playlist index if -1
    playlistIndex = playlistIndex == -1 ? (m_rootPlaylist->currentIndex == -1 ? 0 : m_rootPlaylist->currentIndex) : playlistIndex;

    if (!m_rootPlaylist->isValidIndex (playlistIndex))
        return false;
    auto newPlaylist = m_rootPlaylist->at (playlistIndex);

    // Set to current playlist item index if -1
    if (newPlaylist) {
        itemIndex = itemIndex == -1 ? (newPlaylist->currentIndex == -1 ? 0 : newPlaylist->currentIndex) : itemIndex;
        if (!newPlaylist->isValidIndex (itemIndex) || newPlaylist->at (itemIndex)->type == PlaylistItem::LIST) {
            qWarning() << "Invalid index or attempting to play a list";
            return false;
        }
    } else return false;

    m_isLoading = true;
    emit isLoadingChanged();

    // If same playlist, update the time stamp for the last item


    m_watcher.setFuture(QtConcurrent::run(&PlaylistModel::play, this, playlistIndex, itemIndex));
    return true;

}

void PlaylistModel::play(int playlistIndex, int itemIndex) {
    auto playlist = m_rootPlaylist->at(playlistIndex);
    auto episode = playlist->at(itemIndex);

    qDebug() << "Log (Playlist): Timestamp:" << playlist->at(itemIndex)->timeStamp;
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
        if (!provider){
            m_errorMessage = "Cannot get provider from playlist!";
            return;
        }
        qInfo().noquote() << QString("Log (Playlist): Fetching servers for episode %1 [%2/%3]")
                                 .arg (episodeName).arg (itemIndex + 1).arg (playlist->size());

        QList<VideoServer> servers = provider->loadServers(episode);
        if (servers.isEmpty()) {
            throw MyException("No servers found for " + episodeName);
        }
        qInfo() << "Log (Playlist): Successfully fetched servers for" << episodeName;

        QPair<QList<Video>, int> sourceAndIndex = m_serverList.autoSelectServer(servers, provider);

        videos = sourceAndIndex.first;
        if (!videos.isEmpty ()) {
            m_serverList.setServers(servers, provider, sourceAndIndex.second);
        }
    }

    if (videos.isEmpty ()) {
        throw MyException("No sources extracted from " + episodeName);
        return;
    }

    qInfo() << "Log (Playlist): Fetched source" << videos.first().videoUrl;
    MpvObject::instance()->open(videos.first(), episode->timeStamp);
    // Update current item index only if videos are returned
    auto currentPlaylist = m_rootPlaylist->getCurrentItem ();
    if (currentPlaylist == playlist && currentPlaylist->currentIndex != -1 && currentPlaylist->currentIndex != itemIndex) {
        auto time = MpvObject::instance ()->time ();
        if (time > 0.85 * MpvObject::instance ()->duration ())
            time = 0;
        qInfo() << "Log (Playlist): Saving timestamp" << time << "for" << currentPlaylist->getCurrentItem ()->link;
        currentPlaylist->setLastPlayAt(currentPlaylist->currentIndex, time);
    }

    m_rootPlaylist->currentIndex = playlistIndex;
    playlist->currentIndex = itemIndex;
    emit aboutToPlay();
    emit currentIndexChanged();


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
    if (parentItem == m_rootPlaylist) return;
    int itemIndex = childItem->row();
    int playlistIndex = m_rootPlaylist->indexOf(parentItem);
    play(playlistIndex, itemIndex);
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

    PlaylistItem *parentItem = parent.isValid() ? static_cast<PlaylistItem *>(parent.internalPointer()) : m_rootPlaylist;
    PlaylistItem *childItem = parentItem->at(row);
    return childItem ? createIndex(row, column, childItem) : QModelIndex();
}

QModelIndex PlaylistModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    PlaylistItem *childItem = static_cast<PlaylistItem *>(index.internalPointer());
    PlaylistItem *parentItem = childItem->parent();

    if (parentItem == m_rootPlaylist)
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
