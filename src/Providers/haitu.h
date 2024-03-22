#pragma once
#include <regex>
#include "showprovider.h"
#include "Explorer/Data/showdata.h"

class Haitu : public ShowProvider
{
    std::regex player_aaaa_regex{R"(player_aaaa=(\{.*?\})</script>)"};
public:
    explicit Haitu(QObject *parent = nullptr) : ShowProvider{parent}{

    };
public:
    QString name() const override { return "海兔影院"; }
    std::string hostUrl = "https://www.haituu.tv/";
    QList<int> getAvailableTypes() const override {
        return {ShowData::ANIME, ShowData::MOVIE, ShowData::TVSERIES, ShowData::VARIETY};
    };

    QList<ShowData> search(QString query, int page, int type) override;
    QList<ShowData> popular(int page, int type) override;
    QList<ShowData> latest(int page, int type) override;
    QList<ShowData> filterSearch(const std::string& url);

    void loadDetails(ShowData& show) const override;
    int getTotalEpisodes(const std::string& link) const override
    {
        return 0;
    }
    QList<VideoServer> loadServers(const PlaylistItem* episode) const override;
    QString extractSource(const VideoServer &server) const override;
};
