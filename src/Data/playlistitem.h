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
    std::unique_ptr<QFileSystemWatcher> m_folderWatcher = nullptr;
    QTimer *m_fileCloseTimer = nullptr;

    PlaylistItem *m_parent = nullptr;
    std::unique_ptr<QList<PlaylistItem*>> m_children = nullptr;

private:
    QString fullName;
    ShowProvider* provider;
public:
    //List
    PlaylistItem(const QString& name, ShowProvider* provider, const QString &link, PlaylistItem* parent)
        : name(name), provider(provider), link(std::move(link)), m_parent(parent), type(LIST) {}

    //Item
    PlaylistItem(float number, const QString &link, const QString &name, PlaylistItem *parent, bool online = true);
    ~PlaylistItem() {
        // qDebug() << "deleted" << (m_parent != nullptr ? m_parent->link : "") << fullName;
        clear();
    }

    std::atomic<int> useCount = 0; //todo

    inline ShowProvider *getProvider() const { return provider; }
    inline QString getFullName() const { return fullName; }

    QUrl loadLocalSource(int index);
    static PlaylistItem *fromUrl(const QUrl &pathUrl, PlaylistItem *parent = nullptr);

    void emplaceBack(float number, const QString &link, const QString &name, bool online = true);
    void clear();

    PlaylistItem *parent() const { return m_parent; }
    int row() {
        return m_parent ? m_parent->m_children->indexOf(const_cast<PlaylistItem *>(this)) : 0;
    }
    PlaylistItem *at(int i) const {
        return (m_children || !m_children->isEmpty()
                || i >= 0 || i < m_children->count()) ? m_children->at(i) : nullptr;
    }
    PlaylistItem *first() {
        return (!m_children || m_children->isEmpty()) ? nullptr : m_children->operator[](0);
    }
    bool isEmpty() const { return !m_children || m_children->count() == 0; }

    enum Type { LIST, ONLINE, LOCAL };
    Type type;

    QString name;
    float number = -1;
    QString link;
    int currentIndex = -1;
    int lastPlayTime = 0;

public:
    int size() const {
        if (!m_children || m_children->isEmpty ())
            return 0;
        return m_children->count();
    }
    QString getDisplayName(int index) const;
    bool isValidIndex(int index) const;
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
