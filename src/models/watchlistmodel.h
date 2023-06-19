#ifndef WATCHLISTMODEL_H
#define WATCHLISTMODEL_H

#include <global.h>
#include <fstream>
#include <iostream>
#include <QAbstractListModel>
#include <parsers/showresponse.h>
#include <nlohmann/json.hpp>


class WatchListModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int listType READ getlistType WRITE setListType NOTIFY layoutChanged);
private:
    enum{
        WATCHING,
        PLANNED,
        ON_HOLD,
        DROPPED,
        COMPLETED
    };
    nlohmann::json m_jsonList;
    QVector<std::shared_ptr<ShowResponse>> m_list;
    QVector<std::shared_ptr<ShowResponse>> m_watchingList;
    QVector<std::shared_ptr<ShowResponse>> m_plannedList;
    QVector<std::shared_ptr<ShowResponse>> m_onHoldList;
    QVector<std::shared_ptr<ShowResponse>> m_droppedList;
    QVector<std::shared_ptr<ShowResponse>> m_completedList;
    QVector<std::shared_ptr<ShowResponse>>* m_currentList = &m_watchingList;

    QMap<int,QVector<std::shared_ptr<ShowResponse>>*>listMap{
        {WATCHING, &m_watchingList},
        {PLANNED, &m_plannedList},
        {ON_HOLD, &m_onHoldList},
        {DROPPED, &m_droppedList},
        {COMPLETED,&m_completedList}
    };

    int m_listType = WATCHING;
    int getlistType(){
        return m_listType;
    }
    void setListType(int listType){
        m_listType = listType;
        m_currentList = listMap[listType];
        emit layoutChanged ();
    }
    int m_currentShowListIndex = -1;
public:
    explicit WatchListModel(QObject *parent = nullptr): QAbstractListModel(parent){
        std::ifstream infile(".watchlist");
        if (!infile.good()) { // file doesn't exist or is corrupted
            std::ofstream outfile(".watchlist"); // create new file
            outfile << "[]"; // write [] to file
            outfile.close();
        }else if(infile.peek() == '['){
            infile>> m_jsonList;
        }else{
            m_jsonList = nlohmann::json::array ();
        }

        for (int i = 0; i < m_jsonList.size (); ++i) {
            m_list.push_back(std::make_shared<ShowResponse>(ShowResponse(m_jsonList[i])));
            switch(m_list.last ()->listType){
            case WATCHING:
                m_watchingList.push_back (m_list.last ());
                break;
            case PLANNED:
                m_plannedList.push_back (m_list.last ());
                break;
            case ON_HOLD:
                m_onHoldList.push_back (m_list.last ());
                break;
            case DROPPED:
                m_droppedList.push_back (m_list.last ());
                break;
            }
        }
    };
    ~WatchListModel() {
        for(const auto& item:m_list){

        }
        m_list.clear ();
        m_watchingList.clear ();
        m_plannedList.clear ();
        m_onHoldList.clear ();
        m_droppedList.clear ();
    }
