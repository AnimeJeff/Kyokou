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
        return Providers::NIVOD;
    }
    QString name() override {return "泥巴影院";}

    std::string hostUrl = "https://www.nivod4.tv/";

    QVector<ShowData> filterSearch(int page, const QString& sortBy,int type,const QString& regionId = "0",const QString& langId="0",const QString& yearRange=" ") {
        std::string channel;
        if(type == ShowData::DOCUMENTARY){
            channel = "6";
        }else{
            channel = std::to_string (type);
        }
        m_currentPage = page;
        std::map<std::string, std::string> data = {
            {"sort_by", sortBy.toStdString ()},
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
        emit fetchedResults (results);
        return results;
    }

    QVector<ShowData> search(QString query, int page, int type) override;

    QVector<ShowData> popular(int page, int type)override {
        return filterSearch(page,"1",type);
    }

    QVector<ShowData> latest(int page, int type)override {
        auto results = filterSearch(page,"4",type);
//        emit results;
        return results;
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
            show.views = QString::fromStdString (std::to_string(infoJson["hot"].get<int>()));
        if(!infoJson["rating"].is_null())
            show.rating = QString::fromStdString (std::to_string ((infoJson["rating"].get<int>())/10));
        for (const auto& item : infoJson["plays"].items()){
            auto episode = item.value ();
            QString title = QString::fromStdString (episode["displayName"].get<std::string>());
            int number = -1;

            bool ok;
            int intTitle = title.toInt (&ok);
            if(ok){
                number = intTitle;
                title = "";
            }
            std::string link = show.link + "&" + episode["playIdCode"].get<std::string>();
            show.episodes.emplaceBack (number,link,title);
            show.totalEpisodes++;
        }
        return show;
    };

    QMap<QString, QString> objKeySort(const QMap<QString, QString>& inputMap) {
        if (inputMap.isEmpty())
            return inputMap;

        QMap<QString, QString> sortedMap;
        for (const auto& key : inputMap.keys()) {
            const QString& value = inputMap.value(key);

            if (key.isEmpty() || value.isEmpty() || key == "sign")
                continue;

            sortedMap.insert(key, value);
        }

        return sortedMap;
    }

    nlohmann::json getInfoJson(const ShowData &show){
        auto response = callAPI("https://api.nivodz.com/show/detail/WEB/3.3", {{"show_id_code", show.link},{"episode_id","0"}});
        //qDebug() << QString::fromStdString (response);
        return nlohmann::json::parse(response)["entity"];
    }

    int getTotalEpisodes(const ShowData &show) override {
        return getInfoJson (show)["plays"].size ();
    }

    QVector<VideoServer> loadServers(const Episode& episode) override{
        return QVector<VideoServer>{VideoServer{"default",episode.link,{{"referer","https://www.nivod.tv/"}}}};
    };

    QString extractSource(VideoServer& server) override{
        auto codes = Functions::split(server.link,'&');
        std::map<std::string, std::string> data = {
            {"play_id_code", codes[1]},
            {"show_id_code", codes[0]},
            {"oid","1"},
            {"episode_id","0"}
        };
        auto playUrl = nlohmann::json::parse(callAPI("https://api.nivodz.com/show/play/info/WEB/3.3", data))["entity"]["plays"][0]["playUrl"].get <std::string>();//video quality
        return QString::fromStdString (playUrl);
    };

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
    const std::map<std::string, std::string> mudvodHeaders = {{"referer", "https://www.nivod4.tv"}};
    //    const QMap<int, std::string> channelId = {
    //        {ShowType::Movie, "1"},
    //        {ShowType::TvSeries, "2"},
    //        {ShowType::Reality, "3"},
    //        {ShowType::Anime, "4"},
    //        {ShowType::Documentary, "6"}
    //    };
private:
    std::string createSign(const std::map<std::string, std::string>& bodyMap, const std::string& secretKey = "2x_Give_it_a_shot");
    std::string decryptedByDES(const std::string& input);
private:
    int getShowType(const std::string& channelName){
        if ( channelName== "电影") {
            return ShowData::MOVIE;
        } else if (channelName == "电视剧") {
            return ShowData::TVSERIES;
        } else if (channelName == "综艺") {
            return ShowData::VARIETY;
        } else if (channelName == "动漫") {
            return ShowData::ANIME;
        } else if (channelName == "纪录片") {
            return ShowData::DOCUMENTARY;
        }
        qDebug() << "Cannot infer show type from" << QString::fromStdString (channelName);
        return ShowData::NONE;
    }

    QVector<ShowData> showsFromJsonArray(const nlohmann::json& showList,int type = 0) {
        QVector<ShowData> results;
        for (auto& el : showList.items())
        {
            auto item = el.value ();
            int tvType = getShowType(item["channelName"].get<std::string>());
            //            if(item["showTitle"].get<std::string>().find ("假面骑士"));
            QString title = QString::fromStdString (item["showTitle"].get<std::string>());
            QString coverUrl = QString::fromStdString (item["showImg"].get<std::string>());
            std::string link = item["showIdCode"].get<std::string>();
            results.push_back (ShowData{ title,link,coverUrl,Providers::NIVOD,"", tvType });
            if(!item["episodesTxt"].is_null ()){
                results.last ().latestTxt = QString::fromStdString (item["episodesTxt"].get<std::string>());
                //                int status = ShowData::Status::Completed;
                //                if (results.last ().latestTxt.contains("更新")) {
                //                    results.last ().status = ShowData::Status::Ongoing;
                //                }
            }
            if(!item["postYear"].is_null ()){
                results.last ().releaseDate = QString::fromStdString (std::to_string(item["postYear"].get<int>()));
            }
        }
        m_canFetchMore=!results.isEmpty ();
        return results;
    }

    std::string callAPI(const std::string& url, const std::map<std::string, std::string>& data){
        std::string sign = createSign(data);
        std::string postUrl = url + "?_ts=" + _mts + "&app_version=1.0&platform=3&market_id=web_nivod&device_code=web&versioncode=1&oid=" + _oid + "&sign=" + sign;
        auto response = NetworkClient::post(postUrl,mudvodHeaders,data).body;
        //qDebug()<<QString::fromStdString (NetworkClient::post (postUrl,{{"referer","https://nivod4.tv"}},{{"episode_id","0"},{"show_id_code","WPqWVrWnwsrG8sH4l24qEO2UX4Ak5xgO"}}).body);
        //qDebug() << postUrl;
        //qDebug() << data;
        //        while(response.size () == 0){
        //            response = NetworkClient::post(postUrl,mudvodHeaders,data).body;
        //            qDebug()<<"Bad Response";
        //        }
        try{
            return decryptedByDES(response);
        }catch(std::exception& e){
            qDebug() << "Bad Input";
            qDebug() << postUrl;
            //            qDebug()<<response;
        }
        return "";
    }
};

#endif // NIVOD_H
