#ifndef SHOWMANAGER_H
#define SHOWMANAGER_H
#include <QAbstractListModel>
#include <QObject>

#include "Providers/tangrenjie.h"
#include "Providers/gogoanime.h"
#include "Providers/nivod.h"
#include "Providers/haitu.h"
#include "Providers/allanime.h"

#include "Explorer/Data/showdata.h"
#include "Providers/testprovider.h"

class ShowManager: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(ShowData currentShow READ getCurrentShow NOTIFY currentShowChanged)
    Q_PROPERTY(bool hasCurrentShow READ hasCurrentShow NOTIFY currentShowChanged) //info page cover

    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)

    // ShowData is not a QObject so these data which are mutable must be provided by this proxy class
    Q_PROPERTY(int currentShowListType READ getCurrentShowListType NOTIFY listTypeChanged)
    Q_PROPERTY(int currentShowLastWatchedIndex READ getLastWatchedIndex NOTIFY lastWatchedIndexChanged)
    Q_PROPERTY(bool currentShowIsInWatchList READ isInWatchList NOTIFY listTypeChanged)

    ShowData currentShow{"Undefined","xasdd","dsads",-1};
private:
    explicit ShowManager()
    {
        providers =
            {
                std::make_shared<Nivod> (),
                std::make_shared<Gogoanime> (),
                std::make_shared<Tangrenjie> (),
                std::make_shared<Haitu> (),
#ifdef QT_DEBUG
                std::make_shared<TestProvider> ()
#endif \
                //             std::make_shared<AllAnime> (),
            };

        for (const auto& provider:providers)
        {
            providersHashMap.insert(provider->name (),provider);
            providersEnumHashMap.insert(provider->providerEnum (),provider);
        }

#ifdef QT_DEBUG
        m_currentSearchProvider = providersHashMap["TestProvider"];
#else
        m_currentSearchProvider = providersHashMap["泥巴影院"];
#endif
        providers.move (providers.indexOf (m_currentSearchProvider),providers.size ()-1);
        QObject::connect(&m_detailLoadingWatcher, &QFutureWatcher<ShowData>::finished, this, [this]() {
            try
            {
                currentShow = m_detailLoadingWatcher.future ().result ();
                emit detailsLoaded();
            }
            catch (...)
            {
                ErrorHandler::instance().show("Failed to load details for show");
            }
            setLoading(false);
        });
    }
    ~ShowManager() {

    }
    ShowManager(const ShowManager&) = delete;
    ShowManager& operator=(const ShowManager&) = delete;

    QFutureWatcher<ShowData> m_detailLoadingWatcher{};

    bool m_loading = false;
    bool isLoading(){return m_loading;}
    void setLoading(bool loading)
    {
        m_loading = loading;
        emit loadingChanged();
    }

public:
    bool hasCurrentShow()
    {
        return !currentShow.link.empty ();
    }
    ShowData getCurrentShow() const
    {
        return currentShow;
    };
    void setCurrentShow(const ShowData& show)
    {
        if(ShowManager::instance ().getCurrentShow ().link == show.link)
        {
            emit detailsLoaded();
            return;
        }
        setLoading(true);
        m_detailLoadingWatcher.setFuture(
            QtConcurrent::run (
                [show,this]()
                {
                    return loadDetails (show);
                }));
    }

    bool isInWatchList()
    {
        return currentShow.isInWatchList ();
    }

    std::shared_ptr<ShowProvider> getProvider(int provider)
    {
        if(providersEnumHashMap.contains(provider)) return providersEnumHashMap[provider];
        return nullptr;
    }
    std::shared_ptr<ShowProvider> getCurrentShowProvider()
    {
        return providersEnumHashMap[currentShow.provider];
    }
    std::shared_ptr<ShowProvider> getCurrentSearchProvider() const
    {
        Q_ASSERT(m_currentSearchProvider!=nullptr);
        return m_currentSearchProvider;
    }
    Q_INVOKABLE void changeSearchProvider(int index)
    {
        m_currentSearchProvider = providers.at (index);
        providers.move (index,providers.size ()-1);
        emit layoutChanged();
    }

    void setLastWatchedIndex(int index)
    {
        currentShow.setLastWatchedIndex (index);
        emit lastWatchedIndexChanged ();
    }
    int getLastWatchedIndex() const
    {
        return currentShow.getLastWatchedIndex ();
    }
    void setListType(int listType)
    {

        currentShow.setListType (listType);
        emit listTypeChanged ();
    }
    int getCurrentShowListType()
    {
        return currentShow.getListType ();
    }
    ShowData loadDetails(const ShowData& show)
    {
        if(show.provider == -1){
            //lazy show
            ShowData loadedShow {show};
            //            loadedShow.episodes.emplaceBack (Episode{ 1,show.link,show.title });
            //todo playlistitem
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
    void loadingChanged();
    void detailsLoaded();
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
