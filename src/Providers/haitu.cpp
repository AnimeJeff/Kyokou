#include "haitu.h"
#include <pugixml/pugixml.hpp>
QList<ShowData> Haitu::search(QString query, int page, int type)
{
    // QList<ShowData> shows;

    // std::string url = hostUrl + "vodsearch/" + query.toStdString () + "----------" + std::to_string (page) + "---.html";
    // auto showNodes = NetworkClient::get(url).document().select("//div[@class='module-items']/div");
    // for (pugi::xpath_node_set::const_iterator it = showNodes.begin(); it != showNodes.end(); ++it)
    // {
    //     auto img = it->selectFirst(".//img");
    //     QString title = img.attr("alt").as_string();
    //     QString coverUrl = img.attr("data-src").as_string();
    //     if (!coverUrl.startsWith ("http"))
    //     {
    //         coverUrl = QString::fromStdString (hostUrl) + coverUrl;
    //     }
    //     std::string link = it->selectFirst(".//div[@class='module-item-pic']/a").attr ("href").as_string ();
    //     shows.emplaceBack(title, link, coverUrl, this);
    // }

    // return shows;
    query.replace (" ", "+");
    return filterSearch(QUrl::toPercentEncoding (query).toStdString (), "--", page);
}

QList<ShowData> Haitu::popular(int page, int type)
{
    return filterSearch(std::to_string (type), "hits", page);
}

QList<ShowData> Haitu::latest(int page, int type)
{
    return filterSearch(std::to_string (type), "time", page);
}

QList<ShowData> Haitu::filterSearch(const std::string &query, const std::string &sortBy, int page)
{

    std::string url = hostUrl + (sortBy == "--" ? "vodsearch/": "vodshow/") + query + "--" + sortBy + "------" + std::to_string(page) + "---.html";
    // qDebug() << QString::fromStdString (url);
    auto showNodes = NetworkClient::get(url).document().select("//div[@class='module-list']/div[@class='module-items']/div");
    QList<ShowData> shows;

    for (pugi::xpath_node_set::const_iterator it = showNodes.begin(); it != showNodes.end(); ++it)
    {
        auto img = it->selectFirst(".//div[@class='module-item-pic']/img");
        QString title = img.attr("alt").as_string();
        QString coverUrl = img.attr("data-src").as_string();
        if(coverUrl.startsWith ('/')) {
            coverUrl = QString::fromStdString (hostUrl) + coverUrl;
        }
        // qDebug() << title <<coverUrl;
        std::string link = it->selectFirst (".//div[@class='module-item-pic']/a").attr ("href").as_string ();
        QString latestText;
        int type = -1;

        if (sortBy == "--"){
            latestText = it->selectFirst (".//a[@class='video-serial']").node ().child_value ();
        } else {
            latestText = it->selectFirst (".//div[@class='module-item-text']").node ().child_value ();
            type = std::stoi (query);
        }

        shows.emplaceBack(title, link, coverUrl, this, latestText, type);
    }

    return shows;
}

void Haitu::loadDetails(ShowData &show) const
{
    auto doc = NetworkClient::get(hostUrl + show.link).document ();

    auto infoItems = doc.select ("//div[@class='video-info-items']/div");
    show.releaseDate = infoItems[2].node ().child_value ();
    show.updateTime = infoItems[3].node ().child_value ();
    show.updateTime = show.updateTime.split ("，").first ();
    show.status = infoItems[4].node ().child_value ();
    show.score = infoItems[5].selectText (".//font");
    show.description = QString(infoItems[7].selectText (".//span")).trimmed ();
    auto genreNodes = doc.select ("//div[@class='tag-link']/a");
    for (pugi::xpath_node_set::const_iterator it = genreNodes.begin(); it != genreNodes.end(); ++it) {
        show.genres += it->node ().child_value ();
    }

    pugi::xpath_node_set serverNodes = doc.select ("//div[@class='scroll-content']");
    if (serverNodes.empty ()) return;
    pugi::xpath_node_set serverNamesNode = doc.select("//div[@class='module-heading']//div[@class='module-tab-content']/div");

    Q_ASSERT (serverNamesNode.size () == serverNodes.size ());
    serverNodes.sort (true);
    serverNamesNode.sort (true);

    PlaylistItem *playlist = nullptr;
    QMap<float, QString> episodesMap;
    // bool makeEpisodesHash = show.type == 2 || show.type == 4;

    for (int i = 0; i < serverNodes.size (); i++) {
        pugi::xpath_node serverNode = serverNodes[i];
        QString serverName = serverNamesNode[i].attr ("data-dropdown-value").as_string ();
        //qDebug() << "serverName" << QString::fromStdString (serverName);
        pugi::xpath_node_set episodeNodes = serverNode.node ().select_nodes (".//a");
        //qDebug() << "episodes" << episodeNodes.size ();

        for (pugi::xpath_node_set::const_iterator it = episodeNodes.begin(); it != episodeNodes.end(); ++it)
        {
            QString title = QString::fromStdString (it->selectFirst(".//span").node ().child_value ());
            static auto replaceRegex = QRegularExpression("[第集话完结期]");
            title = title.replace (replaceRegex,"").trimmed ();
            bool ok;
            float intTitle = title.toFloat (&ok);
            float number = -1;
            if (ok) {
                number = intTitle;
                title.clear ();
            }
            QString link = it->attr ("href").as_string ();
            // qDebug() << "link" << QString::fromStdString (link);

            if (number > -1){
                if (!episodesMap[number].isEmpty ()) episodesMap[number] += ";";
                episodesMap[number] +=  serverName + " " + link;
            } else {
                show.addEpisode(number, serverName + " " + link, title);
            }


        }
    }

    for (auto [number, link] : episodesMap.asKeyValueRange()) {
        show.addEpisode (number, link,"");
    }



}

QList<VideoServer> Haitu::loadServers(const PlaylistItem *episode) const
{
    auto serversString = episode->link.split (";");
    QList<VideoServer> servers;
    for (auto& serverString: serversString) {
        auto serverNameAndLink = serverString.split (" ");
        QString serverName = serverNameAndLink.first ();
        QString serverLink = serverNameAndLink.last ();
        servers.emplaceBack (serverName, serverLink);
    }
    return servers;
}

QList<Video> Haitu::extractSource(const VideoServer &server) const
{
    std::string response = NetworkClient::get(hostUrl + server.link.toStdString ()).body;
    std::smatch match;
    if (!std::regex_search(response, match, player_aaaa_regex))
        throw "Failed to extract m3u8";
    // std::string matchedText = ;
    auto source = QJsonDocument::fromJson (match[1].str ().c_str ()).object ()["url"].toString ();
    // qDebug()<<url;

    return { Video(source) };
}
