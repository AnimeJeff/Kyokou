#ifndef DOWNLOADMODEL_H
#define DOWNLOADMODEL_H

#include <QAbstractListModel>
#include <QDir>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QString>
#include <QThread>
#include <QThreadPool>
#include <QCoreApplication>
#include "Explorer/showmanager.h"
#include <mutex>
#include <QMap>
//QString DownloadRunnable::N_m3u8DLPath = "";
//bool DownloadRunnable::N_m3u8DLExists = false;

struct DownloadTask
{
    QString videoName;
    QString folder;
    QString link;
    QString headers;
    QString displayName;
    QString path;
    bool success = false;
    int progressValue = 0;
    QString progressText = "";
    QFutureWatcher<bool>* watcher = nullptr;
    enum State
    {
        SUCCESS,
        FAILED,
        INVALID_URI
    };

    ~DownloadTask()
    {
        qDebug() << displayName << "deleted";
    }
};


static void download(QPromise<bool> &promise, const QString& N_m3u8DLPath, const QStringList &command);

class DownloadModel: public QAbstractListModel
{
    Q_OBJECT
    enum{
        NameRole = Qt::UserRole,
        PathRole,
        ProgressValueRole,
        ProgressTextRole
    };
    QString downloadDir;
    QVector<QFutureWatcher<bool>*> watchers;
    QVector<DownloadTask*> tasksQueue;
    QVector<DownloadTask*> tasks;
    QMap<QFutureWatcher<bool>*,DownloadTask*> watcherTaskTracker;
    QRecursiveMutex mutex;
    QString N_m3u8DLPath;
    QRegularExpression folderNameCleanerRegex = QRegularExpression("[/\\:*?\"<>|]");
public:
    explicit DownloadModel(QObject *parent = nullptr): QAbstractListModel(parent)
    {
        N_m3u8DLPath = QDir::cleanPath (QCoreApplication::applicationDirPath() + QDir::separator() + "N_m3u8DL-CLI_v3.0.2.exe");
        downloadDir = QDir::cleanPath (QCoreApplication::applicationDirPath() + QDir::separator() + "Downloads");
        constexpr int threadCount = 4 ;
//        pool.setMaxThreadCount(threadCount);
        for (int i = 0; i < threadCount; ++i)
        {
            auto watcher = new QFutureWatcher<bool>();
            watchers.push_back (watcher);
            QObject::connect (watcher, &QFutureWatcher<bool>::finished, this, [watcher, this](){
                //set task success
                bool success = watcher->future ().isValid () ? watcher->future().result() : false;
                //                if(success) removeTask(watcher);
                qDebug() << watcherTaskTracker[watcher]->displayName << "completed";
                removeTask(watcher);
                setTask (watcher);
                emit layoutChanged ();
            });
            QObject::connect (watcher, &QFutureWatcher<bool>::canceled, this, [watcher, this](){
                qDebug() << "watcher cancelled";
                removeTask(watcher);
                setTask (watcher);
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
    void removeTask(QFutureWatcher<bool> *watcher)
    {
        QMutexLocker locker(&mutex);
        DownloadTask *task = watcherTaskTracker[watcher];
        Q_ASSERT(task);
        watcherTaskTracker[watcher] = nullptr;
        tasks.removeOne (task);
        delete task;
    }

    void setTask(QFutureWatcher<bool>* watcher);

    static QRegularExpression percentRegex;

    Q_INVOKABLE void downloadCurrentShow(int startIndex, int count = 1);

    void setDownloadFolder(const QString& path)
    {
        downloadDir = path;
    }
    static void download(QPromise<bool> &promise, const QString& N_m3u8DLPath, const QStringList &command)
    {
        promise.setProgressRange (0, 100);
        QProcess process;
        process.setProgram (N_m3u8DLPath);
        process.setArguments (command);
        QRegularExpression re = DownloadModel::percentRegex;
        process.start();
        //        qDebug() << N_m3u8DLPath << process->program ();
        //        qDebug() << process->arguments ();
        //        qDebug() << process->state ();
        //        qDebug() << process->error ();

        int percent;
        while (process.state() == QProcess::Running
               && process.waitForReadyRead()
               && !promise.isCanceled ())
        {
            auto line = process.readAllStandardOutput().trimmed();
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch())
            {
                percent = match.captured().toDouble();
                promise.setProgressValueAndText (percent, line);
            }
            else if(line.contains("Invalid Uri"))
            {
                promise.addResult (false); //todo add reason
                return;
            }
        }
        qDebug() << "exited loop";

        promise.addResult (true);
    }

    void addTask(DownloadTask *task)
    {
        QMutexLocker locker(&mutex);
        tasksQueue.push_back (task);
        tasks.push_back (task);
    }
    void startTasks()
    {
        QMutexLocker locker(&mutex);
        for(auto* watcher:watchers)
        {
            if(tasksQueue.isEmpty ()) break;
            else if(!watcherTaskTracker[watcher]) setTask (watcher);
        }
    }

    void cancelAllTasks() {
        QMutexLocker locker(&mutex);
        qDeleteAll (tasks);
        for (auto* watcher : watchers)
        {
            if(!watcherTaskTracker[watcher]) continue;
            watcherTaskTracker[watcher] = nullptr;
            watcher->cancel ();
            watcher->waitForFinished ();
        }
    }

    ~DownloadModel()
    {
        cancelAllTasks();
        qDeleteAll(watchers);
    }

    void cancelTask(int index)
    {
        if (index >= 0 && index < tasks.size())
        {
            QMutexLocker locker(&mutex);
            DownloadTask* task = tasks[index];
            if(task->watcher)
            {
                task->watcher->cancel ();
                task->watcher->waitForFinished ();
                removeTask (task->watcher);
            }
            else
            {
                delete task;
            }

        }
    }
public:
    int rowCount(const QModelIndex &parent) const
    {
        return tasks.count ();
    };
    QVariant data(const QModelIndex &index, int role) const;;

    QHash<int, QByteArray> roleNames() const;;
};


#endif // DOWNLOADMODEL_H
