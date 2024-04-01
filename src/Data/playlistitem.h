#pragma once
#include <QAbstractListModel>
#include <QDir>
#include <QStandardItemModel>
#include <QtConcurrent>
#include <memory>

class ShowProvider;

class PlaylistItem
{

private:
    std::unique_ptr<QFile> m_historyFile = nullptr;
    QTimer *m_fileCloseTimer = nullptr;

    PlaylistItem *m_parent = nullptr;
    std::unique_ptr<QList<PlaylistItem*>> m_children = nullptr;

private:
    QString fullName;
    ShowProvider* provider;
public:
    //List
    PlaylistItem(const QString& name, ShowProvider* provider, std::string link, PlaylistItem* parent)
        : name(name), provider(provider), link(std::move(link)), m_parent(parent), type(LIST) {}

    //Item
    PlaylistItem(float number, const std::string &link, const QString &name, PlaylistItem *parent, bool online = true);
    ~PlaylistItem() {
        // qDebug() << "deleted" << (m_parent != nullptr ? m_parent->link : "") << fullName;
        clear();
    }

    std::atomic<int> useCount = 0; //todo

    inline ShowProvider *getProvider() const { return provider; }
    inline QString getFullName() const { return fullName; }

    QUrl loadLocalSource(int index);
    static PlaylistItem *fromLocalDir(const QString &localDir);

    void emplaceBack(float number, const std::string &link, const QString &name, bool online = true);
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
    std::string link;
    int currentIndex = -1;

public:
    int count() const {
        if (!m_children || m_children->isEmpty ())
            return 0;
        return m_children->count();
    }
    QString getDisplayName(int index) const;
    bool isValidIndex(int index) const;
};

struct VideoServer {
    QString name;
    std::string link;
    QMap<QString, QString> headers;
    QString source;
    bool working = true;
    struct SkipData {
        unsigned int introBegin;
        unsigned int introEnd;
        unsigned int outroBegin;
        unsigned int outroEnd;
    };
    std::optional<SkipData> skipData;
    VideoServer(const QString& name, const std::string& link):name(name),link(link){

    }
};
