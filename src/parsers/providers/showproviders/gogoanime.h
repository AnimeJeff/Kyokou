#ifndef GOGOANIME_H
#define GOGOANIME_H

#include <QDebug>
#include "parsers/showprovider.h"
#include "parsers/extractors/gogocdn.h"
#include "tools/ErrorHandler.h"
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
    std::string hostUrl = "https://gogoanimehd.io";

    QVector<ShowData> search(QString query, int page, int type=0) override{
        QVector<ShowData> animes;
        if(query.isEmpty ())
        {
            m_canFetchMore = false;
            return animes;
        }
        std::string url = hostUrl + "/search.html?keyword=" + Functions::urlEncode (query.toStdString ())+ "&page="+ std::to_string (page);
        auto animeNodes = NetworkClient::get(url).document().select("//ul[@class='items']/li");
        if(animeNodes.empty ()){
            m_canFetchMore = false;
            return animes;
        }

        for (pugi::xpath_node_set::const_iterator it = animeNodes.begin(); it != animeNodes.end(); ++it)
        {
            auto anchor = it->selectFirst(".//p[@class='name']/a");
            auto title = anchor.attr("title").as_string();
            auto coverUrl = it->selectFirst(".//img").attr("src").as_string();
            auto link = anchor.attr ("href").as_string ();
            animes.emplaceBack(title, link, coverUrl, Providers::GOGOANIME);
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
        std::string url = "https://ajax.gogo-load.com/ajax/page-recent-release-ongoing.html?page=" + std::to_string (page);
        auto animeNodes = NetworkClient::get (url).document ()
            .select ("//div[@class='added_series_body popular']/ul/li");
        if(animeNodes.empty ())
        {
            m_canFetchMore = false;
            return animes;
        }
        for (pugi::xpath_node_set::const_iterator it = animeNodes.begin(); it != animeNodes.end(); ++it)
        {
            pugi::xpath_node anchor = it->selectFirst ("a");
            std::string link = anchor.attr("href").as_string ();
            QString coverUrl = QString(anchor.selectFirst(".//div[@class='thumbnail-popular']").attr ("style").as_string ());
            coverUrl = coverUrl.split ("'").at (1);
            QString title = anchor.attr ("title").as_string ();
            animes.push_back (ShowData{ title, link, coverUrl, Providers::GOGOANIME });
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

    ShowData loadDetails(ShowData anime) override
    {
        CSoup doc = getInfoPage (anime);
        qDebug() << "loaded doc";
        anime.episodes = getEpisodes (getEpisodesLink(doc));
        qDebug() << "loaded ep";
        anime.description = QString(doc.selectFirst ("//span[contains(text() ,'Plot Summary')]").parent ().text ().as_string ()).replace ("\n"," ").trimmed ();
        //anime.status = doc.selectFirst ("//span[contains(text(),'Status')]/following-sibling::a").node ().child_value ();
        qDebug() << "loaded desc";
        pugi::xpath_node_set genreNodes = doc.select ("//span[contains(text(),'Genre')]/following-sibling::a");
        for (pugi::xpath_node_set::const_iterator it = genreNodes.begin(); it != genreNodes.end(); ++it)
        {
            QString genre = QString(it->attr ("title").as_string ()).replace ("\n"," ");
            anime.genres.push_back (genre);
        }
        qDebug() << "loaded genres";
        anime.totalEpisodes = anime.episodes.count ();
        return anime;
    };

    CSoup getInfoPage(const ShowData& anime)
    {
        auto response = NetworkClient::get(hostUrl + anime.link);
        if(response.code == 404){
            QString errorMessage = "Invalid URL: '" + QString::fromStdString (hostUrl + anime.link +"'");
            MyException(errorMessage).raise();
        }
        return response.document ();
    }

    std::string getEpisodesLink(const CSoup& doc)
    {
        std::string lastEpisode = doc.selectFirst ("//ul[@id='episode_page']/li[last()]/a").attr ("ep_end").as_string ();
        std::string animeId = doc.selectFirst ("//input[@id='movie_id']").attr ("value").as_string ();
        qDebug() << "found episodes link";
        return "https://ajax.gogo-load.com/ajax/load-list-episode?ep_start=0&ep_end="+lastEpisode+"&id="+animeId;
    }

    QVector<Episode> getEpisodes(const std::string& episodesLink)
    {
        QVector<Episode> episodes;
        pugi::xpath_node_set episodeNodes;
        try
        {
            auto doc = NetworkClient::get(episodesLink).document ();
            episodeNodes = doc.select("//ul[@id='episode_related']/li/a");
            for (pugi::xpath_node_set::const_iterator it = episodeNodes.end () - 1; it != episodeNodes.begin() - 1; --it)
            {
                QString title = QString::fromStdString (it->selectText (".//div[@class='name']")).replace("EP", "").trimmed ();
                qDebug() << title << it->selectText (".//div[@class='name']");
                int number = -1;
                bool ok;
                int intTitle = title.toInt (&ok);
                if(ok){
                    number = intTitle;
                    title = "";
                }
                std::string link = it->attr ("href").value ();
                episodes.emplaceBack (  Episode(number,link,title));
            }
        }
        catch(std::exception& e)
        {
            qDebug() << "oof " << e.what ();
            return episodes;
        }


        return episodes;
    }

    int getTotalEpisodes(const ShowData &anime) override{
        auto doc = getInfoPage (anime);
        return NetworkClient::get(getEpisodesLink(doc)).document ().select("//ul/li/a").size ();
    }

    QVector<VideoServer> loadServers(const Episode& episode) override{
        QVector<VideoServer> servers;
        pugi::xpath_node_set serverNodes = NetworkClient::get(hostUrl + episode.link).document ().select("//div[@class='anime_muti_link']/ul/li/a");
        for (pugi::xpath_node_set::const_iterator it = serverNodes.begin (); it != serverNodes.end(); ++it)
        {
            std::string link = it->attr("data-video").as_string ();
            QString name = QString(it->node().child_value ()).trimmed ();
            Functions::httpsIfy(link);
            //            server.headers["referer"] = QS(hostUrl);
            qDebug() << name << link;
            servers.emplaceBack (VideoServer(name,link));
        }
        return servers;
    };

    QString extractSource(VideoServer& server) override{
        auto serverName = server.name.toLower ();
        try
        {
            if (serverName.contains ("vidstreaming") ||
                serverName.contains ("gogo"))
            {
                GogoCDN extractor;
                return extractor.extract(server.link);
            }
        }
        catch (QException &e)
        {
            ErrorHandler::instance ().show ("Cannot find extract " + QString::fromStdString (server.link));
        }

        return "";
    };

};

#endif // GOGOANIME_H
