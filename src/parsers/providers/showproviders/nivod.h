#ifndef NIVOD_H
#define NIVOD_H

#include "parsers/showprovider.h"
#include "network/client.h"
#include "tools/functions.h"

class Nivod: public ShowProvider
{
public:
    Nivod(){};
    int providerEnum() override{
        return Providers::Nivod;
    }
    QString name() override {return "泥巴影院";}
    std::string hostUrl() override {return "https://www.nivod4.tv/";}

    QVector<ShowData> search(QString query, int page, int type) override{
        if(query.isEmpty ())return QVector<ShowData>();
        std::map<std::string, std::string> data = {
            {"keyword", query.toStdString ()},
            {"start", std::to_string((page - 1) * 20)},
            {"cat_id", "1"},
            {"keyword_type", "0"}
        };
        auto results = search(data);
        m_currentPage = page;
        lastSearch = [query,type,this]{
            return search(query,++m_currentPage,type);
        };
        return results;
    };

    QVector<ShowData> search(std::map<std::string, std::string> data){
        filterSearched=false;
        std::string response = callAPI("https://api.nivodz.com/show/search/WEB/3.2", data);
        return showsFromJsonArray (nlohmann::json::parse(response)["list"]);
    };

    QVector<ShowData> filterSearch(int page, std::string sortBy,int type,std::string regionId = "0",std::string langId="0",std::string yearRange=" ") {
        std::string channel;
        if(type == ShowData::DOCUMENTARY){
            channel = "6";
        }else{
            channel = std::to_string (type);
        }
        m_currentPage=page;
        std::map<std::string, std::string> data = {
            {"sort_by", sortBy},
            {"channel_id", channel},
            {"show_type_id", "0"},
            {"region_id", "0"},
            {"lang_id", "0"},
            {"year_range", " "},
            {"start", std::to_string((m_currentPage - 1) * 20)}
        };
        std::string response = callAPI("https://api.nivodz.com/show/filter/WEB/3.2", data);
        auto results = showsFromJsonArray(nlohmann::json::parse(response)["list"]);
        m_currentPage = page;
        lastSearch = [sortBy,type,regionId,langId,yearRange,this]{
            return filterSearch(++m_currentPage,sortBy,type,regionId,langId,yearRange);
        };
        return results;
    }

    QVector<ShowData> popular(int page, int type)override {
        return filterSearch(page,"1",type);
    }

    QVector<ShowData> latest(int page, int type)override {
        return filterSearch(page,"4",type);
    }

    ShowData loadDetails(ShowData show) override{
        nlohmann::json infoJson = getInfoJson (show);
        show.description = QString::fromStdString (infoJson["showDesc"].get<std::string>());
        if(infoJson.contains ("episodesUpdateDesc")&&!infoJson.at ("episodesUpdateDesc").is_null ()){
            show.updateTime = QString::fromStdString (infoJson["episodesUpdateDesc"].get<std::string>());
        }
        if(!infoJson["showTypeName"].is_null ())
            show.genres += QString::fromStdString(infoJson["showTypeName"].get<std::string>());
        //        auto actors = QString::fromStdString(infoJson["actors"].get<std::string>());

        if(!infoJson["hot"].is_null())
            show.views = QString::number(infoJson["hot"].get<int>());
        if(!infoJson["rating"].is_null())
            show.rating = QString::number ((infoJson["rating"].get<int>())/10);

        for (const auto& item : infoJson["plays"].items()){
            auto episode = item.value ();
            int number = episode["episodeId"].get<int>();
            QString title = QString::fromStdString (episode["displayName"].get<std::string>());
            std::string link = show.link.toStdString ()+ "&"+episode["playIdCode"].get<std::string>();
            show.episodes.append(Episode(number,QString::fromStdString (link),title));
        }
        return show;
    };

    nlohmann::json getInfoJson(const ShowData &show){
        auto response = callAPI("https://api.nivodz.com/show/detail/WEB/3.2", {{"show_id_code", show.link.toStdString ()}});
        return nlohmann::json::parse(response)["entity"];
    }

    int getEpisodeCount(const ShowData &show){
        return getInfoJson (show)["plays"].size ();
    }

    QVector<VideoServer> loadServers(const Episode& episode) override{
        return QVector<VideoServer>{VideoServer{"default",episode.link.toStdString (),{{"referer","https://www.nivod.tv/"}}}};
    };

