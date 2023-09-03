#ifndef SEARCHRESULTSMODEL_H
#define SEARCHRESULTSMODEL_H

#include <QAbstractListModel>
#include "parsers/data/showdata.h"
#include "showmanager.h"
#include <QtConcurrent>
#include <tools/ErrorHandler.h>


class Application;

class SearchResultsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged);
    bool fetchingMore = false;
    QTimer timeoutTimer;
    const int timeoutDuration = 500;
    bool loading = false;
    QFutureWatcher<QVector<ShowData>> m_searchWatcher{};
    QFutureWatcher<ShowData> m_detailLoadingWatcher{};
    QVector<ShowData> m_list;
    QThread thread;
public:
    explicit SearchResultsModel(QObject *parent = nullptr)
        : QAbstractListModel(parent){

        QObject::connect(&m_searchWatcher, &QFutureWatcher<QVector<ShowData>>::finished,this, [this]() {
            setResults (m_searchWatcher.future ().result ());
        });

        QObject::connect(&ShowManager::instance(), &ShowManager::currentSearchProviderChanged,this, [this]() {
            ShowManager::instance().getCurrentSearchProvider ()->disconnect ();
            connect(ShowManager::instance().getCurrentSearchProvider (), &ShowProvider::fetchedResults, this, &SearchResultsModel::setResults);
            ShowManager::instance().getCurrentSearchProvider ()->moveToThread (&thread);
        });

        QObject::connect(&m_detailLoadingWatcher, &QFutureWatcher<ShowData>::finished,this, [this]() {
            try {
                ShowManager::instance ().setCurrentShow(m_detailLoadingWatcher.future ().result ());
                emit detailsLoaded();
            } catch (...) {
                ErrorHandler::instance().show("Failed to load details for show");
            }


            setLoading(false);
        });
    }

    Q_INVOKABLE void search(const QString& query,int page,int type);

    Q_INVOKABLE void latest(int page,int type);

    Q_INVOKABLE void popular(int page,int type);

    Q_INVOKABLE void loadDetails(int index){
        //        getDetails(m_list[index]);
        auto show = m_list[index];
        if(ShowManager::instance ().getCurrentShow () == show){
            emit detailsLoaded();
            return;
        }
        setLoading(true);
        m_detailLoadingWatcher.setFuture(QtConcurrent::run ([show,this](){
            return ShowManager::instance ().loadDetails (show);
        }));
    }

    inline bool isLoading(){
        return loading;
    }

    void setLoading(bool b){
        loading = b;
        emit loadingChanged();
    }
public slots:
    void getDetails(const ShowData& show);


public:
    Q_INVOKABLE void loadMore();;
    Q_INVOKABLE bool canLoadMore()const;
signals:

    void fetchMoreResults(void);
    void detailsLoaded(void);
    void sourceFetched(QString link);
    void postItemsAppended(void);
    void loadingChanged(void);
private slots:
    void setResults(QVector<ShowData> results){
        if(fetchingMore){
            const int oldCount = m_list.count();
            beginInsertRows(QModelIndex(), oldCount, oldCount + results.count() - 1);
            m_list.reserve(oldCount + results.count());
            m_list += std::move(results);
            endInsertRows();
            fetchingMore = false;
        }else{
            m_list.reserve(results.count());
            m_list.swap(results);
            emit layoutChanged ();
        }
        loading=false;
        emit loadingChanged();
    }

    // QAbstractItemModel interface
private:
    enum{
        TitleRole = Qt::UserRole,
        CoverRole
    };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override{
        QHash<int, QByteArray> names;
        names[TitleRole] = "title";
        names[CoverRole] = "cover";
        return names;
    };

};



#endif // SEARCHRESULTSMODEL_H
