#pragma once
#include <QDebug>
#include "showprovider.h"



class Gogoanime : public ShowProvider
{
public:
    Gogoanime() = default;
    QString name() const override { return "Anitaku"; }
    std::string hostUrl = "https://anitaku.to";
    QList<int> getAvailableTypes() const override {
        return {ShowData::ANIME};
    };

    QList<ShowData> search(QString query, int page, int type = 0) override;
    QList<ShowData> popular(int page, int type = 0) override;
    QList<ShowData> latest(int page, int type = 0) override;

    void loadDetails(ShowData& anime) const override;
    int getTotalEpisodes(const std::string& link) const override;
    QList<VideoServer> loadServers(const PlaylistItem* episode) const override;
    QString extractSource(const VideoServer& server) const override;
private:
    CSoup getInfoPage(const std::string& link) const;
    std::string getEpisodesLink(const CSoup& doc) const;
};



