
#ifndef ANIMEDAO_H
#define ANIMEDAO_H
#include "parsers/showprovider.h"



class AnimeDao : public ShowProvider
{
public:
    AnimeDao();

    // ShowProvider interface
public:
    int providerEnum(){
        return Providers::ANIMEDAO;
    };
    QString name() {
        return "AnimeDao";
    };
    std::string hostUrl() override {
        return "https://animedao.to";
    };

    QVector<ShowData> search(QString query, int page, int type) override {
        std::string url = hostUrl() + "/search/?search=" + query.toStdString ()+ "&page=" + std::to_string (page);
        auto animeNodes = NetworkClient::get( url ).document ().select("//div[@class='card-body']");
        QVector<ShowData> animes;
        for (pugi::xpath_node_set::const_iterator it = animeNodes.begin(); it != animeNodes.end(); ++it)
        {
            auto title = it->selectFirst(".//span[@class='animename']").node ().child_value ();
            auto coverUrl = hostUrl() + it->selectFirst(".//div[@class='animeposter']/div/a").attr ("href").as_string ();
            auto link = it->selectFirst(".//div[@class='animeinfo']/a").attr ("href").as_string ();
            animes.emplaceBack(ShowData(title,link,QString::fromStdString (coverUrl),Providers::ANIMEDAO));
        }
        return animes;
    };
    QVector<ShowData> popular(int page, int type){
        return QVector<ShowData>{};
    };
    QVector<ShowData> latest(int page, int type) override {
        return QVector<ShowData>{};
    };
    ShowData loadDetails(ShowData show) override {
        return show;
    };
    int getTotalEpisodes(const ShowData &show) override {
        return 0;
    };
    QVector<VideoServer> loadServers(const Episode &episode) override {
        return QVector<VideoServer>{};
    };
    QString extractSource(VideoServer &server) override {
        return "";
    };
};

#endif // ANIMEDAO_H
