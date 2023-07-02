#ifndef SHOWDATAOBJECT_H
#define SHOWDATAOBJECT_H

#include <QObject>
#include "showdata.h"

class ShowDataObject:public QObject{
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY showChanged);
    Q_PROPERTY(QString coverUrl READ coverUrl NOTIFY showChanged);
    Q_PROPERTY(QString desc READ desc NOTIFY showChanged);
    Q_PROPERTY(QString year READ year NOTIFY showChanged);
    Q_PROPERTY(QString status READ status NOTIFY showChanged);
    Q_PROPERTY(QString updateTime READ updateTime NOTIFY showChanged);
    Q_PROPERTY(QString rating READ rating NOTIFY showChanged);
    Q_PROPERTY(QString views READ views NOTIFY showChanged);
    Q_PROPERTY(QString genresString READ genresString NOTIFY showChanged);
    Q_PROPERTY(bool isInWatchList READ isInWatchList NOTIFY showChanged NOTIFY showPropertyChanged);
    Q_PROPERTY(int lastWatchedIndex READ lastWatchedIndex NOTIFY showChanged NOTIFY showPropertyChanged NOTIFY lastWatchedIndexChanged);
    Q_PROPERTY(bool hasShow READ hasShow NOTIFY showChanged);
    Q_PROPERTY(int listType READ listType NOTIFY listTypeChanged);

    ShowData show;
public:
    ShowDataObject(QObject* parent = nullptr) : QObject(parent) {}
    inline void setLastWatchedIndex(int index) {
        show.setLastWatchedIndex (index);
        emit showPropertyChanged();
        emit lastWatchedIndexChanged();
    }

    inline void setListType(int listType) {
        show.setListType(listType);
        emit listTypeChanged();

    }
    inline void setShow(const ShowData& show) {
        this->show = show;
//        this->show.object = this;
        emit showChanged();
    }
    inline void setJsonObject(nlohmann::json& jsonobj){
        this->show.setJsonObject(jsonobj);
    }
    inline ShowData getShow() const {return show;}

public:
    inline QString title() const {return show.title;}
    inline QString coverUrl() const {return show.coverUrl;}
    inline bool hasShow() const {return !show.title.isEmpty ();}
    inline QString desc() const {return show.description;}
    inline QString year() const {return show.releaseDate;}
    inline QString updateTime() const {return show.updateTime;}
    inline QString rating() const {return show.rating;}
    inline QString views() const {return show.views;}
    inline QString status() const {return show.status;}
    inline QString genresString() const {return show.genres.join (' ');}
    inline int lastWatchedIndex() const {return show.lastWatchedIndex;}
    inline bool isInWatchList() const {return show.isInWatchList();}
    inline QString link() const {return show.link;}
    inline QVector<Episode> episodes() const {return show.episodes;}
    inline int listType() const {return show.listType;}
    inline int provider() const {return show.provider;}
signals:
    void showChanged(void);
    void showPropertyChanged(void);
    void lastWatchedIndexChanged(void);
    void listTypeChanged(void);
};

#endif // SHOWDATAOBJECT_H