    QString extractSource(VideoServer& server) override{
        auto codes = Functions::split(server.link,'&');
        std::map<std::string, std::string> data = {
            {"play_id_code", codes[1]},
            {"show_id_code", codes[0]},
            {"oid","1"},
            {"episode_id","0"}
        };
        auto playUrl=nlohmann::json::parse(callAPI("https://api.nivodz.com/show/play/info/WEB/3.3", data))["entity"]["plays"][0]["playUrl"].get <std::string>();//video quality
        return QString::fromStdString (playUrl);
    };

    QVector<ShowData> fetchMore() override{
        return lastSearch();
    }
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
    const std::string _oid = "8c387951eff11000ef5216f0e7cdca70956c120f20e90d2f";
    std::string _mts = "1679142478147";//QString::number(QDateTime::currentMSecsSinceEpoch()).toStdString ();
    const std::map<std::string, std::string> queryMap = {
        {"_ts", _mts},
        {"app_version", _bp_app_version},
        {"device_code", _bp_device_code},
        {"market_id", _bp_market_id},
        {"oid", _oid},
        {"platform", _bp_platform},
        {"versioncode", _bp_versioncode}
    };
    bool filterSearched = false;
//    std::map<std::string, std::string> lastSearch;
    const std::map<std::string, std::string> mudvodHeaders = {{"referer", "https://www.nivod4.tv"}};
//    const QMap<int, std::string> channelId = {
//        {ShowType::Movie, "1"},
//        {ShowType::TvSeries, "2"},
//        {ShowType::Reality, "3"},
//        {ShowType::Anime, "4"},
//        {ShowType::Documentary, "6"}
//    };
private:
    std::string createSign(const std::map<std::string, std::string> & bodyMap, const std::string& secretKey = "2x_Give_it_a_shot");
    std::string decryptedByDES(const std::string& input);
private:
    QVector<ShowData> showsFromJsonArray(const nlohmann::json& showList,int type = 0) {
        QVector<ShowData> results;
        for (auto& el : showList.items())
        {
            auto item=el.value ();
            ShowData show;

            int tvType = ShowData::ANIME;
            std::string channelName = item["channelName"].get<std::string>();
            if ( channelName== "电影") {
                tvType = ShowData::MOVIE;
            } else if (channelName == "电视剧") {
                tvType = ShowData::TVSERIES;
            } else if (channelName == "综艺") {
                tvType = ShowData::VARIETY;
            } else if (channelName == "动漫") {
                tvType = ShowData::ANIME;
            } else if (channelName == "纪录片") {
                tvType = ShowData::DOCUMENTARY;
            }
            show.type = tvType;
            std::string showImg = item["showImg"].get<std::string>();
            if(showImg.back () != 'g') showImg += "_300x400.jpg";
            //            if(item["showTitle"].get<std::string>().find ("假面骑士"));
            show.title = QString::fromStdString (item["showTitle"].get<std::string>());
            show.coverUrl = QString::fromStdString (showImg);
            show.link = QString::fromStdString (item["showIdCode"].get<std::string>());
            if(!item["episodesTxt"].is_null ()){
                show.latestTxt = QString::fromStdString (item["episodesTxt"].get<std::string>());
            }
            show.provider=Providers::Nivod;
            if(!item["postYear"].is_null ()){
                show.releaseDate = QString::number(item["postYear"].get<int>());
            }
            //            Status status = Status::Completed;
            //            if (ShowData.latestTxt.contains("更新")) {
            //                status = Status::Ongoing;
            //            }

            results.push_back (std::move(show));
        }
        m_canFetchMore=!results.isEmpty ();
        return results;
    }

    std::string callAPI(const std::string& url, const std::map<std::string, std::string>& data){
        std::string sign = createSign(data);
        std::string postUrl = url + "?_ts=" + _mts + "&app_version=1.0&platform=3&market_id=web_nivod&device_code=web&versioncode=1&oid=" + _oid + "&sign=" + sign;
        auto response = client.post(postUrl,mudvodHeaders,data).body;
        //        qDebug()<<QString::fromStdString (postUrl);
//        while(response.size () == 0){
//            response = client.post(postUrl,mudvodHeaders,data).body;
//            qDebug()<<"Bad Response";
//        }
        try{
            return decryptedByDES(response);
        }catch(std::exception& e){
            qDebug()<<"Bad Input";
            qDebug()<<QString::fromStdString (postUrl);
            qDebug()<<QString::fromStdString (response);
        }
        return "";

    }
};

#endif // NIVOD_H