public:
    Q_INVOKABLE void add(const ShowResponse& show, int listType = WATCHING){
        //        if(show.isInWatchList || checkInList (show))return;


        nlohmann::json showObj = nlohmann::json::object({
                                                         {"title", show.title.toStdString ()},
                                                         {"cover", show.coverUrl.toStdString ()},
                                                         {"link",show.link.toStdString ()},
                                                         {"provider",show.provider},
                                                         {"listType",listType},
                                                         {"lastWatchedIndex",show.lastWatchedIndex},
                                                         });
        m_jsonList.push_back (showObj);
        //        qDebug()<<showObj.dump();
        m_list.push_back (std::make_shared<ShowResponse>(ShowResponse(m_jsonList[m_jsonList.size()-1])));
        //        qDebug()<<showObj.dump();
        listMap[listType]->push_back (m_list.last ());
        if(m_listType == listType)emit layoutChanged ();
        save();
    }

    Q_INVOKABLE void addCurrentShow(int listType = WATCHING){
        if(Global::instance ().currentShowObject ()->isInWatchList ()){
            changeListType (getShowInList (Global::instance ().currentShowObject ()->getShow ()),listType);
            Global::instance ().currentShowObject ()->setListType (listType);
            emit layoutChanged();
            save();
            return;
        }
        add(Global::instance ().currentShowObject ()->getShow (), listType);
        Global::instance ().currentShowObject ()->setIsInWatchList (true);
        Global::instance ().currentShowObject ()->setListType (listType);
    }

    void changeListType(std::shared_ptr<ShowResponse> show, const int& to){
        if(!m_list.contains (show))return;
        if(show->listType == to) return;

        try{
            listMap[show->listType]->removeOne(show);
            show->setListType (to);
            listMap[to]->push_back(show);

            int index = m_list.indexOf (show);
            m_list.move (index,m_list.count ()-1);

            m_jsonList.push_back(m_jsonList[index]);
            m_jsonList.erase(m_jsonList.begin() + index);
            emit layoutChanged();
        }catch(QException& e){
            e.what ();
        }


    }

    Q_INVOKABLE void remove(const ShowResponse& show){
        for (int i = 0; i < m_list.size(); i++) {
            if (m_list[i]->link == show.link) {
                if(show.object){
                    show.object->setIsInWatchList(false);
                    show.object->setListType(-1);
                }
                listMap[show.listType]->removeOne (m_list[i]);
                removeAtIndex (i);
                break;
            }
        }

    }

    Q_INVOKABLE void removeAtIndex(int index){
        if (index < 0 || index >= m_list.size()) return;
        m_jsonList.erase(m_jsonList.begin() + index);
        //        delete m_list.at (index);
        m_list.removeAt (index);
        emit layoutChanged ();
        save();
    }

    Q_INVOKABLE void removeCurrentShow(){
        remove (Global::instance ().currentShowObject ()->getShow ());
    }

    Q_INVOKABLE void loadDetails(int index){
        switch(m_listType){
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
        int actualFrom = m_list.indexOf(m_currentList->at (from));
        int actualTo = m_list.indexOf (m_currentList->at (to));
        m_currentList->move (from,to);
        m_list.move(actualFrom,actualTo);
        auto element_to_move = m_jsonList[actualFrom];
        m_jsonList.erase(m_jsonList.begin() + actualFrom);
        m_jsonList.insert(m_jsonList.begin() + actualTo, element_to_move);
    }

    Q_INVOKABLE void moveEnded();;

    void update(const ShowResponse& show){
        if(!show.isInWatchList)return;
        getShowInList (show)->setLastWatchedIndex (show.getLastWatchedIndex ());
    }

    void updateCurrentShow(){
        update(Global::instance().currentShowObject ()->getShow ());
    }

    bool checkInList(const ShowResponse& show){
        if(getShowInList (show)){
            return true;
        }
        return false;

    }

    int getIndex(const QString& link){
        for (int i = 0; i < m_list.size(); i++) {
            if (m_list[i]->link == link) {
                return i;
            }
        }
        return -1;
    }

    std::shared_ptr<ShowResponse> getShowInList(const ShowResponse& show) {
        const auto iter = std::find_if(m_list.begin(), m_list.end(),
                                       [&](const std::shared_ptr<ShowResponse> ptr) { return ptr->link == show.link; });
        return iter == m_list.end() ? nullptr : *iter;
    }


public:
    int currentShowListIndex(){
        return m_currentShowListIndex;
    }
signals:
    void detailsRequested(ShowResponse show);
    void indexMoved(int from,int to);

public slots:
    bool checkCurrentShowInList(){
        auto link = Global::instance().currentShowObject ()->link ();
        for(int i = 0;i<m_list.count ();i++){
            const auto& item = m_list[i];
            if(item->link==link){
                Global::instance().currentShowObject()->setIsInWatchList(true);
                Global::instance().currentShowObject()->setLastWatchedIndex(item->lastWatchedIndex);
                Global::instance().currentShowObject()->setListType(item->listType);
                //                Global::instance().currentShowObject()->setJsonObject(m_jsonList[i]);
                m_currentShowListIndex = -1;
                return true;
            }
        }
        Global::instance().currentShowObject()->setIsInWatchList(false);
        Global::instance().currentShowObject()->setListType(-1);
        m_currentShowListIndex = -1;
        //        Global::instance().currentShowObject()->setLastWatchedIndex(-1);
        return false;
    }

    void save(){
        std::ofstream output_file(".watchlist");
        output_file << m_jsonList.dump (4);
        output_file.close();
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
