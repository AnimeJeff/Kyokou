// #pragma once
// #include <QString>
// #include <regex>
// #include "showprovider.h"
// class Tangrenjie: public ShowProvider
// {
// public:
//     Tangrenjie() = default;
//     QString name() const override {return "唐人街影院";}
//     std::string hostUrl = "https://www.tangrenjie.tv";

//     QList<int> getAvailableTypes() const override {
//         return {ShowData::ANIME, ShowData::MOVIE, ShowData::TVSERIES, ShowData::VARIETY, ShowData::DOCUMENTARY};
//     };

//     QList<ShowData> search(QString query, int page, int type) override;
//     QList<ShowData> popular(int page, int type) override;
//     QList<ShowData> latest(int page, int type) override;

//     void loadDetails(ShowData& show) const override;
//     int getTotalEpisodes(const std::string& link) const override;
//     QList<VideoServer> loadServers(const PlaylistItem *episode) const override;
//     QList<Video> extractSource(VideoServer &server) const override;


// private:
//     QList<ShowData> filterSearch(int page, int type,const std::string& sortBy,const std::string& area="",const std::string& year="",const std::string& language="");
//     std::regex player_aaaa_regex{R"(player_aaaa=(\{.*?\})</script>)"};
//     QList<ShowData> selectShow(const std::string& url) const;
// };



