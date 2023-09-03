	
#ifndef HAITU_H
#define HAITU_H
#include <regex>
#include "parsers/showprovider.h"

class Haitu : public ShowProvider
{
    std::regex player_aaaa_regex{R"(player_aaaa=(\{.*?\})</script>)"};
public:
    explicit Haitu(QObject *parent = nullptr):ShowProvider{parent}{

    };
public:
    int providerEnum(){ return Providers::HAITU; }
    QString name(){ return "海兔影院"; }
    std::string hostUrl(){ return "https://www.haitu.tv"; }

    QVector<ShowData> search(QString query, int page, int type){
        QVector<ShowData> shows;
        std::string url = hostUrl() + "/vod/search/page/"+std::to_string (page)+"/wd/"+query.toStdString ()+".html";
        auto showNodes = NetworkClient::get(url).document().select("//div[@class='module-search-item']");
        for (pugi::xpath_node_set::const_iterator it = showNodes.begin(); it != showNodes.end(); ++it)
        {
            auto img = it->selectFirst(".//img");
            QString title = img.attr("alt").as_string();
            QString coverUrl = QString::fromStdString (hostUrl()) + img.attr("data-src").as_string();
            std::string link = it->selectFirst(".//div[@class='module-item-pic']/a").attr ("href").as_string ();
            shows.emplaceBack(ShowData(title,link,coverUrl,Providers::HAITU));
        }
        return shows;
    }
    QVector<ShowData> popular(int page, int type){
        std::string url = hostUrl() + "/vod/show/by/hits/id/4/page/" + std::to_string(page) + ".html";
        return filterSearch (url);
    }
    QVector<ShowData> latest(int page, int type){
        std::string url = hostUrl() + "/vod/show/by/time/id/4/page/" + std::to_string(page) + ".html";
        return filterSearch (url);
    }
    QVector<ShowData> filterSearch(const std::string& url){
        QVector<ShowData> shows;
        auto showNodes = NetworkClient::get(url).document().select("//div[@class='module-item']");
        for (pugi::xpath_node_set::const_iterator it = showNodes.begin(); it != showNodes.end(); ++it)
        {
            auto img = it->selectFirst(".//div[@class='module-item-pic']/img");
            QString title = img.attr("alt").as_string();
            QString coverUrl = QString::fromStdString (hostUrl()) + img.attr("data-src").as_string();
            qDebug() << title <<coverUrl;
            std::string link = it->selectFirst (".//div[@class='module-item-pic']/a").attr ("href").as_string ();
            QString latestText = it->selectFirst (".//div[@class='module-item-text']").node ().child_value ();
            shows.emplaceBack(ShowData(title,link,coverUrl,Providers::HAITU,latestText));
        }

        return shows;
    }

    ShowData loadDetails(ShowData show){
        auto doc = NetworkClient::get(hostUrl() + show.link).document ();
//        show.episodes = getEpisodes (getEpisodesLink(doc));
//        qDebug()<<NetworkClient::get(hostUrl() + show.link);
        auto episodeNodes = doc.select ("//div[@class='scroll-content']/a");
        for (pugi::xpath_node_set::const_iterator it = episodeNodes.begin(); it != episodeNodes.end(); ++it)
        {

            int number = 1;//std::stoi(it->node ().child_value ());
            qDebug()<<it->selectFirst(".//span").node ().child_value ();
            std::string link = it->attr ("href").as_string ();
            show.episodes.emplaceBack (Episode {number,link,it->attr ("title").as_string ()} );
        }
        show.description = QString(doc.selectFirst ("//div[@class='video-info-item video-info-content vod_content']/span").node ().child_value ());
//        show.status = doc.selectFirst ("//span[contains(text(),'Status')]/following-sibling::a").node ().child_value ();
//        pugi::xpath_node_set genreNodes = doc.select ("//span[contains(text(),'Genre')]/following-sibling::a");
//        for (pugi::xpath_node_set::const_iterator it = genreNodes.begin(); it != genreNodes.end(); ++it)
//        {
//            QString genre = QString(it->attr ("title").as_string ()).replace ("\n"," ");
//            anime.genres.push_back (genre);
//        }
//        anime.totalEpisodes = anime.episodes.count ();
        return show;
    }
    int getTotalEpisodes(const ShowData &show){
        return 0;
    }
    QVector<VideoServer> loadServers(const Episode &episode){
        QVector<VideoServer> servers{{"",episode.link}};

        return servers;
    }
    QString extractSource(VideoServer &server){
        std::string response = NetworkClient::get(hostUrl() + server.link).body;
        std::smatch match;
        if (!std::regex_search(response, match, player_aaaa_regex))
            throw "Failed to extract m3u8";
        std::string matchedText = match[1].str();
        auto url = nlohmann::json::parse(matchedText)["url"].get<std::string>();
        qDebug()<<QString::fromStdString (url);
        return QString::fromStdString (url);
    }
};

#endif // HAITU_H
