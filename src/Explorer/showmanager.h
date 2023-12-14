#pragma once
#include <QAbstractListModel>
#include <QObject>
#include "Explorer/Data/showdata.h"
#include "Providers/showprovider.h"


class ShowProvider;
class ShowManager: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(ShowData currentShow READ getCurrentShow NOTIFY currentShowChanged)
    Q_PROPERTY(bool hasCurrentShow READ hasCurrentShow NOTIFY currentShowChanged) //info page cover
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    // ShowData is not a QObject so these dynamic data which are mutable must be provided by this proxy class
    Q_PROPERTY(int currentShowListType READ getCurrentShowListType NOTIFY listTypeChanged)
    Q_PROPERTY(bool currentShowIsInWatchList READ isInWatchList NOTIFY listTypeChanged)
    Q_PROPERTY(int currentShowLastWatchedIndex READ getLastWatchedIndex NOTIFY lastWatchedIndexChanged)
    Q_PROPERTY(QList<int> availableShowTypes READ getAvailableShowTypes NOTIFY searchProviderChanged)

    ShowData currentShow{"Undefined","","",""};
private:
    explicit ShowManager();
    ~ShowManager();
    ShowManager(const ShowManager&) = delete;
    ShowManager& operator=(const ShowManager&) = delete;

    QFutureWatcher<void> m_watcher {};

    bool m_loading = false;
    bool isLoading(){return m_loading;}
    void setLoading(bool loading)
    {
        m_loading = loading;
        emit loadingChanged();
    }
    QTimer m_timeoutTimer {this};
    QString m_cancelReason;

    QList<int> getAvailableShowTypes()
    {
        return m_currentSearchProvider->getAvailableTypes();
    }

public:
    bool hasCurrentShow()
    {
        return !currentShow.link.empty ();
    }
    const ShowData& getCurrentShow()
    {
        return currentShow;
    };
    void setCurrentShow(const ShowData& show);
    bool isInWatchList()
    {
        return currentShow.isInWatchList ();
    }

    ShowProvider* getProvider(const QString& provider)
    {
        if (providersHashMap.contains(provider))
            return providersHashMap[provider];
        return nullptr;
    }
    ShowProvider* getCurrentShowProvider()
    {
        return providersHashMap[currentShow.provider];
    }
    ShowProvider* getCurrentSearchProvider() const
    {
        Q_ASSERT(m_currentSearchProvider != nullptr);
        return m_currentSearchProvider;
    }
    void setLastWatchedIndex(int index);
    int getLastWatchedIndex() const;
    void setListType(int listType);
    int getCurrentShowListType() const;


signals:
    void currentShowChanged(void);
    void lastWatchedIndexChanged(void);
    void listTypeChanged(void);
    void loadingChanged(void);
    void searchProviderChanged(void);
public slots:
    void cancel();
    void changeSearchProvider(int index);
public:
    static ShowManager& instance()
    {
        static ShowManager instance;
        return instance;
    }
private:
    QHash<QString, ShowProvider*> providersHashMap;
    QList<ShowProvider*> providers;
    ShowProvider* m_currentSearchProvider;
    QString currentSearchProviderName();
private:
    enum
    {
        NameRole = Qt::UserRole,
        IconRole,
    };
    int rowCount(const QModelIndex &parent) const override;;
    QVariant data(const QModelIndex &index, int role) const override;;
    QHash<int, QByteArray> roleNames() const override;;
};

