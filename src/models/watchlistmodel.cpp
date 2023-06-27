#include "watchlistmodel.h"
#include "application.h"

void WatchListModel::addCurrentShow(int listType){
    if(Application::instance().currentShowObject.isInWatchList ()){
        //change the list type
        auto show = Application::instance().currentShowObject.getShow ();
        auto showLink = show.link.toStdString ();
        auto& list = m_jsonList[show.listType];
        for(int i = 0;i<list.size ();i++){
            if(list[i]["link"] == showLink){
                m_jsonList[listType].push_back(list[i]);
                m_jsonList[show.listType].erase (list.begin() + i);
                listMap[listType]->push_back (listMap[show.listType]->at (i));
                listMap[show.listType]->removeAt (i);
                Application::instance().currentShowObject.setListType (listType);
                emit layoutChanged();
                save();
                break;
            }
        }
    }else{
        add(Application::instance().currentShowObject.getShow (), listType);
        Application::instance().currentShowObject.setListType (listType);
    }
}

void WatchListModel::removeCurrentShow(){
    remove (Application::instance().currentShowObject.getShow ());
    Application::instance().currentShowObject.setListType(-1);
}

void WatchListModel::moveEnded(){
//    emit layoutChanged ();
    save();
}

bool WatchListModel::checkCurrentShowInList(){
    if(Application::instance().currentShowObject.isInWatchList ()){
        qDebug()<<"Loaded from watch list";
        return true;
    }
    auto showInList = getShowInList (Application::instance().currentShowObject.getShow ());
    if(showInList){
        qDebug()<<"Found" << showInList->title << "in watch list";
        Application::instance().currentShowObject.setLastWatchedIndex(showInList->lastWatchedIndex);
        Application::instance().currentShowObject.setListType(showInList->listType);
        return true;
    }
    return false;
}

int WatchListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_currentList->count ();
}

QVariant WatchListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case TitleRole:
        return m_currentList->at(index.row ())->title;
        break;
    case CoverRole:
        return m_currentList->at(index.row ())->coverUrl;
        break;
    default:
        break;
    }
    return QVariant();
}

QHash<int, QByteArray> WatchListModel::roleNames() const{
    QHash<int, QByteArray> names;
    names[TitleRole] = "title";
    names[CoverRole] = "cover";
    return names;
}
