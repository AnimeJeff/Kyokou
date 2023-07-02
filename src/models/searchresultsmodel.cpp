#include "searchresultsmodel.h"

//Model implementations
int SearchResultsModel::rowCount(const QModelIndex &parent) const{
    if (parent.isValid())
        return 0;
    return m_list.count ();
}

QVariant SearchResultsModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();

    auto show = m_list.at (index.row ());

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

//load results
void SearchResultsModel::search(const QString &query, int page, int type){
    if(m_searchWatcher.isRunning ())return;
    setLoading (true);
    m_searchWatcher.setFuture(QtConcurrent::run ([query,page,type](){
        try{
            return ShowManager::instance().getCurrentSearchProvider ()->search (query,page,type);
        }catch(const std::exception& e){
            qCritical() << e.what ();
        }catch(...){
            qCritical() << "Uncaught exception while searching";
        }

        return QVector<ShowData>();
    }));
}

void SearchResultsModel::latest(int page, int type){
    if(m_searchWatcher.isRunning ())return;
    setLoading(true);
    m_searchWatcher.setFuture(QtConcurrent::run ([page,type](){
        try{
            return ShowManager::instance().getCurrentSearchProvider ()->latest (page,type);
        }catch(const std::exception& e){
            qCritical() << e.what ();
        }
        return QVector<ShowData>();
    }));
}

void SearchResultsModel::popular(int page, int type){
    if(m_searchWatcher.isRunning ())return;
    setLoading (true);
    fetchingMore = false;
    m_searchWatcher.setFuture(QtConcurrent::run ([page,type](){
        try{
            return ShowManager::instance().getCurrentSearchProvider ()->popular (page,type);
        }catch(const std::exception& e){
            qCritical() << e.what ();
        }catch(...){
            qCritical() << "uncaught exception";
        }
        return QVector<ShowData>();
    }));
}

void SearchResultsModel::loadMore(){
    loading=true;
    emit loadingChanged();
    fetchingMore = true;
    m_searchWatcher.setFuture(QtConcurrent::run ([](){
        try{
            return ShowManager::instance().getCurrentSearchProvider ()->fetchMore ();
        }catch(const std::exception& e){
            qCritical() << e.what ();
        }catch(...){
            qCritical() << "uncaught exception";
        }
        return QVector<ShowData>();
    }));
}

bool SearchResultsModel::canLoadMore() const{
    if(loading)return false;
    return ShowManager::instance().getCurrentSearchProvider ()->canFetchMore ();
}

//
void SearchResultsModel::getDetails(const ShowData &show){
    if(ShowManager::instance ().getCurrentShow () == show){
        emit detailsLoaded();
        return;
    }
    setLoading(true);
    m_detailLoadingWatcher.setFuture(QtConcurrent::run ([show,this](){
        auto provider = ShowManager::instance ().getProvider (show.provider);
        if(!provider) {
            qDebug()<<"Unable to find a provider for provider enum" << show.provider;
            return show;
        }
        try{
            qDebug()<<"Loading details for" << show.title << "with" << provider->name () << "using the link:" << show.link;
            auto loadedShow = provider->loadDetails (show);
            qDebug()<<"Successfully loaded details for" << loadedShow.title;
            return loadedShow;
        }catch(const QException& e){
            qDebug()<<e.what ();
        }catch(...){
            qDebug()<<"Failed to load" << show.title;
        }
        return show;
    }));
}


