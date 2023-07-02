#ifndef SEARCHRESULTSMODEL_H
#define SEARCHRESULTSMODEL_H

#include <QAbstractListModel>
#include "parsers/data/showdata.h"
#include "showmanager.h"
#include <global.h>
#include <QtConcurrent>


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
public:
    explicit SearchResultsModel(QObject *parent = nullptr)
        : QAbstractListModel(parent){
        QObject::connect(&m_searchWatcher, &QFutureWatcher<QVector<ShowData>>::finished,this, [this]() {
            setResults(m_searchWatcher.future ().result ());
        });
        QObject::connect(&m_detailLoadingWatcher, &QFutureWatcher<ShowData>::finished,this, [this]() {
            setCurrentShow(m_detailLoadingWatcher.future ().result ());
        });
    }
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

    void setCurrentShow(const ShowData& show){
        ShowManager::instance ().setCurrentShow(show);
        emit detailsLoaded();
        setLoading(false);
    }

    Q_INVOKABLE void search(const QString& query,int page,int type);

    Q_INVOKABLE void latest(int page,int type);

    Q_INVOKABLE void popular(int page,int type);

    Q_INVOKABLE void loadDetails(int index){
        getDetails(m_list[index]);
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
