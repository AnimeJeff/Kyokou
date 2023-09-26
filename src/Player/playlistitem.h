#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QDir>
#include <QAbstractListModel>
#include <QtConcurrent>
#include <QStandardItemModel>
#include "Explorer/Data/episode.h"
//#include "Providers/showprovider.h"
#include <nlohmann/json.hpp>

struct VideoServer{
    QString name;
    std::string link;
    QMap<QString,QString> headers;
    QString source;
    bool working = true;
    struct SkipData{
        unsigned int introBegin;
        unsigned int introEnd;
        unsigned int outroBegin;
        unsigned int outroEnd;
    };
    std::optional<SkipData> skipData;
};
class ShowProvider;
class PlaylistItem
{

private:
    QFile* m_historyFile = nullptr;
    PlaylistItem* m_parent;
    QVector<std::shared_ptr<PlaylistItem>>* m_children = nullptr;
public:

    PlaylistItem(const QString &name, int provider, PlaylistItem *parent = nullptr)
        : name(name), provider(provider), m_parent(parent)
    {
        qDebug() << 1;
        type = LIST;
    }

    // playlist item constructor, must provide a parent playlist
    PlaylistItem(int number, const std::string& link, const QString& name, PlaylistItem *parent, bool online = true)
        :number(number), name(name), link(link)
    {
        qDebug() << online;
        type = online ? ONLINE : LOCAL;
        qDebug() << type;

        if(number > 0)
        {  // do not show negative episode number - episode is a movie quality;
            this->fullName = QString::number(number);
            if (!name.isEmpty ())
            {
                this->fullName += QString(". ");
            }
        }
        this->fullName += name;
        if(this->fullName.isEmpty ())
        {
            this->fullName = "Unamed Episode";
        }
    };
    ~PlaylistItem(){delete m_children;}


    QString loadLocalSource(int index)
    {
        if (m_historyFile->open(QIODevice::WriteOnly)) {
            QTextStream stream(m_historyFile);
            stream << QString::fromStdString (m_children->at(index)->link).split ("/").last ();
            m_historyFile->close ();
        }
        this->currentIndex = index;
        return QString::fromStdString (m_children->at(index)->link);
    }
    void loadFromLocalDir(const QUrl &localFolderPath, PlaylistItem *parent = nullptr);

    QVector<std::shared_ptr<PlaylistItem>>* getChildren()
    {
        if (!m_children) m_children = new QVector<std::shared_ptr<PlaylistItem>>();
        return m_children;
    }

    void emplaceBack(int number, const std::string& link, const QString& name, PlaylistItem *parent, bool online = true)
    {
        if (!m_children) m_children = new QVector<std::shared_ptr<PlaylistItem>>();
        m_children->emplaceBack (std::make_shared<PlaylistItem>(number, link, name, parent, online));
    }

    void clear()
    {
        if(m_children)
            m_children->clear ();
    }

    PlaylistItem *getParent() const
    {
        return m_parent;
    }
    void setParent(PlaylistItem *parent)
    {
        m_parent = parent;
    }

    std::shared_ptr<PlaylistItem> &operator[](int i) {
        if(!m_children || m_children->empty () || i < 0 || i >= m_children->count ()){
            throw "Index out of bound";
        }
        return m_children->operator[](i);
    }

    const std::shared_ptr<PlaylistItem> &at(int i) const
    {
        if(!m_children || m_children->empty () || i < 0 || i >= m_children->count ()){
            throw "Index out of bound";
        }
        return m_children->at(i);
    }

    std::shared_ptr<PlaylistItem> &first()
    {
        if(!m_children || m_children->empty ()){
            throw "Index out of bound";
        }
        return m_children->operator[](0);
    }

    bool isEmpty() const
    {
        if (!m_children) return true;
        return m_children->count () == 0;
    }

//    int indexOf(std::shared_ptr<PlaylistItem> item)
//    {
//        if(!m_children) return -1;
//        for(int i = 0; i < m_children->count (); i++)
//        {
//            if (m_children->at (i) == item)
//            {
//                return i;
//            }
//        }
//        return -1;
//    }

    enum Type
    {
        LIST,
        ONLINE,
        LOCAL
    };
    Type type;
    nlohmann::json* m_watchListShowItem { nullptr };
    QString name;
    int number = -1;
    std::string link;
    QString fullName;
    std::string sourceLink { "" };
    bool online = true;
    int currentIndex = 0;
    int provider;
    std::shared_ptr<ShowProvider> currentProvider;
public:
    int count() const {
        if(!m_children) return 0;
        return m_children->count ();
    }
    QString getItemName(int index) const
    {
        if(!isValidIndex(index)) return "";
        auto currentItem = m_children->at(index);
        QString itemName = "%1\n[%2/%3] %4";
        itemName = itemName.arg (name).arg (index+1).arg (m_children->size ()).arg (currentItem->fullName);
        return itemName;
    }
    bool isValidIndex(int index) const;

};


#endif // PLAYLISTITEM_H
