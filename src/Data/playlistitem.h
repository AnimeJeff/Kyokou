#pragma once
#include <QAbstractListModel>
#include <QDir>
#include <QStandardItemModel>
#include <QtConcurrent>
#include <memory>

class ShowProvider;

class PlaylistItem {

private:
    std::unique_ptr<QFile> m_historyFile = nullptr;
    // QTimer *m_fileCloseTimer = nullptr;

    inline static QRegularExpression fileNameRegex{ R"((?:Episode\s*)?(?<number>\d+)\s*[\.:]?\s*(?<title>.*)?\.\w{3})" };
    PlaylistItem *m_parent = nullptr;
    std::unique_ptr<QList<PlaylistItem*>> m_children = nullptr;

    bool loadFromFolder(const QUrl &pathUrl = QUrl()) {
        if (!m_isLoadedFromFolder) return false;
        clear ();

        QDir playlistDir;
        QString currentFilename = "";
        QString timeString = "";
        if (!pathUrl.isEmpty ()) {
            QFileInfo path = QFileInfo(pathUrl.toLocalFile ());
            if (!path.exists ()) {
                qDebug() << "Log (PlaylistItem): Path" << path << "doesn't exist";
                return false;
            }

            if (!path.isDir ()) {
                playlistDir = path.dir ();
                currentFilename = path.fileName ();
            } else {
                playlistDir = QDir(pathUrl.toLocalFile ());
            }
            qDebug() << playlistDir <<playlistDir.dirName() << playlistDir.absolutePath ();
            m_historyFile = std::make_unique<QFile> (playlistDir.filePath(".mpv.history"));
            name = playlistDir.dirName();
            link = playlistDir.absolutePath ();

            // Read/update history file
            if (path.isFile()) {
                bool fileOpened = m_historyFile->open(QIODevice::WriteOnly | QIODevice::Text);
                if (!fileOpened) {
                    qDebug() << "Log (PlaylistItem): Failed to open history file";
                    return false;
                }
                m_historyFile->write(currentFilename.toUtf8());
                m_historyFile->close ();
            }
        } else {
            playlistDir = QDir(link);
            if (!playlistDir.exists ()) {
                qDebug() << "Log (PlaylistItem): Path" << link << "doesn't exist";
                currentIndex = -1;
                return false;
            }
        }


        QStringList fileNames = playlistDir.entryList(
            {"*.mp4", "*.mkv", "*.avi", "*.mp3", "*.flac", "*.wav", "*.ogg", "*.webm"}, QDir::Files);
        if (fileNames.isEmpty()) {
            qDebug() << "Log (PlaylistItem): No files to play in" << playlistDir.absolutePath ();
            currentIndex = -1;
            return false; //TODO
        }

        for (int i = 0; i < fileNames.count(); i++) {
            QRegularExpressionMatch match = fileNameRegex.match(fileNames[i]);
            QString title = match.hasMatch() ? match.captured("title").trimmed() : "";
            int itemNumber = match.hasMatch() ? !match.captured("number").isEmpty() ? match.captured("number").toInt() : i : i;

            auto childItem = new PlaylistItem(itemNumber, playlistDir.absoluteFilePath(fileNames[i]), title, this, true);
            m_children->push_back(childItem);
        }

        // sort the episodes in order
        std::sort(m_children->begin(), m_children->end(),
                  [](const PlaylistItem *a, const PlaylistItem *b) {
                      return a->number < b->number;
                  });


        // Read history file
        if (currentFilename.isEmpty() && m_historyFile->exists ()) {
            // Open history file
            bool fileOpened = m_historyFile->open(QIODevice::ReadOnly | QIODevice::Text);
            if (!fileOpened) {
                qDebug() << "Log (PlaylistItem): Failed to open history file";
                return false;
            }
            auto fileData = QTextStream(m_historyFile.get()).readAll().trimmed().split(" ");
            m_historyFile->close();
            if (!fileData.isEmpty()) {
                currentFilename = fileData.first();
                if (fileData.size() == 2) {
                    timeString = fileData.last();
                }
            }

        }

        // Set current index and time
        if (!currentFilename.isEmpty()) {
            bool found = false;
            for (int i = 0; i < m_children->size(); i++) {
                if (m_children->at(i)->link.split("/").last() == currentFilename) {
                    currentIndex = i;
                    found = true;
                    break;
                }
            }
            if (found && !timeString.isEmpty ()){
                bool ok;
                int intTime = timeString.toInt (&ok);
                if (ok) {
                    currentItem ()->lastPlayTime = intTime;
                }
                // qDebug() << "Updating local last play" << i << timeString << ok << intTime;
            }
        }




        if (currentIndex < 0) currentIndex = 0;
        return true;
    }


public:
    //List
    PlaylistItem(const QString& name, ShowProvider* provider, const QString &link, PlaylistItem* parent)
        : name(name), m_provider(provider), link(std::move(link)), m_parent(parent), type(LIST) {}

