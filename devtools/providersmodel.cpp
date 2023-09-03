#include "episodelistmodel.h"
#include "application.h"



void EpisodeListModel::updateLastWatchedName(){
    continueEpisodeName = "";
    auto currentShow = ShowManager::instance ().getCurrentShow ();
    continueIndex = currentShow.getLastWatchedIndex ();
    int totalEpisodes = currentShow.episodes.count ();
//    qDebug()<<continueIndex <<totalEpisodes;
    if(continueIndex >= 0 && continueIndex < totalEpisodes){
        if(continueIndex==totalEpisodes-2)continueIndex++;
        Episode lastWatchedEpisode = currentShow.episodes[continueIndex];
        continueEpisodeName = QString::number (lastWatchedEpisode.number);
        if(!(lastWatchedEpisode.title.isEmpty () || lastWatchedEpisode.title.toInt () == lastWatchedEpisode.number)){
            continueEpisodeName += "\n" + lastWatchedEpisode.title;
        }

    }
    emit continueIndexChanged();
}

int EpisodeListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return ShowManager::instance().getCurrentShow ().episodes.count ();
}

QVariant EpisodeListModel::data(const QModelIndex &index, int role) const{

    if (!index.isValid())
        return QVariant();
    int i = index.row();
    if(isReversed){
        i = ShowManager::instance().getCurrentShow ().episodes.count () - i - 1;
    }
    const Episode& episode = ShowManager::instance().getCurrentShow ().episodes.at (i);

    switch (role) {
    case TitleRole:
        return episode.title.isEmpty () ? QVariant() : episode.title;
    case NumberRole:
        return episode.number;
    case FullTitleRole:
        return episode.getFullTitle();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> EpisodeListModel::roleNames() const{
    QHash<int, QByteArray> names;
    names[TitleRole] = "title";
    names[NumberRole] = "number";
    return names;
}


