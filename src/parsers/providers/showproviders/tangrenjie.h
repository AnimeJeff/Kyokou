
#ifndef TANGRENJIE_H
#define TANGRENJIE_H
#include <QString>
#include <regex>
#include "parsers/showprovider.h"
class Tangrenjie:public ShowProvider
{
    std::regex player_aaaa_regex{R"(player_aaaa=(\{.*?\})</script>)"};
    QVector<ShowData> selectShow(const std::string& url);
public:
    Tangrenjie() = default;
public:
    int providerEnum() override {return Providers::TANGRENJIE;}

    QString name() override {return "唐人街影院";}

    std::string hostUrl() override {return "https://www.tangrenjie.tv";}

    QVector<ShowData> search(QString query, int page, int type) override;;

    QVector<ShowData> popular(int page, int type) override {
        return filterSearch(page,type,"hits");
    }
    QVector<ShowData> latest(int page, int type) override {
        return filterSearch(page,type,"time");
    }
    QVector<ShowData> filterSearch(int page, int type,const std::string& sortBy,const std::string& area="",const std::string& year="",const std::string& language="");
    ShowData loadDetails(ShowData show) override;;
    int getTotalEpisodes(const ShowData& show) override;
    QVector<VideoServer> loadServers(const Episode &episode) override;;
    QString extractSource(VideoServer &server) override;;
};

#endif // TANGRENJIE_H
