//#pragma once
//#include "Providers/showprovider.h"

//class AnimePahe : public ShowProvider
//{
//public:
//    AnimePahe();

//    // ShowProvider interface
//public:
//    int providerEnum(){
//        return Providers::ANIMEPAHE;
//    };
//    QString name(){
//        return "AnimePahe";
//    };
//    std::string hostUrl = "https://animepahe.ru";

//    QList<ShowData> search(QString query, int page, int type) override {
//        std::string url = hostUrl + "/api?m=search&q=" + Functions::urlEncode(query.toStdString ()) ;
//        NetworkClient::get(url); //TODO
//    };
//    QList<ShowData> popular(int page, int type)
//    {
//        return QList<ShowData>();
//    };
//    QList<ShowData> latest(int page, int type)
//    {
//        return QList<ShowData>();
//    };
//    void loadDetails(ShowData& show) override
//        {

//    };
//    int getTotalEpisodes(const ShowData &show)
//    {
//        return 0;
//    };
//    QList<VideoServer> loadServers(const PlaylistItem &episode)
//    {
//        return QList<VideoServer>();
//    };
//    QString extractSource(VideoServer &server){
//        return "";
//    };
//};

