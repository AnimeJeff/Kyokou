#pragma once
#include "Providers/showprovider.h"

class NewProvider : public ShowProvider
{
public:
    explicit NewProvider(QObject *parent = nullptr) : ShowProvider{parent} {};
public:
    int providerEnum()
    {
        return 1;
    }
    QString name()
    {
        return "";
    }
    std::string hostUrl = "";

    QVector<ShowData> search(QString query, int page, int type) override;
    QVector<ShowData> popular(int page, int type) override;
    QVector<ShowData> latest(int page, int type) override;
    void loadDetails(ShowData& show) override;
    int getTotalEpisodes(const ShowData &show) override;
    QVector<VideoServer> loadServers(const PlaylistItem *episode) override;
    QString extractSource(VideoServer &server) override;
};

