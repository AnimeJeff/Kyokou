#include "playlistitem.h"


PlaylistItem::PlaylistItem(int number, const std::string &link, const QString &name, PlaylistItem *parent, bool isLocal)
    : number(number), name(name), link(link), m_parent(parent), type(isLocal ? LOCAL : ONLINE), fullName(name.isEmpty() ? "Unnamed Episode" : "")
{
    // qDebug() << number << name;
    if (number > -1) {
        fullName = QString::number(number) + (name.isEmpty() ? "" : ". ") + name;
    } else {
        fullName = name.isEmpty () ? "[Unnamed Episode]" : name;
    }

}

PlaylistItem *PlaylistItem::fromLocalDir(const QString &pathString) {

    QFileInfo path = QFileInfo(pathString);
    if (!path.exists ()) {
        qDebug() << "Log (PlaylistItem): Path" << path << "doesn't exist";
        return nullptr;
    }

    QDir playlistDir = path.isDir () ? QDir(pathString) : path.dir ();
    QStringList fileNames = playlistDir.entryList(
        {"*.mp4", "*.mkv", "*.avi", "*.mp3", "*.flac", "*.wav", "*.ogg", "*.webm"}, QDir::Files);

    if (fileNames.isEmpty()) {
        qDebug() << "Log (PlaylistItem): No files to play in" << playlistDir.absolutePath ();
        return nullptr;
    }
    //todo check
    PlaylistItem *playlist = new PlaylistItem(playlistDir.dirName(), nullptr, playlistDir.absolutePath().toStdString(), nullptr); //TODO no parent
    playlist->m_children = std::make_unique<QList<PlaylistItem*>>();

    static QRegularExpression fileNameRegex{ R"((?:Episode\s*)?(?<number>\d+)\s*[\.:]?\s*(?<title>.*)?\.\w{3})" };

    for (int i = 0; i < fileNames.count(); i++)
    {
        QRegularExpressionMatch match = fileNameRegex.match(fileNames[i]);
        QString title = match.hasMatch() ? match.captured("title").trimmed() : "";
        int itemNumber = match.hasMatch() ? !match.captured("number").isEmpty() ? match.captured("number").toInt() : i : i;

        auto childItem = new PlaylistItem(itemNumber, playlistDir.absoluteFilePath(fileNames[i]).toStdString(), title, playlist, true);
        playlist->m_children->push_back(childItem);
    }

    // sort the episodes in order
    std::sort(playlist->m_children->begin(), playlist->m_children->end(),
              [](const PlaylistItem *a, const PlaylistItem *b) {
                  return a->number < b->number;
              });

    // check if there is a watch history file
    playlist->m_historyFile = std::make_unique<QFile> (playlistDir.filePath(".mpv.history"));
    //    playlist->m_fileCloseTimer = new QTimer();
    //    playlist->m_fileCloseTimer->setSingleShot (true);
    //    playlist->m_fileCloseTimer->setInterval (60000);
    //    QObject::connect (playlist->m_fileCloseTimer, &QTimer::timeout,
    //    playlist->m_historyFile, &QFile::close);


    QString currentFilename = path.fileName (); // file to set the current index to

    if (playlistDir.exists(".mpv.history"))
    {
        auto openMode = (path.isFile () ? QIODevice::WriteOnly : QIODevice::ReadOnly) | QIODevice::Text;
        if (playlist->m_historyFile->open(openMode))
        {
            if (path.isFile())
            {
                // change the history to the opened file
                playlist->m_historyFile->write (path.fileName ().toUtf8 ());
            } else {
                currentFilename = QTextStream(playlist->m_historyFile.get ()).readAll().trimmed();
                if (!currentFilename.isEmpty()) {
                    for (int i = 0; i < playlist->m_children->size(); i++) {
                        if (QString::fromStdString(playlist->m_children->at(i)->link).split("/").last() == currentFilename) {
                            playlist->currentIndex = i;
                            break;
                        }
                    }
                }
            }
            playlist->m_historyFile->close();
        } else {
            qDebug() << "Log (PlaylistItem): Failed to open history file";
            delete playlist;
            return nullptr;
        }
    }


    for (int i = 0; i < playlist->m_children->size(); i++) {
        if (QString::fromStdString(playlist->m_children->at(i)->link).split("/").last() == currentFilename) {
            playlist->currentIndex = i;
            break;
        }
    }

    if (playlist->currentIndex < 0)
        playlist->currentIndex = 0;
    return playlist;
}

void PlaylistItem::emplaceBack(int number, const std::string &link, const QString &name, bool isLocal) {
    if (!m_children) m_children = std::make_unique<QList<PlaylistItem*>>();
    m_children->emplaceBack(new PlaylistItem(number, link, name, this, isLocal));
}



QUrl PlaylistItem::loadLocalSource(int index) {
    if (index < 0 || index > m_children->count())
        throw std::runtime_error("loading local source for an invalid index");
    static QMutex mutex;
    mutex.lock ();
    if (currentIndex != index) {
        Q_ASSERT(m_historyFile);
        if (m_historyFile->isOpen() || m_historyFile->open(QIODevice::WriteOnly)) {
            // m_fileCloseTimer->start ();
            m_historyFile->resize(0);
            QTextStream stream(m_historyFile.get ());

            stream << QString::fromStdString(m_children->at(index)->link)
                          .split("/")
                          .last();
            currentIndex = index;
            m_historyFile->close();
        }
    }
    mutex.unlock ();
    return QUrl::fromLocalFile(QString::fromStdString(m_children->at(index)->link));
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
    return index >= 0 && index < m_children->size();
}
