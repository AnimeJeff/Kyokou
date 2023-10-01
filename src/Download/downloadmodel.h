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

class DownloadRunnable : public QObject,public QRunnable {
    Q_OBJECT
    QProcess* process;
    static QString N_m3u8DLPath;
    static bool N_m3u8DLExists;
public:
    enum {
        INVALID_URI
    };
    QStringList m_command;
    explicit DownloadRunnable(const QString& videoName, const QString& folder, const QString& link , const QString& headers,QObject* parent = nullptr)
        : m_command(QStringList{link,"--workDir",folder,"--saveName", videoName, "--enableDelAfterDone", "--disableDateInfo"}),QObject(parent)
    {
        setAutoDelete (false);
    }
    ~DownloadRunnable(){
        cancel();
        qDebug()<<"runnable deleted";
    }
    void cancel() {
        m_cancelled = true;
        if(process->state() == QProcess::Running){
            process->terminate ();
        }
        process->deleteLater ();
    }
    void run() override {
        process = new QProcess();
        process->setProgram (N_m3u8DLPath);
        process->setArguments (m_command);

        static QRegularExpression re(R"(\d+\.\d+(?=\%))");
        process->start();

        //        qDebug() << N_m3u8DLPath << process->program ();
        //        qDebug() << process->arguments ();
        //        qDebug() << process->state ();
        //        qDebug() << process->error ();

        while (process->state() == QProcess::Running
               && process->waitForReadyRead()
               && !m_cancelled)
        {
            auto line = process->readAllStandardOutput().trimmed();
            qDebug() << line;
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch())
            {
                QString percentStr = match.captured();
                double percent = percentStr.toDouble();
                emit progress(percent);
            }
            else if (line.contains("Task Done"))
            {
                qDebug() << "Task Done";
                emit finished();
                break;
            }
            else if(line.contains("Invalid Uri"))
            {
                emit error(INVALID_URI);
            }
        }
        process->deleteLater ();
    }

    static void setN_m3u8DLPath(const QString& path){
        N_m3u8DLPath = path;
        QDir dir(N_m3u8DLPath);
        N_m3u8DLExists = dir.exists ();
    }

signals:
    void progress(int value);
    void finished();
    void error(int reason);
private:
    bool m_cancelled = false;
};

class DownloadTask : public QObject {
    Q_OBJECT
    DownloadRunnable runnable;
    bool isRunning = false;
public:
    explicit DownloadTask(const QString& videoName, const QString& folder, const QString& link, const QString& headers,QObject* parent = nullptr)
        : QObject(parent), runnable(DownloadRunnable(videoName, folder, link, headers,parent))
    {
        QObject::connect (&runnable, &DownloadRunnable::progress,this,[this](int progress){
            this->progress = progress;
            emit progressChanged(progress);
        });
        downloadPath = QDir::cleanPath(folder + QDir::separator() + videoName);
    }
    ~DownloadTask(){
        cancel();
    }
    int progress;
    QString downloadPath;
    void start(QThreadPool& pool){
        QObject::connect(&runnable, &DownloadRunnable::finished, this, [this](){
            emit finished();
        });
        pool.start(&runnable);
    }

    void cancel(){
        runnable.cancel ();
    }
signals:
    void finished(void);
    void progressChanged(int progress);
};

class DownloadModel: public QAbstractListModel
{
    Q_OBJECT
    enum{
        TitleRole = Qt::UserRole,
        PathRole,
        ProgressRole
    };
    static QString downloadFolderPath;
public:
    explicit DownloadModel(QObject *parent = nullptr): QAbstractListModel(parent){
        pool.setMaxThreadCount(4);
    }
    static void setDownloadFolderPath(const QString& path){
        downloadFolderPath = path;
    }
    QThreadPool pool{this};

    QVector<DownloadTask*> tasks;

    Q_INVOKABLE void downloadM3u8(const QString& link,QString videoName,
                                  const QString& folder,const QString& referer)
    {
        videoName.replace(':', '.');
        QString headers = "authority:\"AUTHORITY\"|origin:\"https://REFERER\"|referer:\"https://REFERER/\"|user-agent:\"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.0.0 Safari/537.36\"sec-ch-ua:\"Not A;Brand\";v=\"99\", \"Chromium\";v=\"102\", \"Google Chrome\";v=\"102\"";
        headers.replace("REFERER", referer.isEmpty() ? link.split("https://")[1].split("/")[0] : referer);
        DownloadTask* task = new DownloadTask(videoName, folder, link, headers,this);
        tasks.push_back (task);
        connect(task, &DownloadTask::finished, task, [&](){
            tasks.removeOne (task);
            task->deleteLater ();
        });
        connect(task, &DownloadTask::progressChanged, this, [&](){
            emit layoutChanged();
        });
        task->start (pool);
    }

    Q_INVOKABLE void downloadCurrentShow(int startIndex,int endIndex = -1){
        QString folderName = ShowManager::instance ().getCurrentShow ().title;
        static auto folderNameCleanerRegex = QRegularExpression("[/\\:*?\"<>|]");
        folderName = folderName.replace(":",".").replace(folderNameCleanerRegex,"_");
//        auto folderPath = QDir::cleanPath (downloadFolderPath + QDir::separator () + folderName);
        const PlaylistItem* episodes = ShowManager::instance ().getCurrentShow ().getPlaylist ();
        const ShowProvider* provider = ShowManager::instance().getCurrentShowProvider();
        if(endIndex < 0)endIndex = startIndex;
        for(int i = startIndex;i <= endIndex;i++) {
            auto servers = provider->loadServers (episodes->at (i));
            auto link = provider->extractSource (servers.first ());
            downloadM3u8(link, folderName, downloadFolderPath,"");
        }
    }

    void cancelAllTasks() {
        for (auto& task : tasks) {
            task->cancel();
        }
        tasks.clear();
        pool.clear ();
    }

    ~DownloadModel(){
        cancelAllTasks();
    }

    void cancelTask(int index) {
        if (index >= 0 && index < tasks.size()) {
            DownloadTask* task = tasks[index];
            task->cancel();
            tasks.remove(index);
            task->deleteLater();
        }
    }
public:
    int rowCount(const QModelIndex &parent) const{
        return tasks.size ();
    };
    QVariant data(const QModelIndex &index, int role) const{
        if (!index.isValid())
            return QVariant();
        int i = index.row();

        const DownloadTask* task = tasks.at (i);

        switch (role) {
        case TitleRole:
            return task->progress;
            break;
        case ProgressRole:
            return task->progress;
            break;
        default:
            return QVariant();
        }
    };

    QHash<int, QByteArray> roleNames() const{
        QHash<int, QByteArray> names;
        names[TitleRole] = "title";
        names[ProgressRole] = "progress";
        return names;
    };
};


#endif // DOWNLOADMODEL_H
