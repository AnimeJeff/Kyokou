
#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H

#include "parsers/mediaprovider.h"
#include "parsers/providers/mediaproviders/gogoanime.h"
#include "parsers/providers/mediaproviders/nineanimehq.h"
#include "parsers/providers/mediaproviders/nivod.h"
#include <QObject>





class MediaManager: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<MediaProvider*> providers READ providers CONSTANT)
    Q_PROPERTY(MediaProvider* currentSearchProvider READ getCurrentSearchProvider NOTIFY currentSearchProviderChanged)
    Q_PROPERTY(MediaDataObject* currentShowObject READ getCurrentShowObject CONSTANT)

    //    MediaDataObject currentShowObject;
    //    MediaDataObject* getCurrentShowObject(){
    //        return &currentShowObject;
    //    }
    MediaData currentMedia;

    QMap<int,MediaProvider*> providersMap{{MediaProvider::Nivod,new Nivod},
                                            //{Providers::e_HuaLe,new HuaLe},
                                            {MediaProvider::Gogoanime,new Gogoanime},
                                            // {Providers::e_NtDongMan,new NtDongMan},
                                            {MediaProvider::NineAnimeHQ,new NineanimeHQ},
                                            };
    MediaProvider* m_currentSearchProvider = providersMap[MediaProvider::Gogoanime] ;
public:
    MediaProvider* getProvider(int provider) const {
        if(providersMap.contains(provider)) return providersMap[provider];
        return nullptr;
    }
    MediaProvider* getCurrentShowProvider() {
        return providersMap[currentShowObject.provider()];
    }
    MediaProvider* getCurrentSearchProvider() {
        Q_ASSERT(m_currentSearchProvider!=nullptr);
        return m_currentSearchProvider;
    }
    QList<MediaProvider*> providers() const {
        return providersMap.values ();
    }
    Q_INVOKABLE void changeSearchProvider(int providerEnum) {
        m_currentSearchProvider = providersMap[providerEnum];
        emit currentSearchProviderChanged();
    }
signals:
     void currentSearchProviderChanged();
public:
    explicit MediaManager() {}
    static MediaManager& instance()
    {
        static MediaManager instance;
        return instance;
    }
    ~MediaManager() {} // Private destructor to prevent external deletion.
    MediaManager(const MediaManager&) = delete; // Disable copy constructor.
    MediaManager& operator=(const MediaManager&) = delete; // Disable copy assignment.
};

#endif // MEDIAMANAGER_H
