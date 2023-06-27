#include "searchresultsmodel.h"
#include "application.h"
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
    loading=true;
    emit loadingChanged();
    m_searchWatcher.setFuture(QtConcurrent::run ([&](){
        try{
            //                return Global::instance ().getCurrentSearchProvider ()->search (query,page,type);
            return Application::instance().getCurrentSearchProvider ()->search (query,page,type);
        }catch(const std::exception& e){
            qCritical() << e.what ();
        }
        return QVector<MediaData>();
    }));
}

void SearchResultsModel::latest(int page, int type){
    loading = true;
    emit loadingChanged();
    //        m_searchWatcher.setFuture (QtConcurrent::run (&MediaProvider::latest,Global::instance ().getCurrentSearchProvider (),page,type));
    m_searchWatcher.setFuture(QtConcurrent::run ([&](){
        try{
            return Application::instance().getCurrentSearchProvider ()->latest (page,type);
        }catch(const std::exception& e){
            qCritical() << e.what ();
        }
        return QVector<MediaData>();
    }));
}

void SearchResultsModel::popular(int page, int type){
    loading=true;
    emit loadingChanged();
    fetchingMore = false;
    m_searchWatcher.setFuture(QtConcurrent::run ([&](){
        try{
            return Application::instance().getCurrentSearchProvider ()->popular (page,type);
        }catch(const std::exception& e){
            qCritical() << e.what ();
        }
        return QVector<MediaData>();
    }));
}

void SearchResultsModel::loadMore(){
    loading=true;
    emit loadingChanged();
    fetchingMore = true;
    m_searchWatcher.setFuture(QtConcurrent::run ([&](){
        try{
            return Application::instance().getCurrentSearchProvider ()->fetchMore ();
        }catch(const std::exception& e){
            qCritical() << e.what ();
        }
        return QVector<MediaData>();
    }));
}

bool SearchResultsModel::canLoadMore() const{
    if(loading)return false;
    return Application::instance().getCurrentSearchProvider ()->canFetchMore ();
}

//
void SearchResultsModel::getDetails(const MediaData &show){
    if(Application::instance ().currentShowObject.title() == show.title){
        emit detailsLoaded();
        return;
    }
    loading=true;
    emit loadingChanged();
    m_detailLoadingWatcher.setFuture(QtConcurrent::run ([&](){

        auto provider = Application::instance().providersMap[show.provider];//Global::instance().getProvider(show.provider);
        if(provider){
            try{
                qDebug()<<"Loading details for" << show.title << "with" << provider->name () << "using the link:" << show.link;
                auto d = provider->loadDetails (show);
                qDebug()<<"Successfully loaded details for" << d.title;
                Application::instance ().currentShowObject.setShow (d);
                emit detailsLoaded();             
            }catch(const QException& e){
                qDebug()<<e.what ();
            }
            loading = false;
            emit loadingChanged();
        }else{
            qDebug()<<"Unable to find a provider for provider enum" << show.provider;
        }
        return show;
    }));
}


