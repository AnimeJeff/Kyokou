#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include "parsers/showparser.h"
#include <QDir>
#include <QAbstractListModel>
#include "global.h"
#include <QtConcurrent>




class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentItemName READ currentItemName NOTIFY currentIndexChanged)
    Q_PROPERTY(QString showName READ showName NOTIFY showNameChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged);
    inline bool isLoading(){
        return loading;
    }
    bool loading = false;
private:
    ShowParser *currentProvider;
    QString m_currentShowLink;
    QString m_currentShowName;
    int playlistIndex = -1;
    QFile* m_historyFile;
    QFutureWatcher<void> m_watcher;
    QVector<Episode> m_playlist;
    bool online = true;
    QString m_onLaunchFile;
    QString m_onLaunchPlaylist;
    std::shared_ptr<ShowResponse> m_watchListShowItem;
    QString currentItemName() const{
        if(playlistIndex<0 || playlistIndex>m_playlist.size ())return "";
        const Episode& currentItem = m_playlist[playlistIndex];
        QString itemName = "[%1/%2] %3";
        itemName = itemName.arg (playlistIndex+1).arg (m_playlist.size ()).arg (currentItem.number);
        if (currentItem.title.length () > 0) {
            itemName+=". "+ currentItem.title;
        }
        return itemName;
    }
    inline int currentIndex() const{
        return playlistIndex;
    }
    inline QString showName() const{
        return m_currentShowName;
    }

public:
    explicit PlaylistModel(QObject *parent = nullptr):QAbstractListModel(parent){
        connect(&m_watcher, &QFutureWatcher<void>::finished,this,[&]() {

        });
    };

    ~PlaylistModel(){
        delete m_historyFile;
    }

    void setOnLaunchFile(QString file){
        m_onLaunchFile = file;
    }
    QString getPlayOnLaunchFile(){
        return m_onLaunchFile;
    }

    void setOnLaunchPlaylist(QString playlist){
        loadFolder (QUrl::fromLocalFile(playlist),false);
        if(m_playlist.length () == 0){
            qWarning() << "directory has no playable items.";
            return;
        }
        m_onLaunchFile = m_playlist[this->playlistIndex].localPath;

    }

    inline QString getOnLaunchPlaylist(){
        return m_onLaunchPlaylist;
    }

    Q_INVOKABLE void loadFolder(const QUrl& path, bool play = true){
        QDir directory(path.toLocalFile()); // replace with the path to your folder

        directory.setFilter(QDir::Files);
        directory.setNameFilters({"*.mp4", "*.mp3", "*.mov"});
        QStringList fileNames = directory.entryList();

        if(fileNames.empty ())return;
        m_playlist.clear ();
        int loadIndex = 0;

        if(m_historyFile){
            delete m_historyFile;
            m_historyFile=nullptr;
        }

        QString lastWatched;
        m_historyFile = new QFile(directory.filePath(".mpv.history"));
        if (directory.exists(".mpv.history")) {
            if (m_historyFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
                lastWatched = QTextStream(m_historyFile).readAll().trimmed();
                m_historyFile->close ();
            }
        }

        static QRegularExpression fileNameRegex{R"((?:Episode )?(?<number>\d+)\.?(?<title>.*?)?\.\w{3})"};

        for (const QString& fileName:fileNames) {
            Episode playFile;
            QRegularExpressionMatch match = fileNameRegex.match (fileName);
            if (match.hasMatch()) {
                if (!match.captured("title").isEmpty()) {
                    playFile.title = match.captured("title").trimmed ();
                }
                playFile.number = match.captured("number").toInt ();
            }
            playFile.localPath = directory.absoluteFilePath(fileName);
            m_playlist.emplace_back(std::move(playFile));
        }
        std::sort(m_playlist.begin(), m_playlist.end(), [](const Episode &a, const Episode &b) {
            return a.number < b.number;
        });

        if(lastWatched.length () != 0){
            for (int i = 0; i < m_playlist.size(); i++) {
                if(m_playlist[i].localPath.split ("/").last () == lastWatched){
                    loadIndex = i;
                    break;
                }
            }
        }

        online=false;
        playlistIndex = loadIndex;
        m_currentShowName = directory.dirName ();
        emit showNameChanged();
        emit layoutChanged ();
        if(play){
            loadSource (loadIndex);
        }
    }


    Q_INVOKABLE void loadOffset(int offset);
    bool hasNextItem();
    bool hasPrecedingItem();
    void playNextItem();
    void playPrecedingItem();

    Q_INVOKABLE void loadSource(int index){
        loading = true;
        emit loadingChanged();
        this->playlistIndex = index;
        emit currentIndexChanged();
        if(online){
            if(m_watchListShowItem){
                m_watchListShowItem->setLastWatchedIndex (index);
                emit updatedLastWatchedIndex();
            }
            if(m_currentShowLink == Global::instance().currentShowObject ()->link ()){
                Global::instance().currentShowObject ()->setLastWatchedIndex (index);
            }

            m_watcher.setFuture (QtConcurrent::run([&]() {
                try{
                    QVector<VideoServer> servers = currentProvider->loadServers (m_playlist[playlistIndex]);
                    QString source = currentProvider->extractSource(servers[0]);
                    emit sourceFetched(source);
                }catch(const QException& e){
                    qCritical() << e.what ();
                    emit encounteredError("Encountered error while extracting" + QString(e.what ()));
                }
                loading = false;
                emit loadingChanged();
            }));

        }else{
            emit sourceFetched(m_playlist[playlistIndex].localPath);
            if (m_historyFile->open(QIODevice::WriteOnly)) {
                QTextStream stream(m_historyFile);
                stream<<m_playlist[index].localPath.split ("/").last ();
                m_historyFile->close ();
            }
            loading = false;
            emit loadingChanged();
        }
    }



signals:
    void loadingChanged(void);
    void sourceFetched(QString link);
    void currentIndexChanged(void);
    void updatedLastWatchedIndex();
    void showNameChanged(void);
    void encounteredError(QString error);
public slots:
    void syncList(std::shared_ptr<ShowResponse> watchListShowItem){
        online=true;
        currentProvider=Global::instance().getCurrentShowProvider();
        m_playlist = Global::instance ().currentShowObject ()->episodes();
        m_watchListShowItem = watchListShowItem;
        m_currentShowLink = Global::instance().currentShowObject ()->link ();
        m_currentShowName = Global::instance().currentShowObject ()->title ();
        emit layoutChanged ();
        emit showNameChanged();
    }
    inline void changeWatchListIndex(int from,int to){
        //        if(from==m_watchListIndex){
        //            m_watchListIndex=to;
        //        }
    }
private:
    enum{
        TitleRole = Qt::UserRole,
        NumberRole,
        NumberTitleRole
    };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

};

#endif // PLAYLISTMODEL_H
