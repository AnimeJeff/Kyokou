// #include "tangrenjie.h"

// QList<ShowData> Tangrenjie::search(QString query, int page, int type)
// {
//     std::string url = "https://www.tangrenjie.tv/vod/search/page/"+ std::to_string (page) + "/wd/" + Functions::urlEncode (query.toStdString ()) + ".html";
//     auto results = selectShow(url);
//     m_currentPage = page;
//     lastSearch = [query,type,this](int page){
//         return search(query, page, type);
//     };
//     return results;
// }

// QList<ShowData> Tangrenjie::latest(int page, int type)
// {
//     return filterSearch(page,type,"time");
// }

// QList<ShowData> Tangrenjie::popular(int page, int type)
// {
//     return filterSearch(page,type,"hits");
// }

// QList<ShowData> Tangrenjie::selectShow(const std::string& url) const
// {
//     auto resp = NetworkClient::get(url);
//     auto doc = resp.document ();
//     pugi::xpath_node_set showNodes = doc.select("//a[@class='vodlist_thumb lazyload']");
//     QList<ShowData> shows;
//     for (pugi::xpath_node_set::const_iterator it = showNodes.begin (); it != showNodes.end(); ++it)
//     {
//         QString title = it->attr ("title").as_string ();
//         std::string link = it->attr ("href").as_string ();
//         QString coverUrl = it->attr("data-original").as_string ();
//         if (coverUrl.startsWith ("/")){
//             coverUrl = QString::fromStdString (hostUrl) + coverUrl;
//         }
//         auto latestTxt = it->selectFirst (".//span[@class='pic_text text_right']").node ().child_value ();
//         shows.emplaceBack (ShowData(title, link, coverUrl, name (), latestTxt));
//     }

//     return shows;
// }

// QList<ShowData> Tangrenjie::filterSearch(int page, int type, const std::string &sortBy, const std::string &area, const std::string &year, const std::string &language)
// {
//     std::string url = "https://www.tangrenjie.tv/vod/show";
//     if (!area.empty ()){
//         url += "/area/" + area;
//     }
//     url += "/by/" + sortBy;
//     std::string id;
//     if (type == ShowData::DOCUMENTARY)
//     {
//         id = "28";
//     }
//     else
//     {
//         id = std::to_string (type);
//     }
//     url += "/id/" + std::to_string (type);

//     if (!language.empty ())
//     {
//         url += "/lang/" + language;
//     }
//     url += "/page/" + std::to_string (page);

//     if (!year.empty ())
//     {
//         url += "/year/" + year;
//     }
//     url += ".html";

//     QList<ShowData> results = selectShow(url);
//     lastSearch = [sortBy,type,area,year,language,this](int page){
//         return filterSearch(page,type, sortBy, area,year,language);
//     };
//     return results;
// }

// void Tangrenjie::loadDetails(ShowData& show) const
// {
//     CSoup doc = NetworkClient::get(hostUrl + show.link ).document ();
//     show.description = doc.selectText("//div[@class='content']");
//     for (const auto& element:doc.select("(//ul[@class='content_playlist list_scroll clearfix'])[2]/li/a")){
//         QString title = element.node ().child_value();
//         int number = -1;
//         bool ok;
//         int intTitle = title.toInt (&ok);
//         if (ok)
//         {
//             number = intTitle;
//             title = "";
//         }
//         std::string link = element.attr ("href").as_string ();
//         show.addEpisode (number,link,title);
//     }
// }

// int Tangrenjie::getTotalEpisodes(const std::string& link) const
// {
//     CSoup doc = NetworkClient::get(hostUrl + link).document ();
//     return doc.select("//ul[@class='content_playlist list_scroll clearfix']/li/a").size ();
// }

// QList<VideoServer> Tangrenjie::loadServers(const PlaylistItem *episode) const
// {
//     VideoServer server;
//     server.name = "player_aaaa";
//     server.link = episode->link;
//     return QList<VideoServer>{server};
// }

// QString Tangrenjie::extractSource(VideoServer &server) const
// {
//     //qDebug()<<QString::fromStdString (hostUrl+server.link);
//     std::string response = NetworkClient::get(hostUrl + server.link).body;
//     std::smatch match;
//     if (!std::regex_search(response, match, player_aaaa_regex))
//         throw "Failed to extract m3u8";
//     std::string matchedText = match[1].str();
//     auto url = Functions::urlDecode (Functions::base64Decode (nlohmann::json::parse(matchedText)["url"].get<std::string>()));
//     return QString::fromStdString (url);
// }

