#include "playlistmodel.h"
#include "showmanager.h"

void PlaylistModel::setLaunchFolder(const QString& path){
    loadFolder (QUrl::fromLocalFile(path),false);
    if(m_currentShow.playlist.length () == 0){
        qWarning() << "Directory has no playable items.";
        return;
    }
    m_onLaunchFile = QString::fromStdString (m_currentShow.playlist[this->m_currentIndex].link);
}
void PlaylistModel::setLaunchFile(const QString& path){
    m_onLaunchFile = QUrl::fromLocalFile(path).toString ();
}
void PlaylistModel::loadFolder(const QUrl &path, bool play){
    QDir directory(path.toLocalFile()); // replace with the path to your folder
    directory.setFilter(QDir::Files);
    directory.setNameFilters({"*.mp4", "*.mp3", "*.mov"});
    QStringList fileNames = directory.entryList();
    if(fileNames.empty ())return;
    m_currentShow.playlist.clear ();
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

    static QRegularExpression fileNameRegex{R"((?:Episode\s*)?(?<number>\d+)\s*[\.:]?\s*(?<title>.*)?\.\w{3})"};
    //todo use std
    for (int i=0;i<fileNames.count ();i++) {
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
        m_currentShow.playlist.emplace_back(Episode(number,directory.absoluteFilePath(fileName).toStdString (),title));
    }
    std::sort(m_currentShow.playlist.begin(), m_currentShow.playlist.end(), [](const Episode &a, const Episode &b) {
        return a.number < b.number;
    });

    if(lastWatched.length () != 0){
        for (int i = 0; i < m_currentShow.playlist.size(); i++) {
            if(QString::fromStdString (m_currentShow.playlist[i].link).split ("/").last () == lastWatched){ //todo split
                loadIndex = i;
                break;
            }
        }
    }

    online=false;
    m_currentIndex = loadIndex;
    m_currentShow.title = directory.dirName ();
    emit showNameChanged();
    emit layoutChanged ();
    if(play){
        loadSource (loadIndex);
    }
}

void PlaylistModel::loadSource(int index){
    if(!m_currentShow.isValidIndex(index))return;
    setLoading(true);

    if(online)
    {
        m_watcher.setFuture (QtConcurrent::run([index,this]() {
            try{
                loadOnlineSource (index);
                setLoading(false);
            }catch(...){
                //                qCritical() << e.what ();
                //                emit encounteredError("Encountered error while extracting" + QString(e.what ()));
                std::exception_ptr p = std::current_exception();
                std::clog <<(p ? p.__cxa_exception_type()->name() : "null") << std::endl;
                setLoading(false);
            }
        }));
    }else{
        loadLocalSource(index);

    }
}

void PlaylistModel::loadOnlineSource(int index){
    //lazy show in watchlist
    if(m_currentShow.link.startsWith ("http")){
        emit sourceFetched(m_currentShow.link);
        return;
    }
    qDebug()<<"Fetching servers for episode" << m_currentShow.playlist[index].number;
    qDebug()<<"Playlist index:" << index+1 << "/" <<m_currentShow.playlist.count ();
    QVector<VideoServer> servers = currentProvider->loadServers (m_currentShow.playlist[index]);
    if(!servers.empty ()){
        qDebug()<<"Successfully fetched servers for" << m_currentShow.playlist[index].number;
        QString source = currentProvider->extractSource(servers[0]);
        emit sourceFetched(source);
        if(m_watchListShowItem){
            m_watchListShowItem->at("lastWatchedIndex")= index;
            emit updatedLastWatchedIndex();
        }
        if(m_currentShow.link == QString::fromStdString (ShowManager::instance().getCurrentShow ().link)){ //todo
            ShowManager::instance().setLastWatchedIndex (index);
        }
        this->m_currentIndex = index;
        emit currentIndexChanged();
    }

}

void PlaylistModel::loadLocalSource(int index){
    emit sourceFetched(QString::fromStdString (m_currentShow.playlist[index].link));
    if (m_historyFile->open(QIODevice::WriteOnly)) {
        QTextStream stream(m_historyFile);
        stream << QString::fromStdString (m_currentShow.playlist[index].link).split ("/").last ();
        m_historyFile->close ();
    }
    this->m_currentIndex = index;
    emit currentIndexChanged();
}

void PlaylistModel::syncList(){
    auto currentShow = ShowManager::instance().getCurrentShow ();
    if(currentShow.link == m_currentShow.link.toStdString ()
        && currentShow.title == m_currentShow.title)return;
    online=true;
    currentProvider = ShowManager::instance().getCurrentShowProvider ();
    m_currentShow.title = currentShow.title;
    m_currentShow.link = QString::fromStdString (currentShow.link);
    m_currentShow.playlist = currentShow.episodes;
    emit layoutChanged ();
    emit showNameChanged();
}

void PlaylistModel::loadOffset(int offset){
    auto newIndex = m_currentIndex+offset;
    if(m_currentShow.isValidIndex(newIndex)){
        qDebug()<<"Loading offset "<<newIndex;
        loadSource(newIndex);
    }
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_currentShow.playlist.count ();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const Episode& episode = m_currentShow.playlist[index.row()];
    switch (role) {
    case TitleRole:
        return episode.title;
        break;
    case NumberRole:
        return episode.number;
        break;
    case NumberTitleRole:{
        QString lastWatchedEpisodeName = QString::number (episode.number);
        if(!(episode.title.isEmpty () || episode.title.toInt () == episode.number)){
            lastWatchedEpisodeName += "\n" + episode.title;
        }
        return lastWatchedEpisodeName;
        break;
    }
    default:
        break;
    }
    return QVariant();
}

QHash<int, QByteArray> PlaylistModel::roleNames() const{
    QHash<int, QByteArray> names;
    names[TitleRole] = "title";
    names[NumberRole] = "number";
    names[NumberTitleRole] = "numberTitle";
    return names;
}
