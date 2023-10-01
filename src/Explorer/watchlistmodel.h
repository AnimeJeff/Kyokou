#pragma once
#include <fstream>
#include <iostream>
#include <QAbstractListModel>
#include <QStandardItemModel>
#include <nlohmann/json.hpp>
#include <QDir>
#include <QCoreApplication>
#include <QtConcurrent>
#include "Data/showdata.h"

class WatchListModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int listType READ getCurrentListType WRITE setDisplayingListType NOTIFY layoutChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
private:
    enum ListType
    {
        WATCHING,
        PLANNED,
        ON_HOLD,
        DROPPED,
        COMPLETED
    };
    //hashmap containing json for fast indexing
    nlohmann::json m_jsonList;
    int m_currentListType = WATCHING;
    int getCurrentListType();
    void setDisplayingListType(int listType);
    bool isLoading()
    {
        return loading;
    }
    bool loading = false;
private:
    std::unordered_map<std::string,std::tuple<nlohmann::json*,int,int>> jsonHashmap;
    void loadWatchList(QString filePath = "");
    void changeListType(const ShowData& show,int listType);
    void fetchUnwatchedEpisodes();
    QString watchListFilePath;
public:
    explicit WatchListModel(QObject *parent = nullptr): QAbstractListModel(parent){
        loadWatchList ();
//        auto lol = QtConcurrent::run([this](){
//            fetchUnwatchedEpisodes ();
//        });
    };
    Q_INVOKABLE void add(const ShowData& show, int listType = WATCHING);
    Q_INVOKABLE void addCurrentShow(int listType = WATCHING);
    Q_INVOKABLE void remove(const ShowData& show);
    Q_INVOKABLE void removeCurrentShow();
    Q_INVOKABLE void move(int from, int to);
    Q_INVOKABLE void moveEnded();
    Q_INVOKABLE void loadShow(int index);
signals:
    void loadingChanged(void);
public slots:
    bool syncCurrentShow();
    void save()
    {
        static std::mutex mutex;
        mutex.lock ();
        std::ofstream output_file(watchListFilePath.toStdString ());
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

