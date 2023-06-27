#include "playlistmodel.h"
#include "application.h"

void PlaylistModel::loadFolder(const QUrl &path, bool play){
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

    static QRegularExpression fileNameRegex{R"((?:Episode\s*)?(?<number>\d+)\s*[\.:]?\s*(?<title>.*)?\.\w{3})"};

    for (int i=0;i<fileNames.count ();i++) {
        QString fileName = fileNames[i];
        QRegularExpressionMatch match = fileNameRegex.match (fileName);
        QString title = "";
        int number = i;
        if (match.hasMatch()) {
            if (!match.captured("title").isEmpty()) {
                title = match.captured("title").trimmed ();
            }
            number = match.captured("number").toInt ();
        }
        Episode playFile(number,"");
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
    m_currentIndex = loadIndex;
    m_currentShowName = directory.dirName ();
    emit showNameChanged();
    emit layoutChanged ();
    if(play){
        loadSource (loadIndex);
    }
}



void PlaylistModel::loadSource(int index){
    if(!isValidIndex(index))return;
    setLoading(true);

    emit currentIndexChanged();
    if(online){
        m_watcher.setFuture (QtConcurrent::run([&]() {
            loadOnlineSource (index);
        }));

    }else{

    }
}

void PlaylistModel::loadOnlineSource(int index){
    try{
        qDebug()<<"Fetching servers for" << m_playlist[index].number;
        QVector<VideoServer> servers = currentProvider->loadServers (m_playlist[index]);
        if(!servers.empty ()){
            qDebug()<<"Successfully fetched servers for" << m_playlist[index].number;
            QString source = currentProvider->extractSource(servers[0]);
            emit sourceFetched(source);
            if(m_watchListShowItem){
                m_watchListShowItem->at("lastWatchedIndex")= index;
                emit updatedLastWatchedIndex();
            }
            if(m_currentShowLink == Application::instance().currentShowObject.link ()){
                Application::instance().currentShowObject.setLastWatchedIndex (index);
                qDebug()<<"set index";
            }
            this->m_currentIndex = index;
        }
        setLoading(false);
    }catch(const QException& e){
        qCritical() << e.what ();
        emit encounteredError("Encountered error while extracting" + QString(e.what ()));
        setLoading(false);
    }
}

void PlaylistModel::loadLocalSource(int index){
    emit sourceFetched(m_playlist[index].localPath);
    if (m_historyFile->open(QIODevice::WriteOnly)) {
        QTextStream stream(m_historyFile);
        stream<<m_playlist[index].localPath.split ("/").last ();
        m_historyFile->close ();
    }
    setLoading(false);
    this->m_currentIndex = index;
}

void PlaylistModel::syncList(){
    if(Application::instance().currentShowObject.link ()==m_currentShow.link)return;
    online=true;
    m_playlist = Application::instance().currentShowObject.episodes();
    emit layoutChanged ();
    currentProvider = Application::instance().getCurrentShowProvider ();
    m_currentShowLink = Application::instance().currentShowObject.link ();
    m_currentShowName = Application::instance().currentShowObject.title ();
    m_currentShow = Application::instance().currentShowObject.getShow ();
    emit showNameChanged();
}

void PlaylistModel::loadOffset(int offset){
    auto newIndex = m_currentIndex+offset;
    if(isValidIndex(newIndex)){
        m_currentIndex = newIndex;
        loadSource(m_currentIndex);
    }
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_playlist.count ();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const Episode& episode = m_playlist[index.row()];
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
