#include "searchresultsmodel.h"
#include "Providers/showprovider.h"
#include "Components/ErrorHandler.h"


SearchResultsModel::SearchResultsModel(QObject *parent) : QAbstractListModel(parent)
{
    QObject::connect (&m_watcher, &QFutureWatcher<QList<ShowData>>::finished, this, [this](){

        if (!m_watcher.future().isValid()) {
            // Operation was cancelled
            ErrorHandler::instance().show ("Operation cancelled: " + m_cancelReason);
        } else {
            try {
                auto results = m_watcher.result();
                m_canFetchMore = !results.isEmpty ();
                if (m_currentPageIndex > 1) {
                    const int oldCount = m_list.count();
                    beginInsertRows(QModelIndex(), oldCount, oldCount + results.count() - 1);
                    m_list.reserve(oldCount + results.count());
                    m_list += results;
                    endInsertRows();
                } else {
                    if (!m_list.isEmpty ()) {
                        beginRemoveRows(QModelIndex(), 0, m_list.size () - 1);
                        m_list.clear ();
                        endRemoveRows ();
                    }
                    if (!results.isEmpty ()) {
                        beginInsertRows(QModelIndex(), 0, results.size() - 1);
                        m_list.reserve(results.size());
                        m_list.swap(results);
                        endInsertRows();
                    }
                }
            }
            catch (QException& ex) {
                ErrorHandler::instance().show (ex.what ());
            }
        }

        setIsLoading (false);
    });

}

void SearchResultsModel::search(const QString &query, int page, int type, ShowProvider* provider)
{
    if (m_watcher.isRunning ()) return;
    setIsLoading (true);
    m_currentPageIndex = page;
    m_watcher.setFuture(QtConcurrent::run (&ShowProvider::search, provider, query, page, type));
    lastSearch = [this, query, type, provider](int page) {
        search(query, page, type, provider);
    };
}

void SearchResultsModel::latest(int page, int type, ShowProvider* provider)
{
    if (m_watcher.isRunning ()) return;
    setIsLoading (true);
    m_currentPageIndex = page;
    m_watcher.setFuture(QtConcurrent::run (&ShowProvider::latest, provider, page,type));
    lastSearch = [this, type, provider](int page){
        latest(page, type, provider);
    };
}

void SearchResultsModel::popular(int page, int type, ShowProvider* provider){
    if (m_watcher.isRunning ()) return;
    setIsLoading (true);
    m_currentPageIndex = page;
    m_watcher.setFuture(QtConcurrent::run (&ShowProvider::popular, provider, page, type));
    lastSearch = [this, type, provider](int page) {
        popular(page, type, provider);
    };
}

void SearchResultsModel::cancel() {
    if (m_watcher.isRunning ()) {
        m_watcher.cancel ();
    }
}

void SearchResultsModel::reload() {
    if (m_watcher.isRunning ()) return;
    lastSearch(m_currentPageIndex);
}

void SearchResultsModel::loadMore() {
    if (m_watcher.isRunning ()) return;
    lastSearch(m_currentPageIndex + 1);
}

bool SearchResultsModel::canLoadMore() const {
    return (m_isLoading || m_watcher.isRunning ()) ? false : m_canFetchMore;
}

int SearchResultsModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return m_list.count ();
}

QVariant SearchResultsModel::data(const QModelIndex &index, int role) const {
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


