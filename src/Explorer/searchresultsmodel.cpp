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
            // future was cancelled
            ErrorHandler::instance().show ("Operation cancelled: " + m_cancelReason);
            setLoading (false);
            return;
        }
        try
        {

            auto results = m_watcher.result();
            m_canFetchMore = !results.isEmpty ();
            if (!results.isEmpty ())
            {
                if (fetchingMore)
                {
                    const int oldCount = m_list.count();
                    beginInsertRows(QModelIndex(), oldCount, oldCount + results.count() - 1);
                    m_list.reserve(oldCount + results.count());
                    m_list += results;
                    endInsertRows();
                    fetchingMore = false;
                } else {
                    m_list.reserve(results.count());
                    m_list.swap(results);
                    emit layoutChanged ();
                }
            }
        }
        catch (QException& ex)
            {
            ErrorHandler::instance().show (ex.what ());
        }
        setLoading (false);
    });

}

void SearchResultsModel::search(const QString &query, int page, int type, ShowProvider* provider)
{
    if (m_watcher.isRunning ()) return;
    setLoading (true);
    m_currentPageIndex = page;
    m_watcher.setFuture(QtConcurrent::run (&ShowProvider::search, provider, query, page, type));
    lastSearch = [this, query, type, provider](int page) {
        search(query, page, type, provider);
    };
}

void SearchResultsModel::latest(int page, int type, ShowProvider* provider)
{
    if (m_watcher.isRunning ()) return;
    setLoading (true);
    m_currentPageIndex = page;
    m_watcher.setFuture(QtConcurrent::run (&ShowProvider::latest, provider, page,type));
    lastSearch = [this, type, provider](int page){
        latest(page, type, provider);
    };
}

void SearchResultsModel::popular(int page, int type, ShowProvider* provider){
    if (m_watcher.isRunning ()) return;
    setLoading (true);
    m_currentPageIndex = page;
    m_watcher.setFuture(QtConcurrent::run (&ShowProvider::popular, provider, page, type));
    lastSearch = [this, type, provider](int page) {
        popular(page, type, provider);
    };
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
    lastSearch(m_currentPageIndex);
}

void SearchResultsModel::loadMore()
{
    if (m_watcher.isRunning ()) return;
    setLoading (true);
    fetchingMore = true;
    lastSearch(m_currentPageIndex + 1);
}

bool SearchResultsModel::canLoadMore() const{
    return (loading || m_watcher.isRunning ()) ? false : m_canFetchMore;
}





