#include "watchlistmodel.h"
#include "showmanager.h"

void WatchListModel::readWatchListFile(){
    QFile infile(watchListFileName);
    const char* emptyFile = "[[],[],[],[],[]]";
    if (!infile.exists() || !infile.open(QIODevice::ReadWrite)) { // file doesn't exist or is corrupted
        // File doesn't exist or is corrupted
        qDebug()<<"failed";
        QFile outfile(watchListFileName);
        if (outfile.open(QIODevice::WriteOnly))
        {
            outfile.write (emptyFile);
            outfile.close();
        }
        m_jsonList = nlohmann::json::parse (emptyFile);
    }else{
        if(infile.peek(1)!="[")
        {
            infile.write (emptyFile);
            m_jsonList = nlohmann::json::parse (emptyFile);
        }
        else{
            auto list = infile.readAll ().toStdString ();
//            qDebug() << list;
            m_jsonList = nlohmann::json::parse (list);
        }
        infile.close ();
    }
}

void WatchListModel::parseWatchListFile(){
    for (int i = 0;i<m_jsonList.size ();i++){
        auto array = m_jsonList[i];
        m_list.push_back (QVector<std::shared_ptr<ShowData>>{});
        if(array.empty ())continue;
//        QVector<std::shared_ptr<ShowData>> list{m_list[i]};

        for(auto& item:array){
            // qDebug()<<QString::fromStdString (item["title"].dump ());
            m_list[i].push_back (std::make_shared<ShowData>(ShowData(item)));
            m_list[i].back ()->setListType (i);
        }
    }
}

void WatchListModel::changeListType(const ShowData &show, int listType){
    auto showLink = show.link;
    auto& list = m_jsonList[show.listType];
    for(int i = 0;i<list.size ();i++){
        if(list[i]["link"] == showLink){
            m_jsonList[listType].push_back(list[i]);
            m_jsonList[show.listType].erase (list.begin() + i);
            m_list[listType].push_back (m_list[show.listType][i]);
            m_list[show.listType].removeAt (i);
            ShowManager::instance ().setListType (listType);
            emit layoutChanged();
            save();
            break;
        }
    }
}

void WatchListModel::fetchUnwatchedEpisodes(){
    return;
    int count = 0;
    for(const auto& show:m_list[WATCHING]){
        auto provider = ShowManager::instance ().getProvider (show->provider);
        if(!provider) {
            qDebug()<<"Unable to find a provider for provider enum" << show->provider;
            continue;
        }
        //            qDebug()<<"Getting the total episodes for" << show->title << "with" << provider->name ();
        auto totalEpisodes = provider->getTotalEpisodes(*show);
        show->totalEpisodes = totalEpisodes;
        //            qDebug()<<totalEpisodes;
        //            emit dataChanged(index(count,0),index(count,0));
        count++;
    }
    emit layoutChanged();
}

void WatchListModel::add(const ShowData& show, int listType){
    m_jsonList[listType].push_back (show.toJson ());
    m_list[listType].push_back (std::make_shared<ShowData>(show));
    m_list[listType].last ()->setJsonObject (m_jsonList[listType].back ());
    if(m_displayingListType == listType) emit layoutChanged ();
    save();
}

void WatchListModel::addCurrentShow(int listType){
    auto currentShow = ShowManager::instance ().getCurrentShow ();
    if(currentShow.isInWatchList ()){
        //change the list type
        changeListType (currentShow,listType);
    }else{
        add(currentShow, listType);
        ShowManager::instance().setListType (listType);
    }
}

void WatchListModel::remove(const ShowData &show){
    auto& list = m_jsonList[show.listType];
    for(int i = 0;i<list.size ();i++){
        if(list[i]["link"] == show.link){
            list.erase (list.begin() + i);
            m_list[show.listType].removeAt (i);
            emit layoutChanged();
            save();
            break;
        }
    }


}

void WatchListModel::removeCurrentShow(){
    remove (ShowManager::instance().getCurrentShow ());
    ShowManager::instance ().setListType(-1);
}

void WatchListModel::move(int from, int to)
{

    //beginMoveRows(QModelIndex(), from, from, QModelIndex(), to);
    m_list[m_displayingListType].move (from,to);
    //endMoveRows ();
    auto& list = m_jsonList[m_displayingListType];
    auto element_to_move = list[from];
    list.erase(list.begin() + from);
    list.insert(list.begin() + to, element_to_move);
}

void WatchListModel::moveEnded(){
    emit layoutChanged ();
    save();
}

bool WatchListModel::checkCurrentShowInList(){
    ShowManager::instance ().getCurrentShow ();
    if(ShowManager::instance ().getCurrentShow ().isInWatchList ()){
        qDebug()<<"Loaded from watch list";
        return true;
    }
    auto showInList = getShowInList (ShowManager::instance ().getCurrentShow ());
    if(showInList){
        qDebug() << "Found" << showInList->title << "in watch list";
        ShowManager::instance ().setLastWatchedIndex(showInList->lastWatchedIndex);
        ShowManager::instance().setListType(showInList->listType);
        return true;
    }

    return false;
}

int WatchListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_list[m_displayingListType].count ();
}

QVariant WatchListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    const auto show = m_list[m_displayingListType][index.row ()];
    switch (role) {
    case TitleRole:
        return show->title;
        break;
    case CoverRole:
        return show->coverUrl;
        break;
    case UnwatchedEpisodesRole:
        if(show->totalEpisodes > show->lastWatchedIndex)
        {
            if(show->lastWatchedIndex<0){
                return show->totalEpisodes;
            }
            return show->totalEpisodes - show->lastWatchedIndex - 1;
        }
        break;
    default:
        break;
    }
    return QVariant();
}

QHash<int, QByteArray> WatchListModel::roleNames() const{
    QHash<int, QByteArray> names;
    names[TitleRole] = "title";
    names[CoverRole] = "cover";
    names[UnwatchedEpisodesRole] = "unwatchedEpisodes";
    return names;
}

bool WatchListModel::checkInList(const ShowData &show) {
    if (show.isInWatchList()) {
        return true;
    } else if (getShowInList(show)) {
        return true;
    }
    return false;
}

std::shared_ptr<ShowData> WatchListModel::getShowInList(const ShowData &show) {
    for(int i = 0;i<m_jsonList.size ();i++){
        nlohmann::json::array_t list = m_jsonList[i];
        //                qDebug()<<list.;
        for(int j = 0;j<list.size ();j++){
            if(list[j]["link"] == show.link){
                return m_list[i][j];
            }
        }
    }
    qDebug()<<"Unable to find" << show.title << "in watch list";
    return nullptr;
}

nlohmann::json *WatchListModel::getShowJsonInList(const ShowData &show) {
    for(int i = 0;i<m_jsonList.size ();i++){
        nlohmann::json::array_t list = m_jsonList[i];
        for(int j = 0;j<list.size ();j++){
            if(list[j]["link"] == show.link){
                return &m_jsonList[i][j];
            }
        }
    }
    qDebug()<<"Unable to find" << show.title << "json in watch list";
    return nullptr;
}

void WatchListModel::loadDetails(int index){
    ShowData show = *m_list[m_displayingListType][index];
    emit detailsRequested (show);
    //        auto lol = QtConcurrent::run ([show,this](){
    //            auto loadedShow = ShowManager::instance ().loadDetails (show);
    //            ShowManager::instance ().setCurrentShow (loadedShow);
    //        });
}
