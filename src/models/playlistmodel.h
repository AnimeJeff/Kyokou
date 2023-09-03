#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include "parsers/showprovider.h"
#include <QDir>
#include <QAbstractListModel>
#include <QtConcurrent>

class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentItemName READ getCurrentItemName NOTIFY currentIndexChanged)
    Q_PROPERTY(QString showName READ getTitle NOTIFY showNameChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged);
    Q_PROPERTY(QString onLaunchFile READ getPlayOnLaunchFile CONSTANT);

    struct PlaylistShow{
        QString title;
        QString link;
        QVector<Episode> playlist;
        bool isValidIndex(int index) const{
            return index >= 0 && index < playlist.size ();
        }
    };

    QString getTitle(){
        return m_currentShow.title;
    }
    QString getCurrentItemName() const{
        if(!m_currentShow.isValidIndex(m_currentIndex)) return "";
        const Episode& currentItem = m_currentShow.playlist[m_currentIndex];
        QString itemName = "%4\n[%1/%2] %3";
        itemName = itemName.arg (m_currentIndex+1).arg (m_currentShow.playlist.size ()).arg (currentItem.number).arg (m_currentShow.title);
        if (currentItem.title.length () > 0) {
            itemName+=". "+ currentItem.title;
        }
        return itemName;
    }

public:
    explicit PlaylistModel(QObject *parent = nullptr)
        :QAbstractListModel(parent)
    {

    };
    ~PlaylistModel(){
        delete m_historyFile;
    }
    // opens the file to play immediately when application launches
    void setLaunchFile(const QString& path);
    void setLaunchFolder(const QString& path);
    QString getPlayOnLaunchFile(){
        return m_onLaunchFile;
    }

    //loading files
    Q_INVOKABLE void loadFolder(const QUrl& path, bool play = true);
    Q_INVOKABLE void loadSource(int index);
    void loadOnlineSource(int index);
    void loadLocalSource(int index);

    //  Traversing the playlist
    Q_INVOKABLE void loadOffset(int offset);
    Q_INVOKABLE void playNextItem(){ loadOffset (1); }
    Q_INVOKABLE void playPrecedingItem(){ loadOffset(-1); }

    void setWatchListShowItem(nlohmann::json* json){
        m_watchListShowItem = json;
    }
private:
    inline int currentIndex() const{
        return m_currentIndex;
    }
    inline QString showName() const{
        return m_currentShow.title;
    }
    inline bool isLoading(){
        return loading;
    }
    void setLoading(bool b){
        loading = b;
        emit loadingChanged ();
    }

private:
    ShowProvider *currentProvider;
    int m_currentIndex = -1;
    bool online = true;
    QFile* m_historyFile;
    QFutureWatcher<void> m_watcher;
    QString m_onLaunchFile;
    nlohmann::json* m_watchListShowItem;
    bool loading = false;
    PlaylistShow m_currentShow;
signals:
    void loadingChanged(void);
    void sourceFetched(QString link);
    void currentIndexChanged(void);
    void updatedLastWatchedIndex();
    void showNameChanged(void);
    void encounteredError(QString error);
public slots:
    void syncList();
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

class Playlist : QObject {
    Q_OBJECT
private:
    QFile* m_historyFile;

    Playlist(const QString& name, const QVector<Episode>& list,const QString& sourceLink,ShowProvider *currentProvider):
        name(name),list(list),sourceLink(sourceLink),currentProvider(currentProvider)
    {

    }
public:

    explicit Playlist(const QUrl &localFolderPath){
        QDir directory(localFolderPath.toLocalFile());
        directory.setFilter(QDir::Files);
        directory.setNameFilters({"*.mp4", "*.mp3", "*.mov"});
        QStringList fileNames = directory.entryList();
//        if(fileNames.empty ())return;
        int loadIndex = 0;
        QString lastWatched;

        static QRegularExpression fileNameRegex{R"((?:Episode\s*)?(?<number>\d+)\s*[\.:]?\s*(?<title>.*)?\.\w{3})"};

        //todo use std
        for (int i = 0 ; i < fileNames.count () ; i++) {
            QString fileName = fileNames[i];
            QRegularExpressionMatch match = fileNameRegex.match (fileName);
            QString title = "";
            int number = i;
            if (match.hasMatch())
            {
                if (!match.captured("title").isEmpty())
                {
                    title = match.captured("title").trimmed ();
                }
                number = match.captured("number").toInt ();
            }
            list.emplace_back(Episode(number,directory.absoluteFilePath(fileName).toStdString (),title));
        }
        std::sort(list.begin(), list.end(), [](const Episode &a, const Episode &b) {
            return a.number < b.number;
        });

        auto m_historyFile = new QFile(directory.filePath(".mpv.history"));
        if (directory.exists(".mpv.history")) {
            if (m_historyFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
                lastWatched = QTextStream(m_historyFile).readAll().trimmed();
                m_historyFile->close ();
            }
        }
        if(lastWatched.length () != 0){
            for (int i = 0; i < list.size(); i++) {
                if(QString::fromStdString (list[i].link).split ("/").last () == lastWatched){ //todo split
                    loadIndex = i;
                    break;
                }
            }
        }
        name=directory.dirName ();
        online = false;
        m_currentIndex = loadIndex;
    }


    QString name;
    QString sourceLink {""};
    QVector<Episode> list;
    bool online = true;
    int m_currentIndex = 0;
    bool isValidIndex(int index) const{
        return index >= 0 && index < list.size ();
    }
    ShowProvider *currentProvider;
    QString loadOnlineSource(int index){
        //lazy show in watchlist
        if(sourceLink.endsWith (".m3u8")){
            return sourceLink;
        }
        qDebug()<<"Fetching servers for episode" << list[index].number;
        qDebug()<<"Playlist index:" << index+1 << "/" << list.count ();
        QVector<VideoServer> servers = currentProvider->loadServers (list[index]);
        if(!servers.empty ()){
            qDebug()<<"Successfully fetched servers for" << list[index].number;
            QString source = currentProvider->extractSource(servers[0]);

//            if(m_watchListShowItem){
//                m_watchListShowItem->at("lastWatchedIndex")= index;
//                emit updatedLastWatchedIndex();
//            }
            if(sourceLink == QString::fromStdString (ShowManager::instance().getCurrentShow ().link)){ //todo
                ShowManager::instance().setLastWatchedIndex (index);
            }
            this->m_currentIndex = index;
//            emit currentIndexChanged();
            return source;
        }

    }

};

#endif // PLAYLISTMODEL_H
