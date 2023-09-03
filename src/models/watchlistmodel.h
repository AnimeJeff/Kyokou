#ifndef WATCHLISTMODEL_H
#define WATCHLISTMODEL_H

#include <fstream>
#include <iostream>
#include <QAbstractListModel>
#include <QStandardItemModel>
#include "parsers/data/showdata.h"
#include <nlohmann/json.hpp>
#include <QDir>
#include <QCoreApplication>
#include <QtConcurrent>

class WatchListModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int listType READ getDisplayingListType WRITE setDisplayingListType NOTIFY layoutChanged);
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged);
private:
    enum{
        WATCHING,
        PLANNED,
        ON_HOLD,
        DROPPED,
        COMPLETED
    };
    nlohmann::json m_jsonList;
    QVector<QVector<std::shared_ptr<ShowData>>> m_list;
//    QVector<std::shared_ptr<ShowData>>& m_currentList = m_list[WATCHING];

    int m_displayingListType = WATCHING;
    int getDisplayingListType(){
        return m_displayingListType;
    }
    void setDisplayingListType(int listType){
        m_displayingListType = listType;
        emit layoutChanged ();
    }
    bool isLoading(){
        return loading;
    }
    bool loading = false;
    QString watchListFileName = QDir::cleanPath (QCoreApplication::applicationDirPath() + QDir::separator() + ".watchlist");
private:
    void readWatchListFile();
    void parseWatchListFile();
    void changeListType(const ShowData& show,int listType);
    void fetchUnwatchedEpisodes();
public:
    explicit WatchListModel(QObject *parent = nullptr): QAbstractListModel(parent){
        m_list.reserve (5);
        readWatchListFile();
        parseWatchListFile();
        auto lol = QtConcurrent::run([this](){
            fetchUnwatchedEpisodes ();
        });

    };
    ~WatchListModel() {
        m_list.clear ();
    }
public:
    Q_INVOKABLE void add(const ShowData& show, int listType = WATCHING);
    Q_INVOKABLE void addCurrentShow(int listType = WATCHING);
    Q_INVOKABLE void remove(const ShowData& show);
    Q_INVOKABLE void removeCurrentShow();
    Q_INVOKABLE void move(int from, int to);
    Q_INVOKABLE void moveEnded();

    bool checkInList(const ShowData &show);
    std::shared_ptr<ShowData> getShowInList(const ShowData& show);
    nlohmann::json* getShowJsonInList(const ShowData& show);
    Q_INVOKABLE void loadDetails(int index);

signals:
    void detailsRequested(ShowData show);
    void loadingChanged(void);
public slots:
    bool checkCurrentShowInList();

    void save(){
        static std::mutex mutex;
        mutex.lock ();
        std::ofstream output_file(watchListFileName.toStdString ());
        output_file << m_jsonList.dump (4);
        output_file.close();
        mutex.unlock ();
        qDebug()<<"Saved";
    }

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

#endif // WATCHLISTMODEL_H
