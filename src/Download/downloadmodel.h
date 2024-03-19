#ifndef DOWNLOADMODEL_H
#define DOWNLOADMODEL_H

#include <QAbstractListModel>
#include <QDir>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QString>
#include <QCoreApplication>
#include <QFutureWatcher>

#include <QMap>
//QString DownloadRunnable::N_m3u8DLPath = "";
//bool DownloadRunnable::N_m3u8DLExists = false;



class DownloadModel: public QAbstractListModel
{
    Q_OBJECT
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

    enum{
        NameRole = Qt::UserRole,
        PathRole,
        ProgressValueRole,
        ProgressTextRole
    };

    Q_PROPERTY(QString workDir READ getWorkDir WRITE setWorkDir NOTIFY workDirChanged)
    QString m_workDir;
    QString N_m3u8DLPath;

    QList<QFutureWatcher<bool>*> watchers;
    QList<DownloadTask*> tasksQueue;
    QList<DownloadTask*> tasks;
    QMap<QFutureWatcher<bool>*,DownloadTask*> watcherTaskTracker;
    QRecursiveMutex mutex;
    QRegularExpression folderNameCleanerRegex = QRegularExpression("[/\\:*?\"<>|]");
public:
    QString getWorkDir(){
        return m_workDir;
    }
    explicit DownloadModel(QObject *parent = nullptr);
    void removeTask(QFutureWatcher<bool> *watcher);

    void setTask(QFutureWatcher<bool>* watcher);

    static QRegularExpression percentRegex;

    Q_INVOKABLE void downloadCurrentShow(int startIndex, int count = 1);

    Q_INVOKABLE void downloadLink(QString link) {
        QString headers = "authority:\"AUTHORITY\"|origin:\"https://REFERER\"|referer:\"https://REFERER/\"|user-agent:\"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Safari/537.36\"sec-ch-ua:\"Not A;Brand\";v=\"99\", \"Chromium\";v=\"102\", \"Google Chrome\";v=\"102\"";
        DownloadTask *task = new DownloadTask(link.mid (0,5), m_workDir, link, headers, link.mid (0,5) , link);
        addTask (task);
        startTasks ();
        emit layoutChanged ();
    }

    void download(QPromise<bool> &promise, const QStringList &command)
    {
        promise.setProgressRange (0, 100);
        QProcess process;
        process.setProgram (N_m3u8DLPath);
        process.setArguments (command);
        QRegularExpression re = DownloadModel::percentRegex;
        process.start();
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
            else if (line.contains("Invalid Uri"))
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
        for (auto* watcher:watchers)
        {
            if (tasksQueue.isEmpty ()) break;
            else if (!watcherTaskTracker[watcher]) setTask (watcher);
        }
    }

    void cancelAllTasks(){
        QMutexLocker locker(&mutex);
        qDeleteAll (tasks);
        for (auto* watcher : watchers)
        {
            if (!watcherTaskTracker[watcher]) continue;
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
            if (task->watcher)
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

    Q_INVOKABLE void openFolder(const QString& path)
    {
        QProcess::startDetached("explorer.exe", QStringList() << path);
    }



    bool setWorkDir(const QString& path);

public:
    int rowCount(const QModelIndex &parent) const
    {
        return tasks.count ();
    };
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;
signals:
    void workDirChanged(void);
};


#endif // DOWNLOADMODEL_H
