#include "episodelistmodel.h"



int EpisodeListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    if (!m_playlist) return 0;
    return m_playlist->count ();
}

QVariant EpisodeListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (!m_playlist) return QVariant();
    int i = m_isReversed ? m_playlist->count () - index.row() - 1 : index.row();
    const PlaylistItem* episode = m_playlist->at(i);

    switch (role)
    {
    case TitleRole:
        return episode->name;
    case NumberRole:
        return episode->number;
    case FullTitleRole:
        return episode->getFullName();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> EpisodeListModel::roleNames() const {
    QHash<int, QByteArray> names;
    names[TitleRole] = "title";
    names[NumberRole] = "number";
    names[FullTitleRole] = "fullTitle";
    return names;
}
