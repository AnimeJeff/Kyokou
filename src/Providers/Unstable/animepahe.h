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

//    QVector<ShowData> search(QString query, int page, int type) override {
//        std::string url = hostUrl + "/api?m=search&q=" + Functions::urlEncode(query.toStdString ()) ;
//        NetworkClient::get(url); //TODO
//    };
//    QVector<ShowData> popular(int page, int type)
//    {
//        return QVector<ShowData>();
//    };
//    QVector<ShowData> latest(int page, int type)
//    {
//        return QVector<ShowData>();
//    };
//    void loadDetails(ShowData& show) override
//        {

//    };
//    int getTotalEpisodes(const ShowData &show)
//    {
//        return 0;
//    };
//    QVector<VideoServer> loadServers(const PlaylistItem &episode)
//    {
//        return QVector<VideoServer>();
//    };
//    QString extractSource(VideoServer &server){
//        return "";
//    };
//};

