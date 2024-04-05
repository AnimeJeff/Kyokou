#pragma once


#include <QAbstractListModel>
#include <QDir>
#include <QStandardItemModel>
#include <QtConcurrent>
#include <memory>

class ShowProvider;
class Video;

class PlaylistItem {
public:
    //List
    PlaylistItem(const QString& name, ShowProvider* provider, const QString &link)
        : name(name), m_provider(provider), link(link), type(LIST) {

    }

    static PlaylistItem *fromUrl(const QUrl &pathUrl, PlaylistItem *parent = nullptr);

    //Item
    PlaylistItem(float number, const QString &link, const QString &name, PlaylistItem *parent, bool isLocal = false);
    ~PlaylistItem() {
        // qDebug() << "deleted" << (m_parent != nullptr ? m_parent->link : "") << fullName;
        clear();
    }

    inline bool reloadFromFolder() { return loadFromFolder (QUrl()); }

    inline PlaylistItem *parent() const { return m_parent; }
    inline PlaylistItem *at(int i) const { return !isValidIndex(i) ? nullptr : m_children->at(i); }
    inline int row() { return m_parent ? m_parent->m_children->indexOf(const_cast<PlaylistItem *>(this)) : 0; }
    inline PlaylistItem *first() const { return at(0); }
    inline PlaylistItem *getCurrentItem() const { return at(currentIndex); }
    inline int indexOf(PlaylistItem *child) { return m_children->indexOf (child); }
    int indexOf(const QString &link);
    inline bool isEmpty() const { return !m_children || m_children->size() == 0; }
    inline int size() const { return m_children ? m_children->size() : 0; }
    bool isValidIndex(int index) const;

    void append(PlaylistItem *value);
    void emplaceBack(float number, const QString &link, const QString &name, bool isLocal = false);
    void clear();
    void removeAt(int index);
    bool replace(int index, PlaylistItem *value);
    QString getDisplayNameAt(int index) const;
    void updateHistoryFile(qint64 time = 0);
    void setLastPlayAt(int index, int time);
    inline bool isLoadedFromFolder() const { return m_isLoadedFromFolder; }

    inline ShowProvider *getProvider() const { return m_provider; }
    inline QString getFullName() const { return fullName; }

    enum Type { LIST, ONLINE, LOCAL };
    Type type;

    QString name;
    float number = -1;
    QString link;
    int currentIndex = -1;
    int timeStamp = 0;
    void use(){
        ++useCount;
    }
    void disuse() {
        --useCount;
        if (useCount == 0) {
            qDebug() << "Log (Downloader): Playlist deleted by downloader" ;
            delete this;
        }
    }
private:
    QString fullName;
    ShowProvider* m_provider;
    bool m_isLoadedFromFolder = false;
    std::unique_ptr<QFile> m_historyFile = nullptr;
    inline static QRegularExpression fileNameRegex{ R"((?:Episode\s*)?(?<number>\d+)\s*[\.:]?\s*(?<title>.*)?\.\w{3})" };
    PlaylistItem *m_parent = nullptr;
    std::unique_ptr<QList<PlaylistItem*>> m_children = nullptr;
    std::atomic<int> useCount = 0;

    bool loadFromFolder(const QUrl &pathUrl);
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
