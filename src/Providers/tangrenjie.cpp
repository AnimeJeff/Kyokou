
#include "tangrenjie.h"

QVector<ShowData> Tangrenjie::search(QString query, int page, int type)
{
    std::string url = "https://www.tangrenjie.tv/vod/search/page/"+ std::to_string (page) + "/wd/" + Functions::urlEncode (query.toStdString ()) + ".html";
    auto results = selectShow(url);
    m_currentPage = page;
    lastSearch = [query,type,this](int page){
        return search(query, page, type);
    };
    m_canFetchMore=!results.empty ();
    return results;
}

QVector<ShowData> Tangrenjie::latest(int page, int type)
{
    return filterSearch(page,type,"time");
}

QVector<ShowData> Tangrenjie::popular(int page, int type)
{
    return filterSearch(page,type,"hits");
}

QVector<ShowData> Tangrenjie::selectShow(const std::string& url) const
{
    auto resp = NetworkClient::get(url);
    auto doc = resp.document ();
    pugi::xpath_node_set showNodes = doc.select("//a[@class='vodlist_thumb lazyload']");
    QVector<ShowData> shows;
    for (pugi::xpath_node_set::const_iterator it = showNodes.begin (); it != showNodes.end(); ++it)
    {
        QString title = it->attr ("title").as_string ();
        std::string link = it->attr ("href").as_string ();
        QString coverUrl = it->attr("data-original").as_string ();
        if(coverUrl.startsWith ("/")){
            coverUrl = QString::fromStdString (hostUrl) + coverUrl;
        }
        auto latestTxt = it->selectFirst (".//span[@class='pic_text text_right']").node ().child_value ();
        shows.emplaceBack (ShowData(title, link, coverUrl, name (), latestTxt));
    }

    return shows;
}

QVector<ShowData> Tangrenjie::filterSearch(int page, int type, const std::string &sortBy, const std::string &area, const std::string &year, const std::string &language)
{
    std::string url = "https://www.tangrenjie.tv/vod/show";
    if(!area.empty ()){
        url += "/area/" + area;
    }
    url += "/by/" + sortBy;
    std::string id;
    if(type == ShowData::DOCUMENTARY)
    {
        id = "28";
    }
    else
    {
        id = std::to_string (type);
    }
    url += "/id/" + std::to_string (type);

    if(!language.empty ())
    {
        url += "/lang/" + language;
    }
    url += "/page/" + std::to_string (page);

    if(!year.empty ())
    {
        url += "/year/" + year;
    }
    url += ".html";

    QVector<ShowData> results = selectShow(url);
    m_canFetchMore = !results.isEmpty ();
    if (m_canFetchMore) m_currentPage = page;
    lastSearch = [sortBy,type,area,year,language,this](int page){
        return filterSearch(page,type, sortBy, area,year,language);
    };
    return results;
}

void Tangrenjie::loadDetails(ShowData& show) const
{
    CSoup doc = NetworkClient::get(hostUrl + show.link ).document ();
    show.description = doc.selectText("//div[@class='content']");
    int count = 1;
    for (const auto& element:doc.select("//ul[@class='content_playlist list_scroll clearfix']/li/a")){
        bool isInt;
        QString name = element.node ().child_value();
        int number = name.toInt (&isInt);
        std::string link = element.attr ("href").as_string ();
        if(!isInt){
            number = count;
        }else{
            name.clear ();
        }
        count++;
//        show.episodes.emplace_back (Episode(number,link,title));
        show.addEpisode (number,link,name);
        show.totalEpisodes++;
    }
}

int Tangrenjie::getTotalEpisodes(const ShowData &show) const
{
    CSoup doc = NetworkClient::get(hostUrl + show.link).document ();
    return doc.select("//ul[@class='content_playlist list_scroll clearfix']/li/a").size ();
}

QVector<VideoServer> Tangrenjie::loadServers(const PlaylistItem *episode) const
{
    VideoServer server;
    server.name = "player_aaaa";
    server.link = episode->link;
    return QVector<VideoServer>{server};
}

QString Tangrenjie::extractSource(VideoServer &server) const
{
    //qDebug()<<QString::fromStdString (hostUrl+server.link);
    std::string response = NetworkClient::get(hostUrl + server.link).body;
    std::smatch match;
    if (!std::regex_search(response, match, player_aaaa_regex))
        throw "Failed to extract m3u8";
    std::string matchedText = match[1].str();
    auto url = Functions::urlDecode (Functions::base64Decode (nlohmann::json::parse(matchedText)["url"].get<std::string>()));
    return QString::fromStdString (url);
}

