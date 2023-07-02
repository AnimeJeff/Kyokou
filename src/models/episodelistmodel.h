#ifndef EPISODELISTMODEL_H
#define EPISODELISTMODEL_H

#include <Global.h>
#include <QAbstractListModel>

//#include "parsers/data/episode.h"
//Todo list that holds episodes
//connect signal from showparser to slot
class EpisodeListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool reversed READ getIsReversed WRITE setIsReversed NOTIFY reversedChanged);
    Q_PROPERTY(QString continueEpisodeName READ getContinueEpisodeName NOTIFY continueIndexChanged);
    Q_PROPERTY(int continueIndex READ getContinueIndex NOTIFY continueIndexChanged);
    enum{
        TitleRole = Qt::UserRole,
        NumberRole,
        FullTitleRole
    };
    bool isReversed = false;
    QString continueEpisodeName;
    int continueIndex;

    QString getContinueEpisodeName(){
        return continueEpisodeName;
    }
    int getContinueIndex(){
        return continueIndex;
    }
public slots:
    void updateLastWatchedName();
signals:
    void continueIndexChanged(void);
    void reversedChanged(void);
public:
    void setIsReversed(bool isReversed){
        this->isReversed = isReversed;
        emit layoutChanged ();
        emit reversedChanged();
    }
    bool getIsReversed() const{
        return isReversed;
    }
    explicit EpisodeListModel(QObject *parent = nullptr)
        : QAbstractListModel(parent){

    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;;

    QHash<int, QByteArray> roleNames() const override;;
private:
};

#endif // EPISODELISTMODEL_H
