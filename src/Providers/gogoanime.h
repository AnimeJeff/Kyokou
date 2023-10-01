#pragma once
#include <QDebug>
#include "showprovider.h"
#include "Extractors/gogocdn.h"

class Gogoanime : public ShowProvider
{
public:
    Gogoanime() = default;
    QString name() const override { return "Gogoanime"; }
    std::string hostUrl = "https://gogoanimehd.io";
    QList<int> getAvailableTypes() const override {
        return {ShowData::ANIME};
    };

    QVector<ShowData> search(QString query, int page, int type = 0) override;
    QVector<ShowData> popular(int page, int type = 0) override;
    QVector<ShowData> latest(int page, int type = 0) override;

    void loadDetails(ShowData& anime) const override;
    int getTotalEpisodes(const ShowData &anime) const override;
    QVector<VideoServer> loadServers(const PlaylistItem* episode) const override;
    QString extractSource(VideoServer& server) const override;
private:
    CSoup getInfoPage(const ShowData& anime) const;
    std::string getEpisodesLink(const CSoup& doc) const;
};



