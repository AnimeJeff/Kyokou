
#ifndef SHOWMANAGER_H
#define SHOWMANAGER_H

#include "parsers/providers/showproviders/tangrenjie.h"
#include "parsers/providers/showproviders/gogoanime.h"
#include "parsers/providers/showproviders/nineanimehq.h"
#include "parsers/providers/showproviders/nivod.h"
#include "parsers/providers/showproviders/haitu.h"

#include <QAbstractListModel>
#include <QObject>


class ShowManager: public QAbstractListModel
{
    Q_OBJECT
    //    Q_PROPERTY(ShowProvider* currentSearchProvider READ getCurrentSearchProvider NOTIFY currentSearchProviderChanged)
    Q_PROPERTY(ShowData currentShow READ getCurrentShow NOTIFY currentShowChanged)
    Q_PROPERTY(bool hasCurrentShow READ hasCurrentShow NOTIFY currentShowChanged)
    Q_PROPERTY(QString currentSearchProviderName READ currentSearchProviderName NOTIFY currentSearchProviderChanged)
    //info page list type is bound to this
    Q_PROPERTY(int currentShowListType READ getCurrentShowListType NOTIFY listTypeChanged)
    Q_PROPERTY(int currentShowLastWatchedIndex READ getLastWatchedIndex NOTIFY lastWatchedIndexChanged)
    Q_PROPERTY(bool currentShowIsInWatchList READ isInWatchList NOTIFY listTypeChanged)

    QMap<int,ShowProvider*> providersMap{
                                           {ShowProvider::NIVOD,new Nivod},
                                           {ShowProvider::GOGOANIME,new Gogoanime},
                                           {ShowProvider::TANGRENJIE,new Tangrenjie},
                                           {ShowProvider::HAITU,new Haitu},
                                           };
    QVector<ShowProvider*> providers = providersMap.values ();
    ShowProvider* m_currentSearchProvider = providersMap[ShowProvider::GOGOANIME];
    ShowData currentShow;
    QString currentSearchProviderName(){
        return m_currentSearchProvider->name ();
    }

public:
    ShowData loadDetails(const ShowData& show){
        if(show.provider==-1){
            //lazy show
            ShowData loadedShow {show};
            loadedShow.episodes.emplaceBack (Episode{ 1,show.link,show.title });
            return loadedShow;
        }
        auto provider = getProvider (show.provider);
        if(provider) {
            qDebug()<<"Loading details for" << show.title << "with" << provider->name () << "using the link:" << show.link;
            try{
                auto loadedShow = provider->loadDetails (show);
                qDebug()<<"Successfully loaded details for" << loadedShow.title;
                return loadedShow;
            }catch(const QException& e){
                qDebug() << e.what ();
            }catch(std::exception& e){
                qDebug()<<e.what ();
                std::cout << e.what();
            }catch(...){
                qDebug()<<"Failed to load" << show.title;
            }
            return show;
        }
        qDebug()<<"Unable to find a provider for provider enum" << show.provider;
        return show;
    }
    bool hasCurrentShow(){
        return !currentShow.title.isEmpty ();
    }
    ShowData getCurrentShow() const {
        return currentShow;
    };
    void setCurrentShow(const ShowData& show){
        currentShow = show;
        emit currentShowChanged();
    }
    bool isInWatchList(){
        return currentShow.isInWatchList ();
    }

    ShowProvider* getProvider(int provider) const {
        if(providersMap.contains(provider)) return providersMap[provider];
        return nullptr;
    }
    ShowProvider* getCurrentShowProvider() {
        return providersMap[currentShow.provider];
    }
    ShowProvider* getCurrentSearchProvider() const {
        Q_ASSERT(m_currentSearchProvider!=nullptr);
        return m_currentSearchProvider;
    }

    Q_INVOKABLE void changeSearchProvider(int providerEnum) {
        m_currentSearchProvider = providersMap[providerEnum];
        providers.move (providers.indexOf (m_currentSearchProvider),0);
        emit currentSearchProviderChanged();
    }
    void setLastWatchedIndex(int index){
        currentShow.setLastWatchedIndex (index);
        emit lastWatchedIndexChanged ();
    }
    int getLastWatchedIndex() const {
        return currentShow.getLastWatchedIndex ();
    }
    void setListType(int listType){
        currentShow.setListType (listType);
        emit listTypeChanged ();
    }
    int getCurrentShowListType(){
        return currentShow.getListType ();
    }
signals:
    void currentSearchProviderChanged();
    void currentShowChanged();
    void lastWatchedIndexChanged();
    void listTypeChanged();
public:
    explicit ShowManager() {
        providers.move (providers.indexOf (m_currentSearchProvider),0);
    }
    static ShowManager& instance()
    {
        static ShowManager instance;
        return instance;
    }
    ~ShowManager() {
        for(auto& p:providers){
            delete p;
        }
    } // Private destructor to prevent external deletion.
    ShowManager(const ShowManager&) = delete; // Disable copy constructor.
    ShowManager& operator=(const ShowManager&) = delete; // Disable copy assignment.

    // QAbstractItemModel interface
public:
    enum{
        NameRole = Qt::UserRole,
        IconRole,
        EnumRole
    };
    int rowCount(const QModelIndex &parent) const override{
        return providers.count ();
    };
    QVariant data(const QModelIndex &index, int role) const override{
        if (!index.isValid())
            return QVariant();
        ShowProvider* provider = providers.at(index.row ());
        switch (role) {
        case NameRole:
            return provider->name ();
            break;
        case IconRole:
            break;
        case EnumRole:
            return provider->providerEnum ();
            break;
        default:
            break;
        }
        return QVariant();
    };
    QHash<int, QByteArray> roleNames() const override{
        QHash<int, QByteArray> names;
        names[NameRole] = "text";
        names[IconRole] = "icon";
        names[EnumRole] = "providerEnum";
        return names;
    };
};

#endif // SHOWMANAGER_H
