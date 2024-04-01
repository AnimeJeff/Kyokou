#include "downloadmodel.h"
#include <QtConcurrent>
#include "Data/playlistitem.h"
#include "Data/showdata.h"



DownloadModel::DownloadModel(QObject *parent): QAbstractListModel(parent)
{
    N_m3u8DLPath = QDir::cleanPath (QCoreApplication::applicationDirPath() + QDir::separator() + "N_m3u8DL-CLI_v3.0.2.exe");
    //m_workDir = QDir::cleanPath (QCoreApplication::applicationDirPath() + QDir::separator() + "Downloads");
    m_workDir = QDir::cleanPath("D:\\TV\\Downloads");
    constexpr int threadCount = 4 ;
    //        pool.setMaxThreadCount(threadCount);
    for (int i = 0; i < threadCount; ++i)
    {
        auto watcher = new QFutureWatcher<bool>();
        watchers.push_back (watcher);

        QObject::connect (watcher, &QFutureWatcher<bool>::finished, this, [watcher, this](){
            //set task success
            if (!watcher->future().isValid()) {
                qDebug() << "Log (Downloader):" << watcherTaskTracker[watcher]->displayName << "cancelled successfully";
            } else {
                try {
                    auto res = watcher->future().result();
                } catch (...) {
                    qDebug() << "Log (Downloader):" << watcherTaskTracker[watcher]->displayName << "task failed";
                }
            }
            removeTask(watcherTaskTracker[watcher]);
            setTask (watcher);
            emit layoutChanged ();
        });

        QObject::connect (watcher, &QFutureWatcher<bool>::progressValueChanged, this, [watcher, this](){
            Q_ASSERT(watcherTaskTracker[watcher]);
            watcherTaskTracker[watcher]->progressValue = watcher->progressValue ();
            watcherTaskTracker[watcher]->progressText = watcher->progressText ();
            int i = tasks.indexOf (watcherTaskTracker[watcher]);
            emit dataChanged (index(i, 0),index(i, 0));
        });

    }
}

void DownloadModel::removeTask(DownloadTask *task)
{
    QMutexLocker locker(&mutex);
    if (task->watcher)
    {
        // task has started, not in task queue
        Q_ASSERT(task == watcherTaskTracker[task->watcher]);
        if (task->watcher->isRunning ()){
            qDebug() << "Log (Downloader): Attempting to cancel the ongoing task" << task->displayName;
            task->watcher->cancel ();
            task->watcher->waitForFinished ();
            return; // this function is called again from the canceled signal slot
        }
        watcherTaskTracker[task->watcher] = nullptr;
    }
    else {
        tasksQueue.removeOne (task);
    }
    qDebug() << "Log (Downloader): Removed task" << task->displayName;
    tasks.removeOne (task);
    delete task;

}

void DownloadModel::setTask(QFutureWatcher<bool> *watcher)
{
    QMutexLocker locker(&mutex);
    if (tasksQueue.isEmpty ())
        return;
    DownloadTask *task = tasksQueue.front();
    task->watcher = watcher;
    watcherTaskTracker[watcher] = task;
    QStringList command {task->link,"--workDir", task->folder,"--saveName", task->videoName, "--enableDelAfterDone", "--disableDateInfo"};
    watcher->setFuture (QtConcurrent::run (&DownloadModel::download, this, command));
    tasksQueue.pop_front ();
}

void DownloadModel::downloadShow(ShowData &show, int startIndex, int count)
{
    auto playlist = show.getPlaylist ();
    if (!playlist || count < 1) return;

    ++playlist->useCount; // prevents the playlist from being delete whilst

    int endIndex = startIndex + count;
    if (endIndex > playlist->count ()) endIndex = playlist->count ();

    QString showName = QString(show.title).replace(":",".").replace(folderNameCleanerRegex,"_");   //todo check replace
    qDebug() << "Log (Downloader)" << showName << "from index" << startIndex << "to" << endIndex - 1;
    auto provider = show.getProvider ();
    QFuture<void> future = QtConcurrent::run([this, showName, playlist, provider , startIndex, endIndex](){
        try
        {
            for (int i = startIndex; i < endIndex; ++i)
            {
                auto workDir = QDir::cleanPath (m_workDir + QDir::separator () + showName);
                const PlaylistItem* episode = playlist->at (i);
                QList<VideoServer> servers = provider->loadServers(episode);
                QString link;
                for (auto &server : servers)
                {
                    auto source = provider->extractSource (servers.first ());
                    if (!source.isEmpty ()) {
                        link = source.first ().videoUrl.toString ();
                        break;
                    }
                }

                QString displayName = showName + " : " + episode->getFullName ();
                QString path = QDir::cleanPath (workDir + QDir::separator () + episode->getFullName () + ".mp4");
                QString referer = "";
                QString headers = "authority:\"AUTHORITY\"|origin:\"https://REFERER\"|referer:\"https://REFERER/\"|user-agent:\"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Safari/537.36\"sec-ch-ua:\"Not A;Brand\";v=\"99\", \"Chromium\";v=\"102\", \"Google Chrome\";v=\"102\"";
                headers.replace("REFERER", referer.isEmpty() ? link.split("https://")[1].split("/")[0] : referer);
                DownloadTask *task = new DownloadTask(episode->getFullName (), workDir, link, headers, displayName , path);
                qDebug() << "Log (Downloader): Appending new download task for" << episode->getFullName ();
                addTask (task);
                if (i == startIndex) startTasks ();
            }
            startTasks ();
            emit layoutChanged ();
        }catch(...)
        {
            ErrorHandler::instance ().show ("error adding download task");
        }
        //delete episode if no longer used by playlistmodel or current show
        qDebug() << "Log (Downloader): Checking if the playlist should be deleted by the downloader" << playlist->useCount -1;
        if (--playlist->useCount == 0) {
            qDebug() << "Log (Downloader): Deleted playlist by downlaoder" ;
            delete playlist;
        }
    });

}



bool DownloadModel::setWorkDir(const QString &path)
{
    const QFileInfo outputDir(path);
    if ((!outputDir.exists()) || (!outputDir.isDir()) || (!outputDir.isWritable())) {
        qWarning() << "Log (Downloader): Output directory either doesn't exist or isn't a directory or writeable"
                   << outputDir.absoluteFilePath();
        return false;
    }
    m_workDir = path;

    emit workDirChanged ();
    return true;
}

QVariant DownloadModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();
    DownloadTask* task = tasks.at (index.row());

    switch (role){
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
