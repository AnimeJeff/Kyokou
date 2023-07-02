#include "watchlistmodel.h"

void WatchListModel::changeListType(const ShowData &show, int listType){
    auto showLink = show.link.toStdString ();
    auto& list = m_jsonList[show.listType];
    for(int i = 0;i<list.size ();i++){
        if(list[i]["link"] == showLink){
            m_jsonList[listType].push_back(list[i]);
            m_jsonList[show.listType].erase (list.begin() + i);
            listMap[listType]->push_back (listMap[show.listType]->at (i));
            listMap[show.listType]->removeAt (i);
            ShowManager::instance ().setListType (listType);
            emit layoutChanged();
            save();
            break;
        }
    }
}

void WatchListModel::addCurrentShow(int listType){
    auto currentShow = ShowManager::instance ().getCurrentShow ();
    if(currentShow.isInWatchList ()){
        //change the list type
        changeListType (currentShow,listType);
    }else{
        add(currentShow, listType);
        ShowManager::instance().setListType (listType);
    }
}

void WatchListModel::removeCurrentShow(){
    remove (ShowManager::instance().getCurrentShow ());
    ShowManager::instance ().setListType(-1);
}

void WatchListModel::moveEnded(){
    emit layoutChanged ();
    save();
}

bool WatchListModel::checkCurrentShowInList(){
    ShowManager::instance ().getCurrentShow ();
    if(ShowManager::instance ().getCurrentShow ().isInWatchList ()){
        qDebug()<<"Loaded from watch list";
        return true;
    }
    auto showInList = getShowInList (ShowManager::instance ().getCurrentShow ());
    if(showInList){
        qDebug() << "Found" << showInList->title << "in watch list";
        ShowManager::instance ().setLastWatchedIndex(showInList->lastWatchedIndex);
        ShowManager::instance().setListType(showInList->listType);
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
