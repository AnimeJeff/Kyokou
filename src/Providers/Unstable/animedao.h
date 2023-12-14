//#pragma once
//#include "Providers/showprovider.h"



//class AnimeDao : public ShowProvider
//{
//public:
//    AnimeDao();

//    // ShowProvider interface
//public:
//    int providerEnum() override {
//        return Providers::ANIMEDAO;
//    };
//    QString name() override {
//        return "AnimeDao";
//    };
//    std::string hostUrl = "https://animedao.to";

//    QList<ShowData> search(QString query, int page, int type) override {
//        std::string url = hostUrl + "/search/?search=" + query.toStdString ()+ "&page=" + std::to_string (page);
//        auto animeNodes = NetworkClient::get( url ).document ().select("//div[@class='card-body']");
//        QList<ShowData> animes;
//        for (pugi::xpath_node_set::const_iterator it = animeNodes.begin(); it != animeNodes.end(); ++it)
//        {
//            auto title = it->selectFirst(".//span[@class='animename']").node ().child_value ();
//            auto coverUrl = hostUrl + it->selectFirst(".//div[@class='animeposter']/div/a").attr ("href").as_string ();
//            auto link = it->selectFirst(".//div[@class='animeinfo']/a").attr ("href").as_string ();
//            animes.emplaceBack(ShowData(title,link,QString::fromStdString (coverUrl),Providers::ANIMEDAO));
//        }
//        return animes;
//    };
//    QList<ShowData> popular(int page, int type){
//        return QList<ShowData>{};
//    };
//    QList<ShowData> latest(int page, int type) override {
//        return QList<ShowData>{};
//    };
//    void loadDetails(ShowData& show) override {

//    };
//    int getTotalEpisodes(const ShowData &show) override {
//        return 0;
//    };
//    QList<VideoServer> loadServers(const PlaylistItem *episode) override {
//        return QList<VideoServer>{};
//    };
//    QString extractSource(VideoServer &server) override {
//        return "";
//    };
//};

