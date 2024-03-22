#pragma once
#include "showprovider.h"
#include "Explorer/Data/showdata.h"

class Nivod: public ShowProvider
{
public:
    Nivod(){};
    QString name() const override {return "泥巴影院";}
    std::string hostUrl = "https://www.nivod4.tv/";
    QList<int> getAvailableTypes() const override {
        return {ShowData::ANIME, ShowData::MOVIE, ShowData::TVSERIES, ShowData::VARIETY, ShowData::DOCUMENTARY};
    };

    QList<ShowData> search(QString query, int page, int type) override;
    QList<ShowData> popular(int page, int type) override;
    QList<ShowData> latest(int page, int type) override;

    void loadDetails(ShowData& show) const override;
    QList<VideoServer> loadServers(const PlaylistItem* episode) const override;
    int getTotalEpisodes(const std::string& link) const override;
    QString extractSource(const VideoServer& server) const override;

private:
    QMap<QString, QString> objKeySort(const QMap<QString, QString>& inputMap) const;
    nlohmann::json getInfoJson(const std::string& link) const;
    std::string createSign(const std::map<std::string, std::string>& bodyMap, const std::string& secretKey = "2x_Give_it_a_shot") const;
    std::string decryptedByDES(const std::string& input) const;
    int getShowType(const std::string& channelName) const;
    QList<ShowData> showsFromJsonArray(const nlohmann::json& showList,int type = 0);
    QList<ShowData> filterSearch(int page, const QString& sortBy,int type, const QString& regionId = "0", const QString& langId="0", const QString& yearRange = " ");
    std::string callAPI(const std::string& url, const std::map<std::string, std::string>& data) const;
private:
    const std::string _HOST_CONFIG_KEY = "2x_Give_it_a_shot";
    const std::string _bp_app_version = "1.0";
    const std::string _bp_platform = "3";
    const std::string _bp_market_id = "web_nivod";
    const std::string _bp_device_code = "web";
    const std::string _bp_versioncode = "1";
    const std::string _QUERY_PREFIX = "__QUERY::";
    const std::string _BODY_PREFIX = "__BODY::";
    const std::string _SECRET_PREFIX = "__KEY::";
    const std::string _oid = "a376c2407d77f46d0a7af5e4f20e213b67af3d346690b805";
    std::string _mts = "1690816442085";//std::to_string(QDateTime::currentMSecsSinceEpoch()).toStdString ();
    const std::map<std::string, std::string> queryMap =
        {
        {"_ts", _mts},
        {"app_version", _bp_app_version},
        {"device_code", _bp_device_code},
        {"market_id", _bp_market_id},
        {"oid", _oid},
        {"platform", _bp_platform},
        {"versioncode", _bp_versioncode}
    };
    bool filterSearched = false;
    const std::map<std::string, std::string> mudvodHeaders = {{"referer", "https://www.nivod4.tv"}};
    //    const QMap<int, std::string> channelId = {
    //        {ShowType::Movie, "1"},
    //        {ShowType::TvSeries, "2"},
    //        {ShowType::Reality, "3"},
    //        {ShowType::Anime, "4"},
    //        {ShowType::Documentary, "6"}
    //    };
};

