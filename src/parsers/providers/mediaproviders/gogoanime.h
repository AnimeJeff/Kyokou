#ifndef GOGOANIME_H
#define GOGOANIME_H

#include <QDebug>
#include "parsers/mediaprovider.h"
#include "network/client.h"
#include "parsers/extractors/gogocdn.h"
#include "tools/MyException.h"
#include "tools/functions.h"

class Gogoanime : public MediaProvider
{
    Q_OBJECT
    QString m_lastSearch;
    enum e_lastSearch{
        LATEST,
        POPULAR
    };

public:
    int providerEnum() override{
        return Providers::Gogoanime;
    }
    Gogoanime() = default;
    QString name() override{
        return "Gogoanime";
    }
    std::string hostUrl() override{
        return "https://gogoanime.hu";
    }

    QVector<MediaData> search(QString query, int page, int type=0) override{
        QVector<MediaData> animes;
        if(query.isEmpty ())return animes;
        m_currentPage = page;
        m_lastSearch = query;
        std::string url = hostUrl()+"/search.html?keyword=" + QUrl::toPercentEncoding(query).toStdString () + "&page="+std::to_string(page);
        client.get(url).document().select("//ul[@class='items']/li").forEach ([&](pugi::xpath_node node) {
            MediaData anime;
            auto anchor = node.selectFirst(".//p[@class=\"name\"]/a");
            anime.title = anchor.attr("title").as_string();
            anime.coverUrl = node.selectFirst(".//img").attr("src").as_string();
            anime.link = anchor.attr ("href").as_string ();
            anime.provider = Providers::Gogoanime;
            anime.releaseDate = node.selectText (".//p[@class=\"released\"]"); //TODO remove released
            anime.releaseDate.remove (0,10);
            animes.append (std::move(anime));
        });
        m_canFetchMore=!animes.empty ();
        return animes;
    }

    QVector<MediaData> popular(int page, int type=0) override{
        m_currentPage=page;
        QVector<MediaData> animes;
        client.get ("https://ajax.gogo-load.com/ajax/page-recent-release-ongoing.html?page="+std::to_string (page)).document ()
            .select ("//div[@class='added_series_body popular']/ul/li").forEach([&](pugi::xpath_node element){
                MediaData anime;
                pugi::xpath_node anchor = element.selectFirst ("a");
                anime.link = anchor.attr("href").as_string ();
                anime.coverUrl = QString(anchor.selectFirst(".//div[@class='thumbnail-popular']").attr ("style").as_string ()).split ("'").at (1);
                anime.provider = Providers::Gogoanime;
                anime.latestTxt = element.selectText (".//p[last()]/a");
                anime.title = QString(anchor.attr ("title").as_string ());
                animes.push_back (std::move(anime));
            });
        m_canFetchMore=!animes.empty ();
        m_lastSearch = "popular";
        return animes;
    }

    QVector<MediaData> latest(int page, int type=0) override{
        m_currentPage=page;
        QVector<MediaData> animes;
        std::string url = "https://ajax.gogo-load.com/ajax/page-recent-release.html?page=" + std::to_string(page) + "&type=1";
        pugi::xpath_node_set results=
            client.get(url).document().select("//ul[@class='items']/li");
        if (results.empty()) {
            QString errorMessage = QString::fromStdString(
                "Unable to fetch latest animes from '" + url + "'");
            MyException(errorMessage).raise();
        }
        for(const auto& element:results){
            MediaData anime;

            anime.coverUrl = element.selectFirst(".//img").attr("src").as_string();
            static QRegularExpression re{R"(([\w-]*?)(?:-\d{10})?\.)"};
            auto lastSlashIndex =  anime.coverUrl.lastIndexOf("/");
            auto id = re.match (anime.coverUrl.mid(lastSlashIndex+1));
            if(!id.hasMatch ()){
                qDebug() << "Unable to extract Id from" << anime.coverUrl.mid(lastSlashIndex+1);
                continue;
            }
            anime.title = QString(element.selectFirst (".//p[@class='name']/a").node ().child_value ()).trimmed ().replace("\n"," ");
            anime.link = "/category/" + id.captured (1);
            anime.latestTxt = element.selectText(".//p[@class='episode']");
            anime.provider = Providers::Gogoanime;
            animes.push_back(std::move(anime));
        }
        m_lastSearch = "latest";
        m_canFetchMore = !animes.empty ();
        return animes;
    }

    MediaData loadDetails(MediaData anime) override{
        CSoup doc = client.get(hostUrl() + anime.link.toStdString()).document();
        std::string title = doc.selectText ("//head/title");
        if(title.starts_with ("Pages")){
            QString errorMessage = QString::fromStdString ("Invalid URL: '" + hostUrl() + anime.link.toStdString()+"'");
            MyException(errorMessage).raise();
        }
        anime.description = QString(doc.selectFirst ("//span[contains(text() ,'Plot Summary')]").parent ().text ().as_string ()).replace ("\n"," ").trimmed ();
        anime.status = doc.selectFirst ("//span[contains(text(),'Status')]/following-sibling::a").node ().child_value ();
        doc.select ("//span[contains(text(),'Genre')]/following-sibling::a").forEach ([&](pugi::xpath_node node){
            QString genre = QString(node.attr ("title").as_string ()).replace ("\n"," ");
            anime.genres.push_back (genre);
        });
        std::string lastEpisode = doc.selectFirst ("//ul[@id=\"episode_page\"]/li[last()]/a").attr ("ep_end").as_string ();
        std::string animeId = doc.selectFirst ("//input[@id=\"movie_id\"]").attr ("value").as_string ();
        auto epList = client.get("https://ajax.gogo-load.com/ajax/load-list-episode?ep_start=0&ep_end="+lastEpisode+"&id="+animeId).document ()
                          .select("//ul/li/a").map<Episode>([&](pugi::xpath_node node) {
                              int number = std::stoi (node.selectText ("div[@class=\"name\"]"));
                              std::string link = node.attr ("href").value ();
                              return Episode(number,link);
                          });
        std::reverse(epList.begin(), epList.end());
        anime.episodes = QVector<Episode>(epList.begin (),epList.end ());
        return anime;
    };

    QVector<VideoServer> loadServers(const Episode& episode) override{
        QVector<VideoServer> servers;
        client.get(hostUrl ()+episode.link).document ().select("//div[@class='anime_muti_link']/ul/li/a").forEach ([&](pugi::xpath_node node){
            std::string link = node.attr("data-video").as_string ();
            VideoServer server;
            server.name = QString(node.node().child_value ()).trimmed ();
            Functions::httpsIfy(link);
            server.link = link;
            //            server.headers["referer"] = QS(hostUrl ());
            qDebug()<<server.name<<server.link;
            servers.push_back (std::move (server));

        });
        return QVector<VideoServer>(servers.begin (),servers.end ());
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
        }else{
            MyException("Cannot find extract " + QString::fromStdString (server.link)).raise ();
        }
    };

    QVector<MediaData> fetchMore() override {
        if(m_lastSearch == "latest"){
            return latest (++m_currentPage,true);
        }else if(m_lastSearch == "popular"){
            return popular (++m_currentPage,true);
        }else{
            return search (m_lastSearch,++m_currentPage);
        }
    }
};

#endif // GOGOANIME_H
