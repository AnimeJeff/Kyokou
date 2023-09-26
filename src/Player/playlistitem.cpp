#include "playlistitem.h"



void PlaylistItem::loadFromLocalDir(const QUrl &localFolderPath, PlaylistItem *parent)
{
    QDir directory(localFolderPath.toLocalFile());
    directory.setFilter(QDir::Files);
    directory.setNameFilters({"*.mp4", "*.mp3", "*.mov"});
    QStringList fileNames = directory.entryList();
    name = directory.dirName ();
    m_children = new QVector<std::shared_ptr<PlaylistItem>>();
    type = LOCAL;

    QRegularExpression fileNameRegex{R"((?:Episode\s*)?(?<number>\d+)\s*[\.:]?\s*(?<title>.*)?\.\w{3})"};

    for (int i = 0 ; i < fileNames.count () ; i++) {
        QString fileName = fileNames[i];
        QRegularExpressionMatch match = fileNameRegex.match (fileName);
        QString title = "";
        int itemNumber = i;
        if (match.hasMatch())
        {
            if (!match.captured("title").isEmpty())
            {
                title = match.captured("title").trimmed ();
            }
            if(!match.captured("number").isEmpty())
            {
                itemNumber = match.captured("number").toInt ();
            }

        }
        //            PlaylistItem(itemNumber, directory.absoluteFilePath(fileName).toStdString (), title,parent)
        m_children->emplaceBack(std::make_shared<PlaylistItem>(itemNumber, directory.absoluteFilePath(fileName).toStdString (), title,parent));
    }

    // sort the episodes in order
    std::sort(m_children->begin(), m_children->end(),
              [](const std::shared_ptr<PlaylistItem> &a, const std::shared_ptr<PlaylistItem> &b)
              {
        return a->number < b->number;
              });

    // check if there is a watch history file
    m_historyFile = new QFile(directory.filePath(".mpv.history"));
    if (directory.exists(".mpv.history"))
    {
        if (m_historyFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString lastWatched = QTextStream(m_historyFile).readAll().trimmed();
            m_historyFile->close ();
            if(!lastWatched.isEmpty ()){
                for (int i = 0; i < m_children->size(); i++) {
                    if(QString::fromStdString (m_children->at(i)->link).split ("/").last () == lastWatched){ //todo split
                        currentIndex = i;
                        break;
                    }
                }
            }
        }
    }
}

bool PlaylistItem::isValidIndex(int index) const
{
    return index >= 0 && index < m_children->size ();
}
