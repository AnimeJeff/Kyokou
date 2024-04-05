#pragma once
#include "showprovider.h"
#include "Data/showdata.h"
class Nivod: public ShowProvider
{
public:
    Nivod(){};
    QString name() const override {return "泥巴影院";}
    QString hostUrl = "https://www.nivod4.tv/";
    inline QList<int> getAvailableTypes() const override {
        return {ShowData::ANIME, ShowData::MOVIE, ShowData::TVSERIES, ShowData::VARIETY, ShowData::DOCUMENTARY};
    };

    QList<ShowData> search(QString query, int page, int type) override;
    inline QList<ShowData> popular(int page, int type) override { return filterSearch(page, "1", type); };
    inline QList<ShowData> latest(int page, int type) override { return filterSearch(page, "4", type); };

    void loadDetails(ShowData& show) const override;
    inline QList<VideoServer> loadServers(const PlaylistItem *episode) const override { return {VideoServer{"default", episode->link}}; };
    inline int getTotalEpisodes(const QString &link) const override { return getInfoJson(link)["plays"].toArray().size(); }
    QList<Video> extractSource(const VideoServer& server) const override;

private:
    QJsonObject getInfoJson(const QString& link) const;
    QList<ShowData> parseShows(const QJsonArray& showList);
    QList<ShowData> filterSearch(int page, const QString& sortBy, int type, const QString& regionId = "0",
                                 const QString& langId = "0", const QString& yearRange = " ");
    QJsonObject invokeAPI(const QString& url, const QMap<QString, QString>& data) const;
    std::string decryptedByDES(const std::string &input) const;

private:
    const QString _HOST_CONFIG_KEY = "2x_Give_it_a_shot";
    const QString _bp_app_version = "1.0";
    const QString _bp_platform = "3";
    const QString _bp_market_id = "web_nivod";
    const QString _bp_device_code = "web";
    const QString _bp_versioncode = "1";
    const QString _QUERY_PREFIX = "__QUERY::";
    const QString _BODY_PREFIX = "__BODY::";
    const QString _SECRET_PREFIX = "__KEY::";
    const QString _oid = "a376c2407d77f46d0a7af5e4f20e213b67af3d346690b805";
    const QString _mts = "1690816442085";//std::to_string(QDateTime::currentMSecsSinceEpoch())
    const QMap<QString, QString> queryMap =
        {
            {"_ts", _mts},
            {"app_version", _bp_app_version},
            {"device_code", _bp_device_code},
            {"market_id", _bp_market_id},
            {"oid", _oid},
            {"platform", _bp_platform},
            {"versioncode", _bp_versioncode}
        };
    QString MD5(const QString &str) const;
};

