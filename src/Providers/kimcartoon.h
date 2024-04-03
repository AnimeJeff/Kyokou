#pragma once
#include "Providers/showprovider.h"

class Kimcartoon : public ShowProvider {
public:
    explicit Kimcartoon(QObject *parent = nullptr) : ShowProvider{parent} {};
    std::regex sourceRegex{"sources: \\[\\{file:\"(.+?)\"\\}\\]"};



public:
    QString name() const override { return "KIMCartoon"; }
    std::string hostUrl = "https://kimcartoon.li/";

    QVector<ShowData> search(QString query, int page, int type) override;
    QVector<ShowData> popular(int page, int type) override;
    QVector<ShowData> latest(int page, int type) override;
    QVector<ShowData> filterSearch(std::string url);

    void loadDetails(ShowData &show) const override;
    int getTotalEpisodes(const QString &link) const override { return 0; };
    QVector<VideoServer> loadServers(const PlaylistItem *episode) const override;;
    QList<Video> extractSource(const VideoServer &server) const override;

    QList<int> getAvailableTypes() const override { return {ShowData::ANIME}; }
};


