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

class LibraryModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int listType READ getCurrentListType WRITE setDisplayingListType NOTIFY layoutChanged)
    Q_PROPERTY(QVariant changeableListTypes READ getChangeableListTypes NOTIFY layoutChanged)


    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    bool isLoading() { return m_isLoading; }
    bool m_isLoading = false;

public:
    explicit LibraryModel(QObject *parent = nullptr): QAbstractListModel(parent){
        loadWatchList ();
        //m_watcher->setFuture (QtConcurrent::run(&LibraryModel::fetchUnwatchedEpisodes, this, WATCHING));
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
    inline void updateLastWatchedIndex(const QString& showLink, int lastWatchedIndex) {
        updateProperty(showLink, "lastWatchedIndex", lastWatchedIndex, INT);
        updateProperty(showLink, "timeStamp", 0, INT);
    }
    inline void updateTimeStamp(const QString& showLink, int timeStamp) {
        updateProperty(showLink, "timeStamp", timeStamp, INT);
    }
    void updateProperty(const QString& showLink, QString propertyName, QVariant propertyValue, PropertyType propertyType);

    ShowData::LastWatchInfo getLastWatchInfo(const QString& showLink) {
        // Check if the show exists in the hashmap
        int lastWatchedIndex = -1;
        int timeStamp = 0;
        ShowData::LastWatchInfo info;
        if (m_showHashmap.contains (showLink)) {
            // Retrieve the list type and index for the show
            QPair<int, int> listTypeAndIndex = m_showHashmap.value(showLink);
            int listType = listTypeAndIndex.first;
            int index = listTypeAndIndex.second;
            QJsonArray list = m_watchListJson.at(listType).toArray();
            QJsonObject showObject = list.at(index).toObject();

            // Sync details
            info.listType = listType;
            info.lastWatchedIndex = showObject["lastWatchedIndex"].toInt(-1);
            info.timeStamp = showObject["timeStamp"].toInt(0);
        }
        return info;
    }

    void add(ShowData& show, int listType);
    void remove(ShowData& show);
    Q_INVOKABLE void removeAt(int index, int listType = -1);

    int getCurrentListType() const { return m_currentListType; }
    void setDisplayingListType(int listType) {
        if (listType == m_currentListType) return;
        m_currentListType = listType;
        emit layoutChanged();
    }

    Q_INVOKABLE void cycleDisplayingListType();
    Q_INVOKABLE QVariant getChangeableListTypes();


    Q_INVOKABLE void changeListTypeAt(int index, int newListType, int oldListType = -1);
    void changeShowListType(ShowData& show, int newListType);

    QJsonObject loadShow(int index);


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
    void isLoadingChanged(void);
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

