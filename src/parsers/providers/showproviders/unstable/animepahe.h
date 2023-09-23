
#ifndef ANIMEPAHE_H
#define ANIMEPAHE_H

#include "parsers/showprovider.h"
#include "tools/functions.h"
class AnimePahe : public ShowProvider
{
public:
    AnimePahe();

    // ShowProvider interface
public:
    int providerEnum(){
        return Providers::ANIMEPAHE;
    };
    QString name(){
        return "AnimePahe";
    };
    std::string hostUrl = "https://animepahe.ru";

    QVector<ShowData> search(QString query, int page, int type) override {
        std::string url = hostUrl + "/api?m=search&q=" + Functions::urlEncode(query.toStdString ()) ;
        NetworkClient::get(url); //TODO
    };
    QVector<ShowData> popular(int page, int type){
        return QVector<ShowData>();
    };
    QVector<ShowData> latest(int page, int type){
        return QVector<ShowData>();
    };
    ShowData loadDetails(ShowData show){
        return show;
    };
    int getTotalEpisodes(const ShowData &show){
        return 0;
    };
    QVector<VideoServer> loadServers(const Episode &episode){
        return QVector<VideoServer>();
    };
    QString extractSource(VideoServer &server){
        return "";
    };
};

#endif // ANIMEPAHE_H
