#pragma once
#include "Data/showdata.h"
#include "Controllers/episodelistmodel.h"

#include <QObject>

class ShowManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ getTitle NOTIFY showChanged);
    Q_PROPERTY(QString coverUrl READ getCoverUrl NOTIFY showChanged);
    Q_PROPERTY(QString description READ getDescription NOTIFY showChanged);
    Q_PROPERTY(QString releaseDate READ getReleaseDate NOTIFY showChanged);
    Q_PROPERTY(QString status READ getStatus NOTIFY showChanged);
    Q_PROPERTY(QString updateTime READ getUpdateTime NOTIFY showChanged);
    Q_PROPERTY(QString rating READ getRating NOTIFY showChanged);
    Q_PROPERTY(QString views READ getViews NOTIFY showChanged);
    Q_PROPERTY(QString genresString READ getGenresString NOTIFY showChanged);

    Q_PROPERTY(bool exists READ exists NOTIFY showChanged);
    Q_PROPERTY(bool inWatchList READ inWatchList NOTIFY listTypeChanged); //todo
    Q_PROPERTY(int listType READ getListType NOTIFY listTypeChanged)

    QString getTitle() const {return m_show.title;}
    QString getCoverUrl() const {return m_show.coverUrl;}
    QString getDescription() const {return m_show.description.isEmpty () ? "???" : m_show.description;}
    QString getReleaseDate() const {return m_show.releaseDate.isEmpty () ? "???" : m_show.releaseDate;}
    QString getUpdateTime() const {return m_show.updateTime.isEmpty () ? "???" : m_show.updateTime;}
    QString getRating() const {return m_show.score.isEmpty () ? "???" : m_show.score;}
    QString getGenresString() const {return m_show.genres.isEmpty () ? "???" : m_show.genres.join (' ');}
    QString getViews() const {return m_show.views.isEmpty () ? "???" : m_show.views;}
    QString getStatus() const {return m_show.status.isEmpty () ? "???" : m_show.status;}

    Q_PROPERTY(EpisodeListModel *episodeList READ episodeListModel CONSTANT)
private:
    inline bool exists() const { return !m_show.link.empty (); }
private:
    ShowData m_show{"", "", "", nullptr};
    EpisodeListModel m_episodeListModel{this};
    QFutureWatcher<void> m_watcher;
public:
    explicit ShowManager(QObject *parent = nullptr);
    ~ShowManager();

    ShowData &getShow() { return m_show; }
    void setShow(const ShowData &show);
    void updateLastWatchedIndex(int index);
    int correctIndex(int index) const;
    int getContinueIndex() const;

    inline PlaylistItem *getPlaylist() const { return m_show.playlist; }
    EpisodeListModel *episodeListModel() { return &m_episodeListModel; }

    inline bool inWatchList() const { return m_show.listType != -1; }
    inline int getListType() const { return m_show.listType; }
    void setListType(int listType);
signals:
    void showChanged();
    void listTypeChanged();
};
