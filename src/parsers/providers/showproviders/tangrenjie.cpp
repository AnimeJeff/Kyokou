
#include "tangrenjie.h"
#include "tools/functions.h"
QVector<ShowData> Tangrenjie::selectShow(const std::string& url){
    //    pugi::xpath_node_set showNodes = NetworkClient::get(url).document().select("//a[@class='vodlist_thumb lazyload']");
    auto resp = NetworkClient::get(url);
    auto doc = resp.document ();
//    auto body = resp.body;
    pugi::xpath_node_set showNodes = doc.select("//a[@class='vodlist_thumb lazyload']");
    QVector<ShowData> shows;
    for (pugi::xpath_node_set::const_iterator it = showNodes.begin (); it != showNodes.end(); ++it)
    {
        QString title = it->attr ("title").as_string ();
        std::string link = it->attr ("href").as_string ();
        QString coverUrl = it->attr("data-original").as_string ();
        if(coverUrl.startsWith ("/")){
            coverUrl = QString::fromStdString (hostUrl()) + coverUrl;
        }
        auto latestTxt = it->selectFirst (".//span[@class='pic_text text_right']").node ().child_value ();
        shows.emplace_back(ShowData(title,link,coverUrl,Providers::TANGRENJIE,latestTxt));
    }

    return shows;
}

QVector<ShowData> Tangrenjie::search(QString query, int page, int type) {
    std::string url = "https://www.tangrenjie.tv/vod/search/page/"+ std::to_string (page) + "/wd/" + Functions::urlEncode (query.toStdString ()) + ".html";
    auto results = selectShow(url);
    m_currentPage = page;
    lastSearch = [query,type,this]{
        return search(query,++m_currentPage,type);
    };
    m_canFetchMore=!results.empty ();
    return results;
}

QVector<ShowData> Tangrenjie::filterSearch(int page, int type, const std::string &sortBy, const std::string &area, const std::string &year, const std::string &language){
    std::string url = "https://www.tangrenjie.tv/vod/show";
    if(!area.empty ()){
        url += "/area/" + area;
    }
    url += "/by/" + sortBy;
    std::string id;
    if(type == ShowData::DOCUMENTARY){
        id = "28";
    }else{
        id = std::to_string (type);
    }
    url += "/id/" + std::to_string (type);
    if(!language.empty ()){
        url += "/lang/" + language;
    }
    url += "/page/" + std::to_string (page);
    if(!year.empty ()){
        url += "/year/" + year;
    }
    url += ".html";
    auto results = selectShow (url);
    if(results.empty ()){
        m_canFetchMore = false;
        return results;
    }
    m_currentPage = page;
    lastSearch = [sortBy,type,area,year,language,this]{
        return filterSearch(++m_currentPage,type, sortBy, area,year,language);
    };
    m_canFetchMore = true;
    return results;
}

ShowData Tangrenjie::loadDetails(ShowData show) {
    CSoup doc = NetworkClient::get(hostUrl() + show.link ).document ();
    show.description = doc.selectText("//div[@class='content']");
    int count = 1;
    for (const auto& element:doc.select("//ul[@class='content_playlist list_scroll clearfix']/li/a")){
        bool isInt;
        QString title = element.node ().child_value();
        int number = title.toInt (&isInt);
        std::string link = element.attr ("href").as_string ();
        if(!isInt){
            number = count;
        }else{
            title.clear ();
        }
        count++;
        show.episodes.emplace_back (Episode(number,link,title));
        show.totalEpisodes++;
    }
    return show;
}

int Tangrenjie::getTotalEpisodes(const ShowData &show) {
    CSoup doc = NetworkClient::get(hostUrl() + show.link).document ();
    return doc.select("//ul[@class='content_playlist list_scroll clearfix']/li/a").size ();
}

QVector<VideoServer> Tangrenjie::loadServers(const Episode &episode) {
    VideoServer server;
    server.name = "player_aaaa";
    server.link = episode.link;
    return QVector<VideoServer>{server};
}

QString Tangrenjie::extractSource(VideoServer &server) {
    //qDebug()<<QString::fromStdString (hostUrl()+server.link);
    std::string response = NetworkClient::get(hostUrl() + server.link).body;
    std::smatch match;
    if (!std::regex_search(response, match, player_aaaa_regex))
        throw "Failed to extract m3u8";
    std::string matchedText = match[1].str();
    auto url = Functions::urlDecode (Functions::base64Decode (nlohmann::json::parse(matchedText)["url"].get<std::string>()));
    return QString::fromStdString (url);
}

