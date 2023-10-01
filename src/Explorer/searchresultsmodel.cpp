#include "searchresultsmodel.h"
#include "showmanager.h"
#include "Components/errorhandler.h"

int SearchResultsModel::rowCount(const QModelIndex &parent) const{
    if (parent.isValid())
        return 0;
    return m_list.count ();
}

QVariant SearchResultsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const ShowData& show = m_list.at(index.row());

    switch (role) {
    case TitleRole:
        return show.title;
        break;
    case CoverRole:
        return show.coverUrl;
        break;
    default:
        break;
    }
    return QVariant();
}

void SearchResultsModel::search(const QString &query, int page, int type)
{
    if (m_watcher.isRunning ()) return;
    setLoading (true);
    m_watcher.setFuture(QtConcurrent::run ([query,page,type](){
        return ShowManager::instance().getCurrentSearchProvider ()->search (query,page,type);
    }));
    m_watcher.future()
        .then([this](QVector<ShowData> results){
            setResults (results);
        })
        .onFailed ([this](const std::exception &e){
            qDebug() << e.what();
            ErrorHandler::instance().show(e.what());
        })
        .onCanceled ([this](){
            qDebug() << "Operation cancelled:" << m_cancelReason;
        });
}

void SearchResultsModel::latest(int page, int type)
{
    if (m_watcher.isRunning ()) return;
    setLoading (true);
    m_watcher.setFuture(QtConcurrent::run ([page,type](){
        return ShowManager::instance().getCurrentSearchProvider ()->latest (page,type);
    }));
    m_watcher.future()
        .then([this](QVector<ShowData> results){
            setResults (results);
        })
        .onFailed ([this](const std::exception &e){
            qDebug() << e.what();
            setLoading(false);
            ErrorHandler::instance().show(e.what());
        })
        .onCanceled ([this](){
            setLoading(false);
            qDebug() << "Operation cancelled:" << m_cancelReason;
        });
}

void SearchResultsModel::popular(int page, int type){
    if (m_watcher.isRunning ()) return;
    fetchingMore = false;
    setLoading (true);
    m_watcher.setFuture(QtConcurrent::run ([page,type](){
        return ShowManager::instance().getCurrentSearchProvider ()->popular (page,type);
    }));
    m_watcher.future()
        .then([this](QVector<ShowData> results){
            setResults (results);
        })
        .onFailed ([this](const std::exception &e){
            qDebug() << e.what();
            setLoading(false);
            ErrorHandler::instance().show(e.what());
        })
        .onCanceled ([this](){
            setLoading(false);
            qDebug() << "Operation cancelled:" << m_cancelReason;
        });

}

void SearchResultsModel::loadShow(int index)
{
    ShowManager::instance ().setCurrentShow (m_list[index]);
}

void SearchResultsModel::reload(){
    if (m_watcher.isRunning ()) return;
    setLoading (true);
    m_watcher.setFuture(QtConcurrent::run ([](){
        return ShowManager::instance().getCurrentSearchProvider ()->reload();
    }));
    m_watcher.future()
        .then([this](QVector<ShowData> results){
            setResults (results);
        })
        .onFailed ([this](const std::exception &e){
            qDebug() << e.what();
            setLoading(false);
            ErrorHandler::instance().show(e.what());
        })
        .onCanceled ([this](){
            setLoading(false);
            qDebug() << "Operation cancelled:" << m_cancelReason;
        });
}

void SearchResultsModel::loadMore()
{
    if (m_watcher.isRunning ()) return;
    fetchingMore = true;
    setLoading (true);
    m_watcher.setFuture(QtConcurrent::run ([](){
        return ShowManager::instance().getCurrentSearchProvider ()->fetchMore ();
    }));
    m_watcher.future()
        .then([this](QVector<ShowData> results){
            setResults (results);
        })
        .onFailed ([this](const std::exception &e){
            qDebug() << e.what();
            setLoading(false);
            ErrorHandler::instance().show(e.what());
        })
        .onCanceled ([this](){
            setLoading(false);
            qDebug() << "Operation cancelled:" << m_cancelReason;
        });
}

bool SearchResultsModel::canLoadMore() const{
    if(loading)return false;
    return ShowManager::instance().getCurrentSearchProvider ()->canFetchMore ();
}

void SearchResultsModel::setResults(QVector<ShowData> results)
{
    if(fetchingMore)
    {
        const int oldCount = m_list.count();
        beginInsertRows(QModelIndex(), oldCount, oldCount + results.count() - 1);
        m_list.reserve(oldCount + results.count());
        m_list += results;
        endInsertRows();
        fetchingMore = false;
    }
    else
    {
        m_list.reserve(results.count());
        m_list.swap(results);
        emit layoutChanged ();
    }
    setLoading (false);
}



