#pragma once
#include <regex>
#include "showprovider.h"

class Haitu : public ShowProvider
{
    std::regex player_aaaa_regex{R"(player_aaaa=(\{.*?\})</script>)"};
public:
    explicit Haitu(QObject *parent = nullptr) : ShowProvider{parent}{

    };
public:
    QString name() const override { return "海兔影院"; }
    std::string hostUrl = "https://www.haitu.tv";
    QList<int> getAvailableTypes() const override {
        return {ShowData::ANIME, ShowData::MOVIE, ShowData::TVSERIES, ShowData::VARIETY, ShowData::DOCUMENTARY};
    };

    QVector<ShowData> search(QString query, int page, int type) override;
    QVector<ShowData> popular(int page, int type) override;
    QVector<ShowData> latest(int page, int type) override;
    QVector<ShowData> filterSearch(const std::string& url);

    void loadDetails(ShowData& show) const override;
    int getTotalEpisodes(const ShowData &show) const override
    {
        return 0;
    }
    QVector<VideoServer> loadServers(const PlaylistItem* episode) const override;
    QString extractSource(VideoServer &server) const override;
};
