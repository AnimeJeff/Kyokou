#pragma once
#include <QAbstractListModel>
#include <QDir>
#include <QStandardItemModel>
#include <QtConcurrent>
#include <nlohmann/json.hpp>
#include <memory>

class ShowProvider;
class PlaylistItem {

private:
    std::unique_ptr<QFile> m_historyFile = nullptr;
    QTimer *m_fileCloseTimer = nullptr;

    PlaylistItem *m_parent = nullptr;
    std::unique_ptr<QList<PlaylistItem*>> m_children = nullptr;

private:
         // friend class PlaylistModel;
         // friend class WatchListModel;
         // friend class DownloadModel;
         // friend class ShowManager;
    nlohmann::json *m_watchListShowItem = nullptr;
    QString fullName;
    const QString provider;


public:
    PlaylistItem(const QString &name, const QString &provider, std::string link, PlaylistItem *parent = nullptr);
    PlaylistItem(int number, const std::string &link, const QString &name, PlaylistItem *parent, bool online = true);
    ~PlaylistItem() {

        qDebug() << "deleted" << (m_parent != nullptr ? m_parent->fullName : "") << fullName;
        clear();
    }

    std::atomic<int> useCount = 0; //todo
    inline QString getProviderName() const { return provider; }
    inline QString getFullName() const { return fullName; }
    inline nlohmann::json *getJsonPtr() const { return m_watchListShowItem; }
    void setJsonPtr(nlohmann::json *jsonPtr) { m_watchListShowItem = jsonPtr; }

    QUrl loadLocalSource(int index);
    static PlaylistItem *fromLocalDir(const QString &localDir);


    void emplaceBack(int number, const std::string &link, const QString &name, bool online = true);
    void clear();




    PlaylistItem *parent() const { return m_parent; }
    int row() {
        if (!m_parent) return 0;
        return m_parent->m_children->indexOf(const_cast<PlaylistItem *>(this));
    }
    const PlaylistItem *at(int i) const {
        Q_ASSERT(m_children || !m_children->isEmpty() || i >= 0 ||
                 i < m_children->count());
        if (!(m_children || !m_children->isEmpty() || i >= 0 ||
              i < m_children->count()))
            throw std::runtime_error("Index out of bounds");
        return m_children->at(i);
    }
    PlaylistItem *first() {
        Q_ASSERT(m_children || !m_children->isEmpty());
        if (!m_children || m_children->isEmpty())
            throw std::runtime_error("Children is empty");
        return m_children->operator[](0);
    }
    bool isEmpty() const {
        if (!m_children)
            return true;
        return m_children->count() == 0;
    }

    enum Type { LIST, ONLINE, LOCAL };
    Type type;

    QString name;
    int number = -1;
    std::string link;
    int currentIndex = -1;

public:
    int count() const {
        if (!m_children)
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
};
