#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include "parsers/showprovider.h"
#include <QDir>
#include <QAbstractListModel>
#include <QtConcurrent>
#include <QStandardItemModel>
#include <mpv/mpvObject.h>
#include <tools/ErrorHandler.h>
#include "showmanager.h"

class Playlist {

private:
    QFile* m_historyFile;
    QVector<Episode> list;
    QString loadOnlineSource(int index){
        //lazy show in watchlist
        if(sourceLink.ends_with (".m3u8")){
            return QString::fromStdString (sourceLink);
        }
        qDebug()<<"Fetching servers for episode" << list[index].number;
        qDebug()<<"Playlist index:" << index+1 << "/" << list.count ();
        QVector<VideoServer> servers = currentProvider->loadServers (list[index]);
        if(!servers.empty ()){
            qDebug()<<"Successfully fetched servers for" << list[index].number;
            QString source = currentProvider->extractSource(servers[0]);
            if(sourceLink == ShowManager::instance().getCurrentShow ().link){ //todo
                ShowManager::instance().setLastWatchedIndex (index);
            }
            this->m_currentIndex = index;
            //            emit currentIndexChanged();
            return source;
        }
        return "";
    }
    QString loadLocalSource(int index){
        if (m_historyFile->open(QIODevice::WriteOnly)) {
            QTextStream stream(m_historyFile);
            stream << QString::fromStdString (list[index].link).split ("/").last ();
            m_historyFile->close ();
        }
        this->m_currentIndex = index;
        return QString::fromStdString (list[index].link);
        //        emit currentIndexChanged();
    }

public:
    explicit Playlist(const QUrl &localFolderPath){
        QDir directory(localFolderPath.toLocalFile());
        directory.setFilter(QDir::Files);
        directory.setNameFilters({"*.mp4", "*.mp3", "*.mov"});
        QStringList fileNames = directory.entryList();
        //        if(fileNames.empty ())return;
        qDebug()<<fileNames;
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

        m_historyFile = new QFile(directory.filePath(".mpv.history"));
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
    Playlist(const ShowData& show,nlohmann::json* json){
        name = show.title;
        list = show.episodes;
        m_watchListShowItem = json;
        sourceLink = show.link;
        currentProvider = ShowManager::instance().getProvider (show.provider);
    }
    nlohmann::json* m_watchListShowItem { nullptr };
    QString name;
    std::string sourceLink { "" };
    bool online = true;
    int m_currentIndex = 0;
    std::shared_ptr<ShowProvider> currentProvider;
public:
    const Episode& get(int index) const
    {
        return list.at (index);
    }
    int size() const {
        return list.size ();
    }
    QString getItemName(int index) const
    {
        if(!isValidIndex(index)) return "";
        const Episode& currentItem = list.at(index);
        QString itemName = "%1\n[%2/%3] %4";
        itemName = itemName.arg (name).arg (index+1).arg (list.size ()).arg (currentItem.getFullTitle ());
        return itemName;
    }
    bool isValidIndex(int index) const
    {
        return index >= 0 && index < list.size ();
    }
    QString load(int index)
    {
        if(!isValidIndex(index))
            return "";

        try{
            if (online) {
                return loadOnlineSource (index);
            } else {
                return loadLocalSource(index);
            }
        }
        catch(...)
        {
            ErrorHandler::instance ().show (QString("Failed to load source for %1").arg(index));
        }
        return "";
    }
    int getIndex() const
    {
        return m_currentIndex;
    }
};

class PlaylistModel : public QStandardItemModel
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentItemName READ getCurrentItemName NOTIFY currentIndexChanged)
    Q_PROPERTY(QString showName READ getTitle NOTIFY showNameChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged);
    Q_PROPERTY(QString onLaunchFile READ getPlayOnLaunchFile CONSTANT);
    QString getTitle(){
        if (m_playlists.isEmpty ()) return "";
        return m_playlists.first ().name;
    }
    QString getCurrentItemName() const{
        if (m_playlists.isEmpty ()) return "";
        return m_playlists.first ().getItemName (m_currentIndex);
    }

