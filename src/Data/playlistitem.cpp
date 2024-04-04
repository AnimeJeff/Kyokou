#include "playlistitem.h"
#include <cmath>
#include "video.h"

bool PlaylistItem::loadFromFolder(const QUrl &pathUrl) {
    if (!m_isLoadedFromFolder) return false;
    clear ();

    QDir playlistDir;
    QString currentFilename = "";
    QString timeString = "";
    if (!pathUrl.isEmpty ()) {
        QFileInfo path = QFileInfo(pathUrl.toLocalFile ());
        if (!path.exists ()) {
            qDebug() << "Log (PlaylistItem): Path" << path << "doesn't exist";
            return false;
        }

        if (!path.isDir ()) {
            playlistDir = path.dir ();
            currentFilename = path.fileName ();
        } else {
            playlistDir = QDir(pathUrl.toLocalFile ());
        }
        //qDebug() << playlistDir <<playlistDir.dirName() << playlistDir.absolutePath ();
        m_historyFile = std::make_unique<QFile> (playlistDir.filePath(".mpv.history"));
        name = playlistDir.dirName();
        link = playlistDir.absolutePath ();

        // Read/update history file
        if (path.isFile()) {
            bool fileOpened = m_historyFile->open(QIODevice::WriteOnly | QIODevice::Text);
            if (!fileOpened) {
                qDebug() << "Log (PlaylistItem): Failed to open history file";
                return false;
            }
            m_historyFile->write(currentFilename.toUtf8());
            m_historyFile->close ();
        }
    } else {
        playlistDir = QDir(link);
        if (!playlistDir.exists ()) {
            qDebug() << "Log (PlaylistItem): Path" << link << "doesn't exist";
            currentIndex = -1;
            return false;
        }
    }


    QStringList fileNames = playlistDir.entryList(
        {"*.mp4", "*.mkv", "*.avi", "*.mp3", "*.flac", "*.wav", "*.ogg", "*.webm"}, QDir::Files);
    if (fileNames.isEmpty()) {
        qDebug() << "Log (PlaylistItem): No files to play in" << playlistDir.absolutePath ();
        currentIndex = -1;
        return false; //TODO
    }

    // Read history file
    if (currentFilename.isEmpty() && m_historyFile->exists ()) {
        // Open history file
        bool fileOpened = m_historyFile->open(QIODevice::ReadOnly | QIODevice::Text);
        if (!fileOpened) {
            qDebug() << "Log (PlaylistItem): Failed to open history file";
            currentIndex = -1;
            return false;
        }
        auto fileData = QTextStream(m_historyFile.get()).readAll().trimmed().split(":");
        m_historyFile->close();
        if (!fileData.isEmpty()) {
            currentFilename = fileData.first();
            if (fileData.size() == 2) {
                timeString = fileData.last();
            }
        }

    }




    PlaylistItem *currentItemPtr = nullptr;

    for (int i = 0; i < fileNames.count(); i++) {
        QRegularExpressionMatch match = fileNameRegex.match(fileNames[i]);
        QString title = match.hasMatch() ? match.captured("title").trimmed() : "";
        int itemNumber = match.hasMatch() ? !match.captured("number").isEmpty() ? match.captured("number").toInt() : i : i;
        auto childItem = new PlaylistItem(itemNumber, playlistDir.absoluteFilePath(fileNames[i]), title, this, true);
        childItem->useCount++;
        if (fileNames[i] == currentFilename) {
            // Set current index and time
            // qDebug() << fileNames[i];
            currentItemPtr = childItem;
        }
        m_children->push_back(childItem);
    }

    // sort the episodes in order
    std::stable_sort(m_children->begin(), m_children->end(),
                     [](const PlaylistItem *a, const PlaylistItem *b) {
                         return a->number < b->number;
                     });

    if (currentItemPtr) {
        currentIndex = indexOf (currentItemPtr);
        if (!timeString.isEmpty ()) {
            bool ok;
            int intTime = timeString.toInt (&ok);
            if (ok) {
                currentItemPtr->timeStamp = intTime;
            }
        }
    }

    if (currentIndex < 0) currentIndex = 0;
    return true;
}

PlaylistItem::PlaylistItem(float number, const QString &link, const QString &name, PlaylistItem *parent, bool isLocal)
    : number(number), name(name), link(link), m_parent(parent), type(isLocal ? LOCAL : ONLINE)
{
    // qDebug() << number << name;
    if (number > -1) {
        int dp = number == floor(number) ? 0 : 1;
        fullName = QString::number(number, 'f', dp) + (name.isEmpty() ? "" : ". ") + name;
    } else {
        fullName = name.isEmpty () ? "[Unnamed Episode]" : name;
    }

}

