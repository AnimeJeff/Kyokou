#include "watchlistmodel.h"


void WatchListModel::moveEnded(){
    emit layoutChanged ();
    save();
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

//    if (index.row() < WATCHING || index.row() > DROPPED)return;


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
