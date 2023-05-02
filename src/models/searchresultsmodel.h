#ifndef SEARCHRESULTSMODEL_H
#define SEARCHRESULTSMODEL_H

#include <QAbstractListModel>
#include <parsers/showresponse.h>
#include <parsers/showparser.h>
#include <global.h>
#include <QtConcurrent>
#include <parsers/providers/gogoanime.h>
#include <parsers/providers/nivod.h>


class SearchResultsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged);
    bool fetchingMore = false;
    QTimer timeoutTimer;
    const int timeoutDuration = 500;
    bool loading = false;
    QFutureWatcher<QVector<ShowResponse>> m_searchWatcher{};
    QFutureWatcher<ShowResponse> m_detailLoadingWatcher{};
    QVector<ShowResponse> m_List;
public:
    explicit SearchResultsModel(QObject *parent = nullptr)
        : QAbstractListModel(parent){
//        timeoutTimer.setSingleShot(true);
        QObject::connect(&m_searchWatcher, &QFutureWatcher<QVector<ShowResponse>>::finished,this, [&]() {
            QVector<ShowResponse> results = m_searchWatcher.future ().result ();
            if(fetchingMore){
                const int oldCount = m_List.count();
                beginInsertRows(QModelIndex(), oldCount, oldCount + results.count() - 1);
                m_List.reserve(oldCount + results.count());
                m_List += std::move(results);
                endInsertRows();
                fetchingMore = false;
            }else{
                m_List.reserve(results.count());
                m_List.swap(results);
                emit layoutChanged ();
            }
            loading=false;
            emit loadingChanged();
        });
        QObject::connect(&m_detailLoadingWatcher, &QFutureWatcher<ShowResponse>::finished,this, [&]() {
            Global::instance().currentShowObject ()->setShow (m_detailLoadingWatcher.future ().result ());
            emit detailsLoaded();
            loading = false;
            emit loadingChanged();
        });
        latest (1,0);
    }

    Q_INVOKABLE void search(const QString& query,int page,int type){
        loading=true;
        emit loadingChanged();
        m_searchWatcher.setFuture(QtConcurrent::run ([&](){
            try{
                return Global::instance ().getCurrentSearchProvider ()->search (query,page,type);
            }catch(const std::exception& e){
                qCritical() << e.what ();
            }
            return QVector<ShowResponse>();
        }));
    }

    Q_INVOKABLE void latest(int page,int type){
        loading = true;
        emit loadingChanged();
        //        m_searchWatcher.setFuture (QtConcurrent::run (&ShowParser::latest,Global::instance ().getCurrentSearchProvider (),page,type));
        m_searchWatcher.setFuture(QtConcurrent::run ([&](){
            try{
                return Global::instance ().getCurrentSearchProvider ()->latest (page,type);
            }catch(const std::exception& e){
                qCritical() << e.what ();
            }
            return QVector<ShowResponse>();
        }));
    }

    Q_INVOKABLE void popular(int page,int type){
        loading=true;
        emit loadingChanged();
        fetchingMore = false;
        m_searchWatcher.setFuture(QtConcurrent::run ([&](){
            try{
                return Global::instance ().getCurrentSearchProvider ()->popular (page,type);
            }catch(const std::exception& e){
                qCritical() << e.what ();
            }
            return QVector<ShowResponse>();
        }));
    }

    Q_INVOKABLE void loadDetails(int index){
        getDetails(m_List[index]);
    }

public slots:
    void getDetails(const ShowResponse& show){
        if(Global::instance().currentShowObject ()->title() == show.title){
            emit detailsLoaded();
            return;
        }
        loading=true;
        emit loadingChanged();
        m_detailLoadingWatcher.setFuture(QtConcurrent::run ([&](){
            try{
                auto provider = Global::instance().getProvider(show.provider);
                if(provider)
                    return provider->loadDetails (show);
                else qDebug()<<"cannot find provider for "<< show.provider;
            }catch(const std::exception& e){
                qCritical() << e.what ();
            }
            return show;
        }));
    }

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
    inline bool isLoading(){
        return loading;
    }

public:
    Q_INVOKABLE void loadMore(){
        loading=true;
        emit loadingChanged();
        fetchingMore = true;
        m_searchWatcher.setFuture(QtConcurrent::run ([&](){
            try{
                return Global::instance ().getCurrentSearchProvider ()->fetchMore ();
            }catch(const std::exception& e){
                qCritical() << e.what ();
            }
            return QVector<ShowResponse>();
        }));
    };

    Q_INVOKABLE bool canLoadMore()const{
        if(loading)return false;
        return Global::instance ().getCurrentSearchProvider ()->canFetchMore ();
    }
signals:
    //    void currentShowChanged(void);
    void fetchMoreResults(void);
    void detailsLoaded(void);
    void sourceFetched(QString link);
    void postItemsAppended(void);
    void loadingChanged(void);

    // QAbstractItemModel interface
public:
    void fetchMore(const QModelIndex &parent)override{};
    bool canFetchMore(const QModelIndex &parent) const override{return false;};
};



#endif // SEARCHRESULTSMODEL_H
