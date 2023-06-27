#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include "parsers/mediaprovider.h"
#include <QDir>
#include <QAbstractListModel>
#include "global.h"
#include <QtConcurrent>




class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentItemName READ currentItemName NOTIFY currentIndexChanged)
    Q_PROPERTY(QString showName READ showName NOTIFY showNameChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged);
public:
    explicit PlaylistModel(QObject *parent = nullptr):QAbstractListModel(parent){ };
    ~PlaylistModel(){
        delete m_historyFile;
    }

    void setOnLaunchFile(QString file){
        m_onLaunchFile = file;
    }
    QString getPlayOnLaunchFile(){
        return m_onLaunchFile;
    }
    void setOnLaunchPlaylist(QString playlist){
        loadFolder (QUrl::fromLocalFile(playlist),false);
        if(m_playlist.length () == 0){
            qWarning() << "directory has no playable items.";
            return;
        }
        m_onLaunchFile = m_playlist[this->m_currentIndex].localPath;

    }
    inline QString getOnLaunchPlaylist(){
        return m_onLaunchPlaylist;
    }
    Q_INVOKABLE void loadFolder(const QUrl& path, bool play = true);
    Q_INVOKABLE void loadSource(int index);
//Traversing the playlist
    Q_INVOKABLE void loadOffset(int offset);
    //    bool hasNextItem(){return m_currentIndex < m_playlist.size ()-1;}
    //    bool hasPrecedingItem(){return m_currentIndex>0;}
    void playNextItem(){loadOffset (1);}
    void playPrecedingItem(){loadOffset (-1);}
    void setWatchListShowItem(nlohmann::json* json){
        m_watchListShowItem = json;
    }
private:
    bool isValidIndex(int index) const{
        return index >= 0 && index < m_playlist.size ();
    }
    QString currentItemName() const{
        if(!isValidIndex(m_currentIndex))return "";
        const Episode& currentItem = m_playlist[m_currentIndex];
        QString itemName = "[%1/%2] %3";
        itemName = itemName.arg (m_currentIndex+1).arg (m_playlist.size ()).arg (currentItem.number);
        if (currentItem.title.length () > 0) {
            itemName+=". "+ currentItem.title;
        }
        return itemName;
    }
    inline int currentIndex() const{
        return m_currentIndex;
    }
    inline QString showName() const{
        return m_currentShowName;
    }
    inline bool isLoading(){
        return loading;
    }
    void loadOnlineSource(int index);
    void loadLocalSource(int index);
    void setLoading(bool b){
        loading = b;
        emit loadingChanged ();
    }

public:

private:
    MediaProvider *currentProvider;
    QString m_currentShowLink;
    QString m_currentShowName;
    QVector<Episode> m_playlist;
    int m_currentIndex = -1;
    bool online = true;
    QFile* m_historyFile;
    QFutureWatcher<void> m_watcher;
    QString m_onLaunchFile;
    QString m_onLaunchPlaylist;
    nlohmann::json* m_watchListShowItem;
    bool loading = false;
    MediaData m_currentShow;


signals:
    void loadingChanged(void);
    void sourceFetched(QString link);
    void currentIndexChanged(void);
    void updatedLastWatchedIndex();
    void showNameChanged(void);
    void encounteredError(QString error);
public slots:
    void syncList();
private:
    enum{
        TitleRole = Qt::UserRole,
        NumberRole,
        NumberTitleRole
    };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
};

#endif // PLAYLISTMODEL_H
