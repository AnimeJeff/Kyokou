#ifndef WATCHLISTMODEL_H
#define WATCHLISTMODEL_H

#include <global.h>
#include <fstream>
#include <iostream>
#include <QAbstractListModel>
#include <QStandardItemModel>
#include "parsers/data/mediadata.h"
#include <nlohmann/json.hpp>


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
    QVector<std::shared_ptr<MediaData>> m_watchingList;
    QVector<std::shared_ptr<MediaData>> m_plannedList;
    QVector<std::shared_ptr<MediaData>> m_onHoldList;
    QVector<std::shared_ptr<MediaData>> m_droppedList;
    QVector<std::shared_ptr<MediaData>> m_completedList;
    QVector<std::shared_ptr<MediaData>>* m_currentList = &m_watchingList;
    QMap<int,QVector<std::shared_ptr<MediaData>>*>listMap{
        {WATCHING, &m_watchingList},
        {PLANNED, &m_plannedList},
        {ON_HOLD, &m_onHoldList},
        {DROPPED, &m_droppedList},
        {COMPLETED,&m_completedList}
    };
    int m_displayingListType = WATCHING;
    int getDisplayingListType(){
        return m_displayingListType;
    }
    void setDisplayingListType(int listType){
        m_displayingListType = listType;
        m_currentList = listMap[listType];
        emit layoutChanged ();
    }
    const std::string watchListFileName = ".watchlist";
private:
    void readWatchListFile(){
        std::ifstream infile(watchListFileName);
        if (!infile.good()) { // file doesn't exist or is corrupted
            std::ofstream outfile(watchListFileName); // create new file
            outfile << "[[],[],[],[],[]]"; // write [] to file
            outfile.close();
        }else if(infile.peek() == '['){
            infile>> m_jsonList;
        }else{
            //            m_jsonList = nlohmann::json::array ();

        }
    }
    void parseWatchListFile(){
        for (int i = 0;i<m_jsonList.size ();i++){
            auto array = m_jsonList[i];
            if(array.empty ())continue;
            QVector<std::shared_ptr<MediaData>>* list;
            switch(i){
            case WATCHING:
                list = &m_watchingList;
                break;
            case PLANNED:
                list = &m_plannedList;
                break;
            case ON_HOLD:
                list = &m_onHoldList;
                break;
            case DROPPED:
                list = &m_droppedList;
                break;
            case COMPLETED:
                list = &m_completedList;
                break;
            }
            for(auto& item:array){
                list->push_back(std::make_shared<MediaData>(MediaData(item)));
                list->back ()->setListType (i);
            }
        }
    }
public:
    explicit WatchListModel(QObject *parent = nullptr): QAbstractListModel(parent){
        readWatchListFile();
        parseWatchListFile();
    };
    ~WatchListModel() {
        m_watchingList.clear ();
        m_plannedList.clear ();
        m_onHoldList.clear ();
        m_droppedList.clear ();
        m_completedList.clear();
    }
public:
    Q_INVOKABLE void add(MediaData show, int listType = WATCHING){
        m_jsonList[listType].push_back (show.toJson ());
        show.setJsonObject(m_jsonList[listType].back ());
        listMap[listType]->push_back (std::make_shared<MediaData>(show));
        if(m_displayingListType == listType) emit layoutChanged ();
        save();
    }

    Q_INVOKABLE void addCurrentShow(int listType = WATCHING);

    Q_INVOKABLE void remove(const MediaData& show){
        auto& list = m_jsonList[show.listType];
        for(int i = 0;i<list.size ();i++){
            if(list[i]["link"] == show.link.toStdString ()){
                list.erase (list.begin() + i);
                listMap[show.listType]->removeAt (i);
                emit layoutChanged();
                save();
                break;
            }
        }


    }

    Q_INVOKABLE void removeCurrentShow();

    Q_INVOKABLE void loadDetails(int index){
        switch(m_displayingListType){
        case WATCHING:
            emit detailsRequested(*m_watchingList[index]);
            break;
        case PLANNED:
            emit detailsRequested(*m_plannedList[index]);
            break;
        case ON_HOLD:
            emit detailsRequested(*m_onHoldList[index]);
            break;
        case DROPPED:
            emit detailsRequested(*m_droppedList[index]);
            break;
        default:
            return;
        }
    }

    Q_INVOKABLE void move(int from, int to){
        m_currentList->move (from,to);
        auto& list = m_jsonList[m_displayingListType];
        auto element_to_move = list[from];
        list.erase(list.begin() + from);
        list.insert(list.begin() + to, element_to_move);
    }

    Q_INVOKABLE void moveEnded();

    bool checkInList(const MediaData& show){
        if(show.isInWatchList()){
            return true;
        }else if(getShowInList (show)){
            return true;
        }
        return false;
    }

    std::shared_ptr<MediaData> getShowInList(const MediaData& show) {
        for(int i = 0;i<m_jsonList.size ();i++){
            nlohmann::json::array_t list = m_jsonList[i];
            //                qDebug()<<list.;
            for(int j = 0;j<list.size ();j++){
                if(list[j]["link"] == show.link.toStdString ()){
                    return listMap[i]->at (j);
                }
            }
        }
        qDebug()<<"Unable to find" << show.title << "in watch list";
        return nullptr;
    }

    nlohmann::json* getShowJsonInList(const MediaData& show) {
        for(int i = 0;i<m_jsonList.size ();i++){
            nlohmann::json::array_t list = m_jsonList[i];
            for(int j = 0;j<list.size ();j++){
                if(list[j]["link"] == show.link.toStdString ()){
                    return &m_jsonList[i][j];
                }
            }
        }
        qDebug()<<"Unable to find" << show.title << "json in watch list";
        return nullptr;
    }
    inline bool isLoading(){
        return loading;
    }
    bool loading = false;
signals:
    void detailsRequested(MediaData show);
    void loadingChanged(void);
public slots:
    bool checkCurrentShowInList();

    void save(){
        std::ofstream output_file(watchListFileName);
        output_file << m_jsonList.dump (4);
        output_file.close();
        qDebug()<<"saved";
    }

private:
    enum{
        TitleRole = Qt::UserRole,
        CoverRole
    };
    int rowCount(const QModelIndex &parent) const override ;
    QVariant data(const QModelIndex &index, int role) const override ;
    QHash<int, QByteArray> roleNames() const override ;
};

#endif // WATCHLISTMODEL_H
