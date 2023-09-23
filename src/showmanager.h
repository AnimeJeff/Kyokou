
#ifndef SHOWMANAGER_H
#define SHOWMANAGER_H

#include "parsers/providers/showproviders/tangrenjie.h"
#include "parsers/providers/showproviders/gogoanime.h"
#include "parsers/providers/showproviders/nivod.h"
#include "parsers/providers/showproviders/haitu.h"

#include <QAbstractListModel>
#include <QObject>

#include <parsers/providers/showproviders/allanime.h>


class ShowManager: public QAbstractListModel
{
    Q_OBJECT
    //    Q_PROPERTY(ShowProvider* currentSearchProvider READ getCurrentSearchProvider NOTIFY currentSearchProviderChanged)
    Q_PROPERTY(ShowData currentShow READ getCurrentShow NOTIFY currentShowChanged)
    Q_PROPERTY(bool hasCurrentShow READ hasCurrentShow NOTIFY currentShowChanged) //info page cover

    // ShowData is not a QObject so these data which are mutable must be provided by this class
    Q_PROPERTY(int currentShowListType READ getCurrentShowListType NOTIFY listTypeChanged)
    Q_PROPERTY(int currentShowLastWatchedIndex READ getLastWatchedIndex NOTIFY lastWatchedIndexChanged)
    Q_PROPERTY(bool currentShowIsInWatchList READ isInWatchList NOTIFY listTypeChanged)


    ShowData currentShow;
public:
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

    std::shared_ptr<ShowProvider> getProvider(int provider) const {
        if(providersEnumHashMap.contains(provider)) return providersEnumHashMap[provider];
        return nullptr;
    }
    std::shared_ptr<ShowProvider> getCurrentShowProvider() {
        return providersEnumHashMap[currentShow.provider];
    }
    std::shared_ptr<ShowProvider> getCurrentSearchProvider() const {
        Q_ASSERT(m_currentSearchProvider!=nullptr);
        return m_currentSearchProvider;
    }
    Q_INVOKABLE void changeSearchProvider(int index) {
        m_currentSearchProvider = providers.at (index);
        providers.move (index,providers.size ()-1);
        emit layoutChanged();
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
signals:
    void currentShowChanged();
    void lastWatchedIndexChanged();
    void listTypeChanged();

public:
    static ShowManager& instance()
    {
        static ShowManager instance;
        return instance;
    }
private:
    QHash<QString,std::shared_ptr<ShowProvider>> providersHashMap;
    QHash<int,std::shared_ptr<ShowProvider>> providersEnumHashMap; //remove in future

    QVector<std::shared_ptr<ShowProvider>> providers;
    std::shared_ptr<ShowProvider> m_currentSearchProvider;
    QString currentSearchProviderName(){
        return m_currentSearchProvider->name ();
    }
    explicit ShowManager() {
        providers =
            {
             std::make_shared<Nivod> (),
             std::make_shared<Gogoanime> (),
             std::make_shared<Tangrenjie> (),
             std::make_shared<Haitu> (),
             std::make_shared<AllAnime> (),
             };

        for (const auto& provider:providers)
        {
            providersHashMap.insert(provider->name (),provider);
            providersEnumHashMap.insert(provider->providerEnum (),provider);
        }
        m_currentSearchProvider = providersHashMap["泥巴影院"];
        providers.move (providers.indexOf (m_currentSearchProvider),providers.size ()-1);
    }
    ~ShowManager() {

    }
    ShowManager(const ShowManager&) = delete;
    ShowManager& operator=(const ShowManager&) = delete;

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
        auto provider = providers.at(index.row ());
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
//        names[IconRole] = "icon";
//        names[EnumRole] = "providerEnum";
        return names;
    };
};

#endif // SHOWMANAGER_H
