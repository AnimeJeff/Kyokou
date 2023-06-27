#ifndef APPLICATION_H
#define APPLICATION_H

#include "models/downloadmodel.h"
#include "models/episodelistmodel.h"
#include "models/playlistmodel.h"
#include "models/searchresultsmodel.h"
#include "models/watchlistmodel.h"
#include <QAbstractListModel>

//#include "parsers/data/mediadataobject.h"
#include "parsers/data/mediadataobject.h"
#include "parsers/providers/mediaproviders/nivod.h"
#include "parsers/providers/mediaproviders/gogoanime.h"
#include "parsers/providers/mediaproviders/nineanimehq.h"
class Application : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PlaylistModel* playlistModel READ playlistModel CONSTANT)
    Q_PROPERTY(EpisodeListModel* episodeListModel READ episodeListModel CONSTANT)
    Q_PROPERTY(SearchResultsModel* showExplorer READ searchResultsModel CONSTANT)
    Q_PROPERTY(WatchListModel* watchList READ watchListModel CONSTANT)

    Q_PROPERTY(QList<MediaProvider*> providers READ providers CONSTANT)
    Q_PROPERTY(MediaProvider* currentSearchProvider READ getCurrentSearchProvider NOTIFY currentSearchProviderChanged)
    Q_PROPERTY(MediaDataObject* currentShowObject READ getCurrentShowObject CONSTANT)

    MediaDataObject currentShowObject;
    MediaDataObject* getCurrentShowObject(){
        return &currentShowObject;
    }
    MediaProvider* m_currentSearchProvider = providersMap[MediaProvider::Gogoanime] ;
    QMap<int,MediaProvider*> providersMap{{MediaProvider::Nivod,new Nivod},
//                                         {Providers::e_HuaLe,new HuaLe},
                                         {MediaProvider::Gogoanime,new Gogoanime},
                                         //        {Providers::e_NtDongMan,new NtDongMan},
                                         {MediaProvider::NineAnimeHQ,new NineanimeHQ},
                                         };

    friend PlaylistModel;
    friend SearchResultsModel;
    friend EpisodeListModel;
    friend WatchListModel;
    //todo mediamanager
    //remove mediadataobject
    //
    inline MediaProvider* getProvider(int provider) const {
        if(providersMap.contains(provider)) return providersMap[provider];
        return nullptr;
    }
    inline MediaProvider* getCurrentShowProvider() {
        return providersMap[currentShowObject.provider()];
    }
    inline MediaProvider* getCurrentSearchProvider() {
        Q_ASSERT(m_currentSearchProvider!=nullptr);
        return m_currentSearchProvider;
    }
    inline QList<MediaProvider*> providers() const {
        return providersMap.values ();
    }

    EpisodeListModel m_episodeListModel{this};
    PlaylistModel m_playlistModel{this};
    SearchResultsModel m_searchResultsModel{this};
    WatchListModel m_watchListModel{this};
    DownloadModel m_downloadModel{this};

public:
    PlaylistModel* playlistModel(){return &m_playlistModel;}
    EpisodeListModel* episodeListModel(){return &m_episodeListModel;}
    SearchResultsModel* searchResultsModel(){return &m_searchResultsModel;}
    WatchListModel* watchListModel(){return &m_watchListModel;}
public:
    Q_INVOKABLE void changeSearchProvider(int providerEnum) {
        m_currentSearchProvider = providersMap[providerEnum];
        emit currentSearchProviderChanged();
    }
    Q_INVOKABLE void loadSourceFromList(int index){
        m_playlistModel.syncList ();
        if(currentShowObject.isInWatchList ()){
            m_playlistModel.setWatchListShowItem (m_watchListModel.getShowJsonInList (currentShowObject.getShow ()));
        }
        m_playlistModel.loadSource (index);
    }

signals:
    void currentSearchProviderChanged();
    //Singleton implementation
public:
    static Application& instance()
    {
        static Application s_instance;
        return s_instance;
    }
private:
    explicit Application(QObject *parent = nullptr): QObject(parent){
        connect(&m_watchListModel,&WatchListModel::detailsRequested,&m_searchResultsModel,&SearchResultsModel::getDetails);
        connect(&m_playlistModel,&PlaylistModel::updatedLastWatchedIndex,&m_watchListModel,&WatchListModel::save);

        connect(&currentShowObject, &MediaDataObject::showChanged,&m_watchListModel,&WatchListModel::checkCurrentShowInList);
        connect(&currentShowObject, &MediaDataObject::showChanged,[&](){
            m_episodeListModel.setIsReversed(false);
            m_episodeListModel.updateLastWatchedName();
        });

        connect(&currentShowObject, &MediaDataObject::lastWatchedIndexChanged,&m_episodeListModel,&EpisodeListModel::updateLastWatchedName);


        //        m_searchResultsModel.latest (1,0);
        //        m_downloadModel.downloadM3u8 ("lmao","D:\\TV\\temp\\尖峰时刻","https://ngzx.vizcloud.co/simple/EqPFIf8QWADtjDlGha7rC5QuqFxVu_T7SkR7rqk+wYMnU94US2El_Po4w1ynT_yP+tyVRt8p/br/H2/v.m3u8","https://ngzx.vizcloud.co");
    };
    ~Application() {} // Private destructor to prevent external deletion.
    Application(const Application&) = delete; // Disable copy constructor.
    Application& operator=(const Application&) = delete; // Disable copy assignment.

private:
};

#endif // APPLICATION_H
