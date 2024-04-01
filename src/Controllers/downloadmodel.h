#ifndef DOWNLOADMODEL_H
#define DOWNLOADMODEL_H

#include "Providers/showprovider.h"
#include <QAbstractListModel>
#include <QDir>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QString>
#include <QCoreApplication>
#include <QFutureWatcher>

#include <QMap>

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
            qDebug() << displayName << "task deleted";
        }
    };

    enum {
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
    void removeTask(DownloadTask *task);

    void setTask(QFutureWatcher<bool>* watcher);

    inline static QRegularExpression percentRegex = QRegularExpression(R"(\d+\.\d+(?=\%))");


    Q_INVOKABLE void downloadLink(const QString &name, const QString &link) {
        if (link.isEmpty ()) {
            qDebug() << "Log (Downloader): Empty link!";
            return;
        }
        if (name.isEmpty ()){
            qDebug() << "Log (Downloader): No filename provided!";
            return;
        }
        QString headers = "authority:\"AUTHORITY\"|origin:\"https://REFERER\"|referer:\"https://REFERER/\"|user-agent:\"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Safari/537.36\"sec-ch-ua:\"Not A;Brand\";v=\"99\", \"Chromium\";v=\"102\", \"Google Chrome\";v=\"102\"";
        DownloadTask *task = new DownloadTask(name, m_workDir, link, headers, name , link);
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
        promise.addResult (true);
    }

    void downloadShow(ShowData &show, int startIndex, int count);

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
            else if (!watcherTaskTracker[watcher]) setTask (watcher); //if watcher not working on a task
        }
    }

    void cancelAllTasks(){
        QMutexLocker locker(&mutex);

        for (auto* watcher : watchers)
        {
            if (!watcherTaskTracker[watcher]) continue;
            watcherTaskTracker[watcher] = nullptr;
            watcher->cancel ();
            watcher->waitForFinished ();
        }
        qDeleteAll (tasks);
        tasks.clear();
        tasksQueue.clear();
    }

    ~DownloadModel()
    {
        cancelAllTasks();
        qDeleteAll(watchers);
    }

    Q_INVOKABLE void cancelTask(int index)
    {
        if (index >= 0 && index < tasks.size())
        {
            removeTask (tasks[index]);
            emit layoutChanged();
        }
    }

    bool setWorkDir(const QString& path);

public:
    int rowCount(const QModelIndex &parent) const { return tasks.count(); };
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;
signals:
    void workDirChanged(void);
};


#endif // DOWNLOADMODEL_H