public:
    explicit PlaylistModel(QObject *parent = nullptr) : QAbstractListModel(parent) {};
    ~PlaylistModel(){}
    // opens the file to play immediately when application launches
    void setLaunchFile(const QString& path);
    void setLaunchFolder(const QString& path);
    QString getPlayOnLaunchFile()
    {
        return m_onLaunchFile;
    }

    std::set<std::string> playlistSet;
    /*
     * hashset containing all playlist links which prevents the same playlist from
     * being added to the list of playlists
    */

    Q_INVOKABLE void loadFolder(const QUrl& path); // loading playlist from folder

    void appendPlaylist(const ShowData& show,nlohmann::json* json)
    {
        if(playlistSet.contains (show.link)) return; // prevents duplicate playlists from being added
        m_playlists.emplaceBack (Playlist(show,json));
        playlistSet.insert (show.link);
    }

    void appendPlaylist(const QUrl& path)
    {
        m_playlists.push_back (Playlist(path));
        emit showNameChanged();
        emit layoutChanged ();
    }

    void replaceCurrentPlaylist(const ShowData& show,nlohmann::json* json)
    {
        if( !m_playlists.isEmpty () &&
            show.link == m_playlists.first ().sourceLink &&
            show.title == m_playlists.first ().name)
        {
            return;
        }

        playlistSet.insert (playlistSet.begin (),show.link);
        if(!m_playlists.isEmpty ()) m_playlists.removeFirst ();
        m_playlists.insert (0,Playlist(show,json));
        emit layoutChanged ();
        emit showNameChanged();
    }

    Q_INVOKABLE void play(int index);

    //  Traversing the playlist
    Q_INVOKABLE void loadOffset(int offset);
    Q_INVOKABLE void playNextItem()
    {
        loadOffset (1);
    }
    Q_INVOKABLE void playPrecedingItem(){
        loadOffset(-1);
    }
private:
    inline int currentIndex() const
    {
        return m_currentIndex;
    }
    inline bool isLoading()
    {
        return loading;
    }
    void setLoading(bool b)
    {
        loading = b;
        emit loadingChanged ();
    }
private:
    int m_currentIndex = -1;
    QFutureWatcher<void> m_watcher;
    QString m_onLaunchFile;
    bool loading = false;
    QVector<Playlist> m_playlists;
signals:
    void loadingChanged(void);
    void sourceFetched();
    void currentIndexChanged(void);
    void updatedLastWatchedIndex();
    void showNameChanged(void);
    void encounteredError(QString error);
public slots:
    void syncList(const ShowData& show,nlohmann::json* json);
private:
    enum
    {
        TitleRole = Qt::UserRole,
        NumberRole,
        NumberTitleRole
    };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
};

class ServerListModel: public QAbstractListModel
{

public:
    ServerListModel() = default;
    ~ServerListModel() = default;
    void setServers(QVector<VideoServer> servers)
    {
        m_servers = servers;
        //testServers ();
    }
    void invalidateServer(int index)
    {
        //server is not working
        if (index > m_servers.size () || index < 0) return;
        m_servers[index].working = false;
    }

    void testServers()
    {
        QFuture<void> future = QtConcurrent::run ([this](){
            for (auto& server : m_servers)
            {
                // extract source first
                if(NetworkClient::get (server.source.toStdString ()).code != 200)
                {
                    server.working = false;
                };
            }
        });
    }

    QVector<VideoServer> servers() const
    {
        return m_servers;
    }

//    VideoServer getFirstWorkingServer() const
//    {
//        for (const auto& server : m_servers)
//        {
//            if(server.working){
//                return server;
//            }
//        }
//        return "";
//    }


private:
    enum{
        NameRole = Qt::UserRole,
    };

    QVector<VideoServer> m_servers {};
public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return m_servers.size();
    };
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid() || m_servers.isEmpty ())
            return QVariant();

        const VideoServer& server = m_servers.at (index.row());
        switch (role)
        {
        case NameRole:
            return server.name;
            break;
        default:
            break;
        }
        return QVariant();
    };
    QHash<int, QByteArray> roleNames() const override;
};

#endif // PLAYLISTMODEL_H
