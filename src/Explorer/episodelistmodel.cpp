#include "episodelistmodel.h"
#include "application.h" //TODO



void EpisodeListModel::updateLastWatchedName()
{
    continueEpisodeName = "";
    auto currentShow = ShowManager::instance ().getCurrentShow ();
    continueIndex = currentShow.getLastWatchedIndex ();
    if(!currentShow.playlist) return;
    int totalEpisodes = currentShow.playlist->count ();
//    qDebug()<<continueIndex <<totalEpisodes;
    if(continueIndex >= 0 && continueIndex < totalEpisodes)
    {
        if(continueIndex==totalEpisodes-2) continueIndex++;
        auto lastWatchedEpisode =
            currentShow.playlist->at(continueIndex); //check
        continueEpisodeName = QString::number (lastWatchedEpisode->number);

        if(!(lastWatchedEpisode->name.isEmpty () ||
              lastWatchedEpisode->name.toInt () == lastWatchedEpisode->number)){
            continueEpisodeName += "\n" + lastWatchedEpisode->name;
        }

    }
    emit continueIndexChanged();
}

int EpisodeListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    auto playlist = ShowManager::instance().getCurrentShow ().playlist;
    return playlist ? playlist->count () : 0;
}

QVariant EpisodeListModel::data(const QModelIndex &index, int role) const{

    if (!index.isValid())
        return QVariant();
    int i = index.row();
    if(isReversed)
    {
        i = ShowManager::instance().getCurrentShow ().playlist->count () - i - 1;
    }
    const auto& episode = ShowManager::instance().getCurrentShow().playlist->getChildren()->at(i);

    switch (role) {
    case TitleRole:
        return episode->name;
    case NumberRole:
        return episode->number;
    case FullTitleRole:
        return episode->fullName;
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


