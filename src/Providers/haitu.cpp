#include "haitu.h"

QList<ShowData> Haitu::search(QString query, int page, int type)
{
    QList<ShowData> shows;

    std::string url = hostUrl + "vodsearch/" + query.toStdString () + "----------" + std::to_string (page) + "---.html";
    qDebug() << QString::fromStdString (url);

    auto showNodes = NetworkClient::get(url).document().select("//div[@class='module-search-item']");
    for (pugi::xpath_node_set::const_iterator it = showNodes.begin(); it != showNodes.end(); ++it)
    {
        auto img = it->selectFirst(".//img");
        QString title = img.attr("alt").as_string();
        QString coverUrl = img.attr("data-src").as_string();
        if (!coverUrl.startsWith ("http"))
        {
            coverUrl = QString::fromStdString (hostUrl) + coverUrl;
        }
        std::string link = it->selectFirst(".//div[@class='module-item-pic']/a").attr ("href").as_string ();
        shows.emplaceBack(title, link, coverUrl, this);
    }

    return shows;
}

QList<ShowData> Haitu::popular(int page, int type)
{
    std::string url = hostUrl + "vodshow/" + std::to_string (type) + "--hits------" + std::to_string(page) + "---.html";
    QList<ShowData> shows = filterSearch(url);
    return shows;
}

QList<ShowData> Haitu::latest(int page, int type)
{
    std::string url = hostUrl + "vodshow/" + std::to_string (type) + "--time------" + std::to_string(page) + "---.html";
    QList<ShowData> shows = filterSearch(url);


    return shows;
}

QList<ShowData> Haitu::filterSearch(const std::string &url)
{
    QList<ShowData> shows;
    auto showNodes = NetworkClient::get(url).document().select("//div[@class='module-item']");
    for (pugi::xpath_node_set::const_iterator it = showNodes.begin(); it != showNodes.end(); ++it)
    {
        auto img = it->selectFirst(".//div[@class='module-item-pic']/img");
        QString title = img.attr("alt").as_string();
        QString coverUrl = img.attr("data-src").as_string();
        if(coverUrl.startsWith ('/'))
        {
            coverUrl = QString::fromStdString (hostUrl) + coverUrl;
        }
        qDebug() << title <<coverUrl;
        std::string link = it->selectFirst (".//div[@class='module-item-pic']/a").attr ("href").as_string ();
        QString latestText = it->selectFirst (".//div[@class='module-item-text']").node ().child_value ();
        shows.emplaceBack(title, link, coverUrl, this, latestText);
    }

    return shows;
}

void Haitu::loadDetails(ShowData &show) const
{
    auto doc = NetworkClient::get(hostUrl + show.link).document ();
    auto episodeNodes = doc.select ("//div[@class='scroll-content']/a");
    for (pugi::xpath_node_set::const_iterator it = episodeNodes.begin(); it != episodeNodes.end(); ++it)
    {
        QString title = QString::fromStdString (it->selectFirst(".//span").node ().child_value ()).trimmed ();
        int number = -1;
        bool ok;
        int intTitle = title.toInt (&ok);
        if (ok)
        {
            number = intTitle;
            title = "";
        }
        std::string link = it->attr ("href").as_string ();
        show.addEpisode(number, link, title);
    }
    show.description = QString(doc.selectFirst ("//div[@class='video-info-item video-info-content vod_content']/span").node ().child_value ());
}

QList<VideoServer> Haitu::loadServers(const PlaylistItem *episode) const
{
    return QList<VideoServer>{{"default",episode->link}};;
}

QString Haitu::extractSource(const VideoServer &server) const
{
    std::string response = NetworkClient::get(hostUrl + server.link).body;
    std::smatch match;
    if (!std::regex_search(response, match, player_aaaa_regex))
        throw "Failed to extract m3u8";
    std::string matchedText = match[1].str();
    auto url = nlohmann::json::parse(matchedText)["url"].get<std::string>();
    qDebug()<<QString::fromStdString (url);
    return QString::fromStdString (url);
}
