#ifndef GOGOANIME_H
#define GOGOANIME_H

#include <QDebug>
#include "parsers/showprovider.h"
#include "network/client.h"
#include "parsers/extractors/gogocdn.h"
#include "tools/MyException.h"
#include "tools/functions.h"

class Gogoanime : public ShowProvider
{
public:
    int providerEnum() override{
        return Providers::GOGOANIME;
    }
    Gogoanime() = default;
    QString name() override{
        return "Gogoanime";
    }
    std::string hostUrl() override {
        return "https://gogoanimehd.to";
    }

    QVector<ShowData> search(QString query, int page, int type=0) override{
        QVector<ShowData> animes;
        if(query.isEmpty ()){
            m_canFetchMore = false;
            return animes;
        }
        std::string url = hostUrl() + "/search.html?keyword=" + Functions::urlEncode (query.toStdString ())+ "&page="+ std::to_string (page);
        auto animeNodes = NetworkClient::get(url).document().select("//ul[@class='items']/li");
        if(animeNodes.empty ()){
            m_canFetchMore = false;
            return animes;
        }
        for (pugi::xpath_node_set::const_iterator it = animeNodes.begin(); it != animeNodes.end(); ++it)
        {
            auto anchor = it->selectFirst(".//p[@class=\"name\"]/a");
            auto title = anchor.attr("title").as_string();
            auto coverUrl = it->selectFirst(".//img").attr("src").as_string();
            auto link = anchor.attr ("href").as_string ();
            animes.emplaceBack(ShowData(title,link,coverUrl,Providers::GOGOANIME));
        }
        m_canFetchMore = true;
        m_currentPage = page;
        lastSearch = [query,this]{
            return search(query,++m_currentPage);
        };
        return animes;
    }

    QVector<ShowData> popular(int page, int type=0) override{
        QVector<ShowData> animes;
        auto animeNodes = NetworkClient::get ("https://ajax.gogo-load.com/ajax/page-recent-release-ongoing.html?page=" + std::to_string (page)).document ()
            .select ("//div[@class='added_series_body popular']/ul/li");
        if(animeNodes.empty ()){
            m_canFetchMore = false;
            return animes;
        }
        for (pugi::xpath_node_set::const_iterator it = animeNodes.begin(); it != animeNodes.end(); ++it)
        {
            pugi::xpath_node anchor = it->selectFirst ("a");
            auto link = anchor.attr("href").as_string ();
            auto coverUrl = QString(anchor.selectFirst(".//div[@class='thumbnail-popular']").attr ("style").as_string ()).split ("'").at (1);
            auto title = anchor.attr ("title").as_string ();
            animes.push_back (ShowData{ title,link,coverUrl,Providers::GOGOANIME });
            animes.last ().latestTxt = it->selectText (".//p[last()]/a");
        }
        m_canFetchMore = true;
        m_currentPage = page;
        lastSearch = [this]{
            return popular(++m_currentPage);
        };
        return animes;
    }

    QVector<ShowData> latest(int page, int type=0) override{
        QVector<ShowData> animes;
        std::string url = "https://ajax.gogo-load.com/ajax/page-recent-release.html?page=" + std::to_string (page) + "&type=1";
        pugi::xpath_node_set animeNodes = NetworkClient::get(url).document().select("//ul[@class='items']/li");
        if (animeNodes.empty()) {
            qDebug() << "Unable to fetch latest animes from" << url;
            m_canFetchMore = false;
            return animes;
        }

        for (pugi::xpath_node_set::const_iterator it = animeNodes.begin(); it != animeNodes.end(); ++it)
        {
            QString coverUrl = it->selectFirst(".//img").attr("src").as_string();
            static QRegularExpression re{R"(([\w-]*?)(?:-\d{10})?\.)"};
            auto lastSlashIndex =  coverUrl.lastIndexOf("/");
            auto id = re.match (coverUrl.mid(lastSlashIndex+1));
            if(!id.hasMatch ()){
                qDebug() << "Unable to extract Id from" << coverUrl.mid(lastSlashIndex+1);
                continue;
            }
            QString title = QString(it->selectFirst (".//p[@class='name']/a").node ().child_value ()).trimmed ().replace("\n"," ");
            std::string link = "/category/" + id.captured (1).toStdString ();
            animes.emplaceBack (ShowData{ title,link,coverUrl,Providers::GOGOANIME });
            animes.last ().latestTxt = it->selectText (".//p[@class='episode']");
        }

        m_canFetchMore = true;
        m_currentPage = page;
        lastSearch = [this]{
            return latest(++m_currentPage);
        };
        emit fetchedResults (animes);
        return animes;
    }

