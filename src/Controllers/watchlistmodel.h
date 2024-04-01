#pragma once
#include <QAbstractListModel>
#include <QStandardItemModel>
#include <QDir>
#include <QCoreApplication>
#include <QtConcurrent>
#include "Data/showdata.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class WatchListModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int listType READ getCurrentListType WRITE setDisplayingListType NOTIFY layoutChanged)
    Q_PROPERTY(QVariant changeableListTypes READ getChangeableListTypes NOTIFY layoutChanged)


    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    bool isLoading() { return loading; }
    bool loading = false;

public:
    explicit WatchListModel(QObject *parent = nullptr): QAbstractListModel(parent){
        loadWatchList ();
        //m_watcher->setFuture (QtConcurrent::run(&WatchListModel::fetchUnwatchedEpisodes, this, WATCHING));
    };
    enum ListType
    {
        WATCHING,
        PLANNED,
        ON_HOLD,
        DROPPED,
        COMPLETED
    };
    Q_ENUM(ListType);

    enum PropertyType {
        INT,
        STRING
    };


    Q_INVOKABLE QString displayableListType(int listType){
        switch (listType) {
        case WATCHING:
            return "Watching";
        case PLANNED:
            return "Planned";
        case ON_HOLD:
            return "On Hold";
        case DROPPED:
            return "Dropped";
        case COMPLETED:
            return "Completed";
        }
        return "Undefined";
    }
    void updateLastWatchedIndex(const QString& showLink, int lastWatchedIndex) {
        updateProperty(showLink, "lastWatchedIndex", lastWatchedIndex, INT);
    }
    void updateProperty(const QString& showLink, QString propertyName, QVariant propertyValue, PropertyType propertyType){
        if (!m_showHashmap.contains(showLink)) return;

        QPair<int, int> listTypeAndIndex = m_showHashmap.value(showLink);
        int listType = listTypeAndIndex.first;
        int index = listTypeAndIndex.second;

        QJsonArray list = m_watchListJson[listType].toArray();
        QJsonObject show = list[index].toObject();

        if (propertyType == INT) {
            show.operator[](propertyName) = propertyValue.toInt ();
        } else {
            show.operator[](propertyName)= propertyValue.toString ();
        }


        list[index] = show; // Update the show in the list
        m_watchListJson[listType] = list; // Update the list in the model

        save(); // Save changes
    }
    void syncShow(ShowData& show);
    void add(ShowData& show, int listType);
    void remove(ShowData& show);
    Q_INVOKABLE void removeAt(int index, int listType = -1) {
        if (listType < 0 || listType > 4) listType = m_currentListType;
        QJsonArray list = m_watchListJson[listType].toArray();
        qDebug () << index << list.size ();
        if (index < 0 || index >= list.size ()) return;


        qDebug() << index << displayableListType (static_cast<ListType>(listType));

        auto showLink = list[index].toObject ()["link"].toString ();

        // Remove the show from the list
        list.removeAt(index);
        m_watchListJson[listType] = list; // Update the list in the JSON structure

        // Remove the show from the hashmap
        m_showHashmap.remove(showLink);

        for (int i = index; i < list.size(); ++i) {
            QJsonObject show = list[i].toObject();
            QString showLink = show["link"].toString();
            m_showHashmap[showLink].second = i; // Update index
        }

        // If the current list type is being displayed, update the model accordingly
        if (m_currentListType == listType) {
            beginRemoveRows(QModelIndex(), index, index);
            endRemoveRows();
        }

        save(); // Save the changes to the JSON file
    }

    int getCurrentListType() const { return m_currentListType; }
    void setDisplayingListType(int listType) {
        if (listType == m_currentListType) return;
        m_currentListType = listType;
        emit layoutChanged();
    }

    Q_INVOKABLE void cycleDisplayingListType(){
        setDisplayingListType ((m_currentListType + 1) % 5);
    }
    Q_INVOKABLE QVariant getChangeableListTypes(){
        QList<int> types = { WATCHING, PLANNED, ON_HOLD, DROPPED, COMPLETED };
        // QList<QString> types = { "Watching", "Planned", "On Hold", "Dropped", "Completed" };
        types.remove (m_currentListType);
        return QVariant::fromValue (types);
    }


    Q_INVOKABLE void changeListTypeAt(int index, int newListType, int oldListType = -1);
    void changeShowListType(ShowData& show, int newListType);


    QJsonObject loadShow(int index);
    inline bool inWatchList(const QString& showLink){
        return m_showHashmap.contains (showLink);
    }
private:
    void save();
private:
    QJsonArray m_watchListJson;
    QHash<QString, QPair<int, int>> m_showHashmap;
    // QHash<QString, int> totalEpisodeMap; //TODO
    int m_currentListType = WATCHING;
private:
    void loadWatchList(QString filePath = "");
    void fetchUnwatchedEpisodes(int listType);
    QString watchListFilePath;
    QMutex mutex;

signals:
    void loadingChanged(void);
public slots:
    void move(int from, int to);

private:
    enum{
        TitleRole = Qt::UserRole,
        CoverRole,
        UnwatchedEpisodesRole
    };
    int rowCount(const QModelIndex &parent) const override ;
    QVariant data(const QModelIndex &index, int role) const override ;
    QHash<int, QByteArray> roleNames() const override ;
};

