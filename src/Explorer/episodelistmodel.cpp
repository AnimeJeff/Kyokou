#include "episodelistmodel.h"
#include "Explorer/showmanager.h"
QString EpisodeListModel::getContinueEpisodeName()
{
    const auto* playlist = ShowManager::instance ().getCurrentShow ().getPlaylist ();
    if(!playlist || continueIndex < 0 || continueIndex >= playlist->count ()) return "";
    const PlaylistItem *episode = playlist->at(continueIndex);
    return episode->name.isEmpty () ? QString::number (episode->number) : episode->number < 0 ? episode->name : QString::number (episode->number) + "\n" + episode->name;
}

void EpisodeListModel::updateLastWatchedName()
{
    if(const auto* playlist = ShowManager::instance ().getCurrentShow ().getPlaylist ())
    {
        continueIndex = ShowManager::instance ().getLastWatchedIndex ();
        if(continueIndex == playlist->count () - 2) ++continueIndex;
        qDebug() << "continue index" << continueIndex << "count" << playlist->count ();
        emit continueIndexChanged();
    }
}

int EpisodeListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    const PlaylistItem* playlist = ShowManager::instance ().getCurrentShow ().getPlaylist();
    if (!playlist) return 0;
    return playlist->count ();
}

QVariant EpisodeListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    const PlaylistItem* playlist = ShowManager::instance ().getCurrentShow ().getPlaylist();
    if (!playlist) return QVariant();
    int i = isReversed ? playlist->count () - index.row() - 1 : index.row();
    const PlaylistItem* episode = playlist->at(i);

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

QHash<int, QByteArray> EpisodeListModel::roleNames() const{
    QHash<int, QByteArray> names;
    names[TitleRole] = "title";
    names[NumberRole] = "number";
    names[FullTitleRole] = "fullTitle";
    return names;
}