    ShowData loadDetails(ShowData anime) override{
        CSoup doc = getInfoPage (anime);
        anime.episodes = getEpisodes (getEpisodesLink(doc));
        anime.description = QString(doc.selectFirst ("//span[contains(text() ,'Plot Summary')]").parent ().text ().as_string ()).replace ("\n"," ").trimmed ();
        anime.status = doc.selectFirst ("//span[contains(text(),'Status')]/following-sibling::a").node ().child_value ();
        pugi::xpath_node_set genreNodes = doc.select ("//span[contains(text(),'Genre')]/following-sibling::a");
        for (pugi::xpath_node_set::const_iterator it = genreNodes.begin(); it != genreNodes.end(); ++it)
        {
            QString genre = QString(it->attr ("title").as_string ()).replace ("\n"," ");
            anime.genres.push_back (genre);
        }
        anime.totalEpisodes = anime.episodes.count ();
        return anime;
    };

    CSoup getInfoPage(const ShowData& anime){
        auto response = NetworkClient::get(hostUrl() + anime.link);
        if(response.code == 404){
            QString errorMessage = "Invalid URL: '" + QString::fromStdString (hostUrl() + anime.link +"'");
            MyException(errorMessage).raise();
        }
        return response.document ();
    }

    std::string getEpisodesLink(const CSoup& doc){
        std::string lastEpisode = doc.selectFirst ("//ul[@id=\"episode_page\"]/li[last()]/a").attr ("ep_end").as_string ();
        std::string animeId = doc.selectFirst ("//input[@id=\"movie_id\"]").attr ("value").as_string ();
        return "https://ajax.gogo-load.com/ajax/load-list-episode?ep_start=0&ep_end="+lastEpisode+"&id="+animeId;
    }

    QVector<Episode> getEpisodes(const std::string& episodesLink){
        auto episodeNodes = NetworkClient::get(episodesLink).document ().select("//ul/li/a");
        QVector<Episode> episodes;
        episodes.reserve (episodeNodes.size ());
        for (pugi::xpath_node_set::const_iterator it = episodeNodes.begin (); it != episodeNodes.end(); ++it)
        {
            int number = std::stoi (it->selectText ("div[@class='name']"));
            std::string link = it->attr ("href").value ();
            episodes.emplaceFront ( Episode { number,link } );
        }
        return episodes;
    }

    int getTotalEpisodes(const ShowData &anime) override{
        auto doc = getInfoPage (anime);
        return NetworkClient::get(getEpisodesLink(doc)).document ().select("//ul/li/a").size ();
    }

    QVector<VideoServer> loadServers(const Episode& episode) override{
        QVector<VideoServer> servers;
        pugi::xpath_node_set serverNodes = NetworkClient::get(hostUrl () + episode.link).document ().select("//div[@class='anime_muti_link']/ul/li/a");
        for (pugi::xpath_node_set::const_iterator it = serverNodes.begin (); it != serverNodes.end(); ++it)
        {
            std::string link = it->attr("data-video").as_string ();
            QString name = QString(it->node().child_value ()).trimmed ();
            Functions::httpsIfy(link);
            //            server.headers["referer"] = QS(hostUrl ());
            qDebug() << name << link;
            servers.emplaceBack (VideoServer(name,link));
        }
        return servers;
    };

    QString extractSource(VideoServer& server) override{

        if (Functions::containsSubstring(server.link, "gogo")
            || Functions::containsSubstring(server.link, "goload")
            || Functions::containsSubstring(server.link, "playgo")
            || Functions::containsSubstring(server.link, "anihdplay")
            || Functions::containsSubstring(server.link, "playtaku")
            || Functions::containsSubstring(server.link, "gotaku1")
            ) {
            GogoCDN extractor;
            return extractor.extract(server.link);
        } else if (Functions::containsSubstring(server.link, "sb")
                   || Functions::containsSubstring(server.link, "sss")) {
            //            extractor = new StreamSB(server);
        } else if (Functions::containsSubstring(server.link, "fplayer")
                   || Functions::containsSubstring(server.link, "fembed")) {
            //            extractor = new FPlayer(server);
        }
        MyException("Cannot find extract " + QString::fromStdString (server.link)).raise ();
        return "";
    };

};

#endif // GOGOANIME_H
