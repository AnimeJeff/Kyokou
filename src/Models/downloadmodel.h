#pragma once

#include <QAbstractListModel>
#include <QDir>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QString>
#include <QCoreApplication>
#include <QFutureWatcher>
#include <QMap>

class ShowData;

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

    void watchTask(QFutureWatcher<bool>* watcher);

    inline static QRegularExpression percentRegex = QRegularExpression(R"(\d+\.\d+(?=\%))");


    Q_INVOKABLE void downloadLink(const QString &name, const QString &link);


    void executeCommand(QPromise<bool> &promise, const QStringList &command);

    void downloadShow(ShowData &show, int startIndex, int count);

    void addTask(DownloadTask *task)
    {
        QMutexLocker locker(&mutex);
        tasksQueue.push_back (task);
        tasks.push_back (task);
    }

    void startTasks();

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