    //Item
    PlaylistItem(float number, const QString &link, const QString &name, PlaylistItem *parent, bool online = true);
    ~PlaylistItem() {
        qDebug() << "deleted" << (m_parent != nullptr ? m_parent->link : "") << fullName;
        clear();
    }

    static PlaylistItem *fromUrl(const QUrl &pathUrl, PlaylistItem *parent = nullptr);



    QUrl loadLocalSource(int index);
    void append(PlaylistItem *value) {
        if (!m_children) m_children = std::make_unique<QList<PlaylistItem*>>();
        value->useCount++;
        value->m_parent = this;
        m_children->push_back (value);
    }
    void emplaceBack(float number, const QString &link, const QString &name, bool online = true);
    void clear();
    void removeAt(int index) {
        auto toRemove = at(index);
        if (toRemove) {
            toRemove->m_parent = nullptr;
            if (--toRemove->useCount == 0) {
                delete toRemove;
            }
            m_children->removeAt (index);
        }
    }
    void replace(int index, PlaylistItem *value) {
        auto toRemove = at(index);
        if (toRemove) {
            toRemove->m_parent = nullptr;
            if (--toRemove->useCount == 0) {
                delete toRemove;
            }
            m_children->replace (index, value);
            value->m_parent = this;
            value->useCount++;

        }
    }

    PlaylistItem *parent() const { return m_parent; }
    int row() { return m_parent ? m_parent->m_children->indexOf(const_cast<PlaylistItem *>(this)) : 0; }
    PlaylistItem *at(int i) const { return !isValidIndex(i) ? nullptr : m_children->at(i); }
    inline PlaylistItem *first() const { return at(0); }
    inline PlaylistItem *currentItem() const { return at(currentIndex); }
    int indexOf(PlaylistItem *child) {
        if (isEmpty ()) return -1;
        return m_children->indexOf (child);
    }
    int indexOf(const QString &link) {
        for (int i = 0; i < m_children->size (); i++) {
            auto child = m_children->at (i);
            if (child->link == link) {
                return i;
            }
        }
        return -1;
    }




    inline bool isEmpty() const { return !m_children || m_children->count() == 0; }
    inline bool hasSameLink(PlaylistItem *playlist) { return playlist ? link == playlist->link : false; }
    bool isValidIndex(int index) const;
    int size() const {
        if (isEmpty ()) return 0;
        return m_children->count();
    }

    QString getDisplayNameAt(int index) const;
    void updateHistoryFile(qint64 time = 0) {
        static QMutex mutex;
        mutex.lock ();
        Q_ASSERT(m_historyFile);
        if (m_historyFile->isOpen() || m_historyFile->open(QIODevice::WriteOnly)) {
            // Q_ASSERT(m_fileCloseTimer);
            // m_fileCloseTimer->start ();
            m_historyFile->resize(0);
            QTextStream stream(m_historyFile.get ());
            QString lastWatchedFilePath = m_children->at(currentIndex)->link;
            stream << lastWatchedFilePath.split("/").last();
            if (time > 0) {
                stream << " " << QString::number (time);
            }
            m_historyFile->close();
        }
        mutex.unlock ();
    }
    void setLastPlayAt(int index, int time) {
        if (!isValidIndex (index)) return;
        qDebug() << "setting playlist item " << index << time;
        currentIndex = index;
        m_children->at (index)->lastPlayTime = time;
    }
    bool isLoadedFromFolder() const { return m_isLoadedFromFolder; }
    inline ShowProvider *getProvider() const { return m_provider; }
    inline QString getFullName() const { return fullName; }

    enum Type { LIST, ONLINE, LOCAL };
    Type type;

    QString name;
    float number = -1;
    QString link;
    int currentIndex = -1;
    int lastPlayTime = 0;
    std::atomic<int> useCount = 0;
private:
    QString fullName;
    ShowProvider* m_provider;
    bool m_isLoadedFromFolder = false;
};

struct VideoServer {
    QString name;
    QString link;
    QMap<QString, QString> headers;
    bool working = true;
    struct SkipData {
        unsigned int introBegin;
        unsigned int introEnd;
        unsigned int outroBegin;
        unsigned int outroEnd;
    };
    std::optional<SkipData> skipData;
    VideoServer(const QString& name, const QString& link):name(name),link(link){

    }
};
