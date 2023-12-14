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

    switch (role){
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

SearchResultsModel::SearchResultsModel(QObject *parent) : QAbstractListModel(parent)
{
    QObject::connect (&m_watcher, &QFutureWatcher<QList<ShowData>>::finished, this, [this](){

        if (!m_watcher.future().isValid()) {
            //future was cancelled
            ErrorHandler::instance().show ("Operation cancelled: " + m_cancelReason);
            setLoading (false);
            return;
        }
        try
        {
            setResults (m_watcher.result());
        }
        catch (QException& ex)
            {
            ErrorHandler::instance().show (ex.what ());
        }
        setLoading (false);
    });

}

void SearchResultsModel::search(const QString &query, int page, int type)
{
    if (m_watcher.isRunning ()) return;
    setLoading (true);
    fetchingMore = false;
    m_watcher.setFuture(QtConcurrent::run (&ShowProvider::search,ShowManager::instance().getCurrentSearchProvider(), query, page, type));
}

void SearchResultsModel::latest(int page, int type)
{
    if (m_watcher.isRunning ()) return;
    setLoading (true);
    fetchingMore = false;
    m_watcher.setFuture(QtConcurrent::run (&ShowProvider::latest,ShowManager::instance().getCurrentSearchProvider(), page,type));
}

void SearchResultsModel::popular(int page, int type){
    if (m_watcher.isRunning ()) return;
    setLoading (true);
    fetchingMore = false;
    m_watcher.setFuture(QtConcurrent::run (&ShowProvider::popular,ShowManager::instance().getCurrentSearchProvider(), page,type));
}

void SearchResultsModel::loadShow(int index)
{
    ShowManager::instance ().setCurrentShow (m_list[index]);
}

void SearchResultsModel::cancel()
{
    if (m_watcher.isRunning ())
    {
        m_watcher.cancel ();
//        setLoading (false);
    }
}

void SearchResultsModel::reload(){
    if (m_watcher.isRunning ()) return;
    setLoading (true);
    fetchingMore = false;
    m_watcher.setFuture(QtConcurrent::run (&ShowProvider::reload,ShowManager::instance().getCurrentSearchProvider()));
}

void SearchResultsModel::loadMore()
{
    if (m_watcher.isRunning ()) return;
    setLoading (true);
    fetchingMore = true;
    m_watcher.setFuture(QtConcurrent::run (&ShowProvider::fetchMore,ShowManager::instance().getCurrentSearchProvider()));
}

bool SearchResultsModel::canLoadMore() const{
    if (loading || m_watcher.isRunning ()) return false;
    return ShowManager::instance().getCurrentSearchProvider ()->canFetchMore ();
}

void SearchResultsModel::setResults(QList<ShowData> results)
{
    if (fetchingMore)
    {
        const int oldCount = m_list.count();
        beginInsertRows(QModelIndex(), oldCount, oldCount + results.count() - 1);
        m_list.reserve(oldCount + results.count());
        m_list += results;
        endInsertRows();
    }
    else
    {
        m_list.reserve(results.count());
        m_list.swap(results);
        emit layoutChanged ();
    }
}



