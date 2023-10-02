#include "downloadmodel.h"

QRegularExpression DownloadModel::percentRegex = QRegularExpression(R"(\d+\.\d+(?=\%))");

void DownloadModel::setTask(QFutureWatcher<bool> *watcher)
{
    if(tasksQueue.isEmpty ())
        return;
    QMutexLocker locker(&mutex);
    DownloadTask *task = tasksQueue.front();
    task->watcher = watcher;
    watcherTaskTracker[watcher] = task;
    QStringList command {task->link,"--workDir", task->folder,"--saveName", task->videoName, "--enableDelAfterDone", "--disableDateInfo"};
    watcher->setFuture (QtConcurrent::run (&DownloadModel::download, N_m3u8DLPath, command));
    tasksQueue.pop_front ();
}

void DownloadModel::downloadCurrentShow(int startIndex, int count)
{
    if (count < 1) return;
    PlaylistItem* episodes = ShowManager::instance ().getCurrentShow ().playlist;
    if(!episodes) return;
    ++episodes->useCount;
    QString showName = ShowManager::instance ().getCurrentShow ().title;
    showName = showName.replace(":",".").replace(folderNameCleanerRegex,"_");   //todo check replace
    const ShowProvider* provider = ShowManager::instance().getCurrentShowProvider();
    if(startIndex + count > episodes->count ()) count = episodes->count () - startIndex;

    QFuture<void> future = QtConcurrent::run([this, showName, episodes, provider,count,startIndex](){
        try
        {
            for(int i = startIndex; i < startIndex + count; ++i)
            {
                auto workDir = QDir::cleanPath (downloadDir + QDir::separator () + showName);
                const PlaylistItem* episode = episodes->at (i);
                QVector<VideoServer> servers = provider->loadServers(episode);
                QString link = provider->extractSource (servers.first ());
                QString displayName = showName + " : " + episode->getFullName ();
                QString path = QDir::cleanPath (workDir + QDir::separator () + episode->getFullName () + ".mp4");
                QString referer = "";
                QString headers = "authority:\"AUTHORITY\"|origin:\"https://REFERER\"|referer:\"https://REFERER/\"|user-agent:\"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Safari/537.36\"sec-ch-ua:\"Not A;Brand\";v=\"99\", \"Chromium\";v=\"102\", \"Google Chrome\";v=\"102\"";
                headers.replace("REFERER", referer.isEmpty() ? link.split("https://")[1].split("/")[0] : referer);
                DownloadTask *task = new DownloadTask(episode->getFullName (), workDir, link, headers, displayName , path);
                addTask (task);
            }
        }catch(...)
        {
            ErrorHandler::instance ().show ("error adding download task");
            if(--episodes->useCount == 0)
            {
                qDebug() << "deleted playlist by downlaoder" ;
                delete episodes;
            }
            return;
        }

        //delete episode if no longer used by playlistmodel or current show
        qDebug() << "checking if should delete by downloader" << episodes->useCount;
        if(--episodes->useCount == 0)
        {
            qDebug() << "deleted playlist by downlaoder" ;
            delete episodes;
        }
        startTasks ();
        //        for(auto* watcher:watchers)
        //        {
        //            if(tasksQueue.isEmpty ()) break;
        //            else if(!watcherTaskTracker[watcher]) setTask (watcher);
        //        }
//        beginInsertRows(QModelIndex(), tasks.count(), tasks.count() + count);
//        endInsertRows();
        emit layoutChanged ();
    });
    //
}

QVariant DownloadModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();
    const DownloadTask* task = tasks.at (index.row());

    switch (role) {
    case NameRole:
        return task->displayName;
        break;
    case PathRole:
        return task->path;
        break;
    case ProgressValueRole:
        return task->progressValue;
        break;
    case ProgressTextRole:
        return task->progressText;
        break;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> DownloadModel::roleNames() const{
    QHash<int, QByteArray> names;
    names[NameRole] = "name";
    names[PathRole] = "path";
    names[ProgressValueRole] = "progressValue";
    names[ProgressTextRole] = "progressText";
    return names;
}
