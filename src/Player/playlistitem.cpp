#include "playlistitem.h"

PlaylistItem::PlaylistItem(const QString& name, const QString& provider, std::string link, PlaylistItem* parent)
    : name(name), provider(provider), link(std::move(link)), m_parent(parent), type(LIST) {}

PlaylistItem::PlaylistItem(int number, const std::string &link, const QString &name, PlaylistItem *parent, bool isLocal)
    : number(number), name(name), link(link), m_parent(parent), type(isLocal ? LOCAL : ONLINE), fullName(name.isEmpty() ? "Unnamed Episode" : "")
{
    if (number > 0) {
        fullName = QString::number(number) + (name.isEmpty() ? "" : ". ") + name;
    }
    fullName = fullName.isEmpty () ? "[Unnamed Episode]" : fullName;
}

PlaylistItem *PlaylistItem::fromLocalDir(const QString &path) {
    QDir localDir(QUrl::fromLocalFile(path).toLocalFile());
    qDebug() << localDir.absolutePath() << localDir.exists();
    if (!localDir.exists()) return nullptr;

    QStringList fileNames =
        localDir.entryList({"*.mp4", "*.mkv", "*.avi", "*.mp3", "*.flac", "*.wav",
                            "*.ogg", "*.webm"},
                           QDir::Files);
    if (fileNames.isEmpty()) return nullptr;

    PlaylistItem *playlist = new PlaylistItem(localDir.dirName(), "", localDir.absolutePath().toStdString());
    playlist->m_children = std::make_unique<QList<PlaylistItem*>>();

    static QRegularExpression fileNameRegex{ R"((?:Episode\s*)?(?<number>\d+)\s*[\.:]?\s*(?<title>.*)?\.\w{3})" };

    for (int i = 0; i < fileNames.count(); i++) {
        QRegularExpressionMatch match = fileNameRegex.match(fileNames[i]);
        QString title = match.hasMatch() ? match.captured("title").trimmed() : "";
        int itemNumber = match.hasMatch() ? !match.captured("number").isEmpty() ? match.captured("number").toInt() : i : i;

        auto childItem = new PlaylistItem(itemNumber, localDir.absoluteFilePath(fileNames[i]).toStdString(), title, playlist, true);
        playlist->m_children->push_back(childItem);
    }

    // sort the episodes in order
    std::sort(playlist->m_children->begin(), playlist->m_children->end(),
              [](const PlaylistItem *a, const PlaylistItem *b) {
                  return a->number < b->number;
              });

    // check if there is a watch history file
    playlist->m_historyFile = std::make_unique<QFile> (localDir.filePath(".mpv.history"));
    //    playlist->m_fileCloseTimer = new QTimer();
    //    playlist->m_fileCloseTimer->setSingleShot (true);
    //    playlist->m_fileCloseTimer->setInterval (60000);
    //    QObject::connect (playlist->m_fileCloseTimer, &QTimer::timeout,
    //    playlist->m_historyFile, &QFile::close);

    if (localDir.exists(".mpv.history")) {
        if (playlist->m_historyFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString lastWatchedText = QTextStream(playlist->m_historyFile.get ()).readAll().trimmed();
            playlist->m_historyFile->close();
            if (!lastWatchedText.isEmpty()) {
                for (int i = 0; i < playlist->m_children->size(); i++) {
                    if (QString::fromStdString(playlist->m_children->at(i)->link).split("/").last() == lastWatchedText) {
                        playlist->currentIndex = i;
                        break;
                    }
                }
            }
        } else {
            qDebug() << "Failed to open history file";
            delete playlist;
            return nullptr;
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
        }
    }
    return QUrl::fromLocalFile(
        QString::fromStdString(m_children->at(index)->link));
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
