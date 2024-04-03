#include "playlistitem.h"
#include <cmath>

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
        auto pathString = pathUrl.toString ();
        playlist->emplaceBack (-1, pathString, pathString, true);
        playlist->currentIndex = 0;
        return playlist;
    }
    QFileInfo path = QFileInfo(pathUrl.toLocalFile ());
    if (!path.exists ()) {
        qDebug() << "Log (PlaylistItem): Path" << path << "doesn't exist";
        return nullptr;
    }

    QDir playlistDir = path.isDir () ? QDir(pathUrl.toLocalFile ()) : path.dir ();
    QStringList fileNames = playlistDir.entryList(
        {"*.mp4", "*.mkv", "*.avi", "*.mp3", "*.flac", "*.wav", "*.ogg", "*.webm"}, QDir::Files);

    if (fileNames.isEmpty()) {
        qDebug() << "Log (PlaylistItem): No files to play in" << playlistDir.absolutePath ();
        return nullptr;
    }


    PlaylistItem *playlist = new PlaylistItem(playlistDir.dirName(), nullptr, playlistDir.absolutePath(), nullptr); //TODO no parent
    playlist->m_children = std::make_unique<QList<PlaylistItem*>>();

    static QRegularExpression fileNameRegex{ R"((?:Episode\s*)?(?<number>\d+)\s*[\.:]?\s*(?<title>.*)?\.\w{3})" };

    for (int i = 0; i < fileNames.count(); i++)
    {
        QRegularExpressionMatch match = fileNameRegex.match(fileNames[i]);
        QString title = match.hasMatch() ? match.captured("title").trimmed() : "";
        int itemNumber = match.hasMatch() ? !match.captured("number").isEmpty() ? match.captured("number").toInt() : i : i;

        auto childItem = new PlaylistItem(itemNumber, playlistDir.absoluteFilePath(fileNames[i]), title, playlist, true);
        playlist->m_children->push_back(childItem);
    }

    // sort the episodes in order
    std::sort(playlist->m_children->begin(), playlist->m_children->end(),
              [](const PlaylistItem *a, const PlaylistItem *b) {
                  return a->number < b->number;
              });

    // Open watch history file
    playlist->m_historyFile = std::make_unique<QFile> (playlistDir.filePath(".mpv.history"));
    // playlist->m_fileCloseTimer = new QTimer(playlist->m_historyFile.get ());
    // playlist->m_fileCloseTimer->setSingleShot (true);
    // playlist->m_fileCloseTimer->setInterval (10000);
    // QObject::connect (playlist->m_fileCloseTimer, &QTimer::timeout, playlist->m_historyFile.get (), &QFile::close);


    // qDebug() << playlist->m_historyFile.get ()->fileName ();
    if (playlistDir.exists(".mpv.history")) {
        //QString currentFilename = path.fileName (); // file to set the current index to
        auto openMode = (!path.isFile () ? QIODevice::WriteOnly : QIODevice::ReadOnly) | QIODevice::Text;
        if (!playlist->m_historyFile->open(openMode)) {
            qDebug() << "Log (PlaylistItem): Failed to open history file";
            delete playlist;
            return nullptr;
        }

        if (!path.isFile()) {
            // Create new history file and write path of opened file
            playlist->m_historyFile->write (path.fileName ().toUtf8 ());
        } else {
            // Find the index of the last played file
            auto fileData = QTextStream(playlist->m_historyFile.get ()).readAll().trimmed().split (" ");;

            if (QString currentFilename = fileData.first (); !currentFilename.isEmpty()) {
                for (int i = 0; i < playlist->m_children->size(); i++) {
                    if (playlist->m_children->at(i)->link.split("/").last() == currentFilename) {
                        playlist->currentIndex = i;
                        // Update the last play time
                        if (fileData.size () == 2){
                            QString timeString = fileData.last ();
                            bool ok;
                            int intTime = timeString.toInt (&ok);
                            if (ok){
                                playlist->at (i)->lastPlayTime = intTime;
                            }
                            // qDebug() << "Updating local last play" << i << timeString << ok << intTime;
                        }
                        break;
                    }
                }
            }
        }
        playlist->m_historyFile->close();
    }


    if (playlist->currentIndex < 0)
        playlist->currentIndex = 0;
    return playlist;
}

void PlaylistItem::emplaceBack(float number, const QString &link, const QString &name, bool isLocal) {
    if (!m_children) m_children = std::make_unique<QList<PlaylistItem*>>();
    m_children->emplaceBack(new PlaylistItem(number, link, name, this, isLocal));
}

QUrl PlaylistItem::loadLocalSource(int index) {
    if (!isValidIndex (index)) return QUrl();

    auto playlistItem = m_children->at(currentIndex);
    if (playlistItem->type != LOCAL) return QUrl();

    if (index != currentIndex)
        updateHistoryFile (0);

    currentIndex = index;
    return QUrl::fromLocalFile(playlistItem->link);
}

void PlaylistItem::clear() {
    if (m_children) {
        qDeleteAll(*m_children);
        m_children->clear();
    }
}

QString PlaylistItem::getDisplayName(int index) const {
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

bool PlaylistItem::isValidIndex(int index) const {
    if (!m_children) return false;
    return index >= 0 && index < m_children->size();
}