PlaylistItem *PlaylistItem::fromUrl(const QUrl &pathUrl, PlaylistItem *parent) {

    if (!pathUrl.isLocalFile ()) {
        PlaylistItem *playlist = new PlaylistItem("Pasted Link", nullptr, "", parent);
        if (parent) parent->append (playlist);
        auto pathString = pathUrl.toString ();
        playlist->emplaceBack (-1, pathString, pathString, true);
        playlist->currentIndex = 0;
        return playlist;
    }

    // QFileInfo path = QFileInfo(pathUrl.toLocalFile ());
    // if (!path.exists ()) {
    //     qDebug() << "Log (PlaylistItem): Path" << path << "doesn't exist";
    //     return nullptr;
    // }
    // QDir playlistDir = path.isDir () ? QDir(pathUrl.toLocalFile ()) : path.dir ();

    PlaylistItem *playlist = new PlaylistItem("", nullptr, "", parent);
    playlist->m_isLoadedFromFolder = true;
    playlist->m_children = std::make_unique<QList<PlaylistItem*>>();

    if (!playlist->loadFromFolder (pathUrl)) {
        delete playlist;
        return nullptr;
    }

    return playlist;
}

void PlaylistItem::emplaceBack(float number, const QString &link, const QString &name, bool isLocal) {
    if (!m_children) m_children = std::make_unique<QList<PlaylistItem*>>();
    m_children->emplaceBack(new PlaylistItem(number, link, name, this, isLocal));
    m_children->last ()->useCount++;
}

QList<Video> PlaylistItem::loadLocalSource(int index) {
    if (!isValidIndex (index)) return {};
    auto playlistItem = m_children->at(currentIndex);
    if (playlistItem->type != LOCAL) return {};
    if (index != currentIndex){
        currentIndex = index;
        updateHistoryFile (0);
    }
    return QList<Video>{ Video(QUrl::fromLocalFile(playlistItem->link)) };
}

void PlaylistItem::clear() {
    if (m_children) {
        for (auto &playlist : *m_children) {
            if (--playlist->useCount == 0)
                delete playlist;
        }
        m_children->clear ();
    }
}

void PlaylistItem::removeAt(int index) {
    auto toRemove = at(index);
    if (toRemove) {
        toRemove->m_parent = nullptr;
        if (--toRemove->useCount == 0) {
            delete toRemove;
        }
        m_children->removeAt (index);
    }
}

void PlaylistItem::replace(int index, PlaylistItem *value) {
    auto toRemove = at(index);
    if (toRemove) {
        toRemove->m_parent = nullptr;
        if (--toRemove->useCount == 0) {
            delete toRemove;
        }
        m_children->replace (index, value);
        value->m_parent = this;
        value->useCount++;
    }
}

int PlaylistItem::indexOf(const QString &link) {
    for (int i = 0; i < m_children->size (); i++) {
        auto child = m_children->at (i);
        if (child->link == link) {
            return i;
        }
    }
    return -1;
}

void PlaylistItem::append(PlaylistItem *value) {
    if (!m_children) m_children = std::make_unique<QList<PlaylistItem*>>();
    value->useCount++;
    value->m_parent = this;
    m_children->push_back (value);
}

QString PlaylistItem::getDisplayNameAt(int index) const {
    if (!isValidIndex(index))
        return "";
    auto currentItem = m_children->at(index);
    QString itemName = "%1\n[%2/%3] %4";
    itemName = itemName.arg(name)
                   .arg(index + 1)
                   .arg(m_children->count())
                   .arg(currentItem->fullName);
    return itemName;
}

void PlaylistItem::updateHistoryFile(qint64 time) {
    if (!m_isLoadedFromFolder) return;
    static QMutex mutex;
    mutex.lock ();
    if (m_historyFile->isOpen() || m_historyFile->open(QIODevice::WriteOnly)) {
        m_historyFile->resize(0);
        QTextStream stream(m_historyFile.get ());
        QString lastWatchedFilePath = m_children->at(currentIndex)->link;
        stream << lastWatchedFilePath.split("/").last();
        if (time > 0) {
            stream << ":" << QString::number (time);
        }
        m_historyFile->close();
    }
    mutex.unlock ();
}

void PlaylistItem::setLastPlayAt(int index, int time) {
    if (!isValidIndex (index)) return;
    qDebug() << "Setting playlist last play info at" << index << time;
    currentIndex = index;
    m_children->at (index)->timeStamp = time;
}

bool PlaylistItem::isValidIndex(int index) const {
    if (!m_children || m_children->isEmpty ()) return false;
    return index >= 0 && index < m_children->size();
}
