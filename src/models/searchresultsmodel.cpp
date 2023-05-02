#include "searchresultsmodel.h"




int SearchResultsModel::rowCount(const QModelIndex &parent) const{
    if (parent.isValid())
        return 0;
    return m_List.count ();
}

QVariant SearchResultsModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();

    auto show = m_List.at (index.row ());

    switch (role) {
    case TitleRole:
        return show.title;
        break;
    case CoverRole:
        return show.coverUrl;
        break;
    default:
        break;
    }
    return QVariant();
}
