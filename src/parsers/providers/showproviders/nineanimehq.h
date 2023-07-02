
#ifndef NINEANIMEHQ_H
#define NINEANIMEHQ_H
#include <QtConcurrent>
#include "parsers/showprovider.h"
#include "parsers/data/videoserver.h"
#include "tools/functions.h"



class NineanimeHQ : public ShowProvider
{
public:
    NineanimeHQ();
    std::string m_lastSearch;
    bool m_widgetSearched = true;
public:
    int providerEnum()override{return Providers::NineAnimeHQ;};
    QString name() override {return "9anime";} ;
    std::string hostUrl()override{return "https://9anime.ph";};
    QVector<ShowData> search(QString query, int page, int type=0)override{
        m_widgetSearched = false;
        //replace subbed with dubbed if ...
        QVector<ShowData> animes;
        //        std::string searchLink = hostUrl ()+"/filter?language%5B%5D=subbed&keyword="+Functions::urlEncode (query.toStdString ())+"&vrf="+encodeVrf(query)+"&page="+std::to_string (page);
        std::string searchLink = "https://9animehq.to/filter?keyword=" + QUrl::toPercentEncoding(query).toStdString () + "&page="+std::to_string (page);
        client.get(searchLink).document()
            .select("//div[@id='list-items']/div[@class='item']").forEach ([&](pugi::xpath_node item) {
                ShowData anime;
                auto anchor = item.selectFirst(".//div[@class='ani poster tip']/a");
                auto img = anchor.selectFirst(".//img");
                anime.title = QString(img.attr ("alt").as_string ());
                anime.coverUrl = QString(img.attr ("src").as_string ());
                anime.link = QString(anchor.attr ("href").as_string ());
                anime.latestTxt = item.selectText(".//left/span/span");
                anime.provider = Providers::NineAnimeHQ;
                animes.push_back (std::move(anime));
            });
        return animes;
    };

    QVector<ShowData> popular(int page,int type)override{
        Q_UNUSED(type);
        return widgetSearch("trending",page);
    };

    QVector<ShowData> latest(int page,int type)override{
        Q_UNUSED(type);
        return widgetSearch ("updated-sub",page);
    };

    QVector<ShowData> widgetSearch(std::string path,int page)
    {
        m_widgetSearched = true;
        m_currentPage = page;
        m_lastSearch = path;
        std::string url = "https://9anime.id/ajax/home/widget/" + path +"?page=" + std::to_string(page);
        QVector<ShowData> animes = parseAnimes (url);
        return animes;
    };
    ShowData loadDetails(ShowData anime) override {
        CSoup document = client.get(hostUrl() + anime.link.toStdString ()).document();
        std::string dataId = document.selectFirst("//div[@class='container watch-wrap']").attr("data-id").as_string ();
        qDebug()<<QS (dataId);
        auto episodesUrl = hostUrl()+ "/ajax/episode/list/" + dataId+"?vrf="+encodeVrf (dataId);
        auto loadEpisodeJob = QtConcurrent::run ([&](){
            loadEpisodes(anime,episodesUrl);
        });
        loadEpisodeJob.waitForFinished ();
        return anime;
    }
    //        ShowData loadDetails(ShowData anime) override {
    //            CSoup document = client.get(hostUrl() + anime.link.toStdString ()).document();
    //            std::string dataId = document.selectFirst("//div[@class='container watch-wrap']").attr("data-id").as_string ();
    //            auto episodesUrl = hostUrl()+ "/ajax/episode/list/" + dataId+"?vrf="+encodeVrf (dataId);
    //            auto loadEpisodeJob = QtConcurrent::run ([&anime,episodesUrl,this](){
    //                loadEpisodes(anime,episodesUrl);
    //            });
    //            pugi::xpath_node element = document.selectFirst("//div[@class='info']");
    //            anime.title = QString::fromStdString(element.selectFirst (".//h1[@class='title d-title']").attr ("data-jp").as_string ());
    //    //        auto genres = element.select("//div[contains(text(), 'Genre']/span/a");
    //    //        for(const auto& genre:genres){
    //    //            anime.genres.push_back (genre.node ().child_value ());
    //    //        }
    //            anime.description = QString::fromStdString(element.selectFirst(".//div[@class='content']").node ().child_value ()).replace ("\n"," ");
    //            anime.status = QString::fromStdString (element.selectFirst (".//div[contains(text(), 'Status')]/span").node ().child_value ());
    //            anime.releaseDate = element.selectText (".//div[contains(text(), \"Date aired\")]/span");
    //            anime.provider = Providers::NineAnimeHQ;
    //            if(anime.status!="Completed"){
    //                anime.updateTime = QString(document.selectFirst(".//div[@class='alert next-episode']").node ().child_value ())
    //                                       .replace ("\n"," ").mid(43);
    //            }
    //            anime.rating = QString(element.selectFirst("//div[contains(text(), 'Scores')]/span").node ().child_value ()).trimmed ();
    //            anime.views = QString(element.selectFirst ("//div[contains(text(), 'Views')]/span").node ().child_value ());
    //            loadEpisodeJob.waitForFinished ();
    //            return anime;
    //        };

    void loadEpisodes(ShowData& show,std::string episodeLink){
        NetworkClient::Response episodeData = client.get(episodeLink);
        if (episodeData.body[0] != '{')return;
        //emit error
        std::string resultJson = episodeData.json()["result"].get<std::string>();
        CSoup document{resultJson};
        document.select(R"(//a[@data-ids])").forEach([&](pugi::xpath_node element) {
            QString title;
            int number = element.attr("data-num").as_int();

            //            std::string ids = Functions::substringBefore(
            //                element.attr("data-ids").as_string(), ",");

            QString ids = element.attr("data-ids").as_string();
            ids = ids.mid (0,ids.indexOf (','));

            QString _name = QString::fromStdString(element.selectText(".//span"));
            QString namePrefix =
                QString("Episode %1").arg(number);
            if (!_name.isEmpty() && _name != namePrefix) {
                title = QString("%1: %2").arg(namePrefix).arg(_name);
            } else {
                title = namePrefix;
            }
            //            std::string link = ;//hostUrl()+ "/ajax/server/list/" + ids + "?vrf=" + encodeVrf(ids);
            show.episodes.append(Episode(number,ids,title));
        });

    }

    QVector<VideoServer> loadServers(const Episode &episode)override{
        QVector<VideoServer> servers;
        auto link = hostUrl() + "/ajax/server/list/" + episode.link.toStdString () + "?vrf=" + encodeVrf(episode.link);
        auto body = client.get(link).json ()["result"].get <std::string>();
        auto document = CSoup(body);
        document.select("//li").forEach ([&](pugi::xpath_node it){
            auto name = it.node ().child_value ();
            std::string id = it.attr("data-link-id").as_string ();
            qDebug()<<name<<id;
            servers.push_back (VideoServer(name, id));
        });
        return servers;
    };

    QString extractSource(VideoServer &server)override{
        if(server.name == "Vidstream"|| server.name == "MyCloud"){
            nlohmann::json result = client.get(hostUrl ()+"/ajax/server/" + server.link + "?vrf="+encodeVrf (server.link)).json ()["result"];
            //            auto skipData = decodeVrf (result["skip_data"].get<std::string>());
            std::string serverUrl = decodeVrf (result["url"].get<std::string>());
            std::string slug = Functions::findBetween (serverUrl,"e/","?");
            bool isMcloud = server.name == "MyCloud";
            std::string serverName = isMcloud ? "Mcloud" : "Vizcloud";
            std::string url = "https://9anime.eltik.net/"+serverName + "?query="+slug + "&apikey=saikou";
            server.source = QString::fromStdString (client.get (url).json ()["data"]["media"]["sources"][0]["file"].get<std::string> ());
            return server.source;
        }else if(server.name == "StreamTape"){

        }else if(server.name == "Mp4upload"){

        }else if(server.name == "Filemoon"){

        }
        return "";
    };

private:
    std::string encodeVrf(const std::string& text){
        return client.get("https://9anime.eltik.net/vrf?query="+text+"&apikey=saikou").json ()["url"].get <std::string>();
    }
    std::string encodeVrf(const QString& text) {
        return encodeVrf(text.toStdString ());
    }

    std::string decodeVrf(const std::string& text){
        return client.get("https://9anime.eltik.net/decrypt?query="+text+"&apikey=saikou").json ()["url"].get <std::string>();
    }
    QVector<ShowData> parseAnimes(std::string url){
        CSoup document(client.get(url).json ()["result"]);
        QVector<ShowData> animes;
        pugi::xpath_node_set results = document.select("//div[@class='item']");
        for(const auto& node:results){
            ShowData anime;
            pugi::xpath_node anchor = node.selectFirst (".//a[@class='name d-title']");
            anime.title = QString(anchor.node ().child_value ()).replace("\n"," ").trimmed ();
            anime.coverUrl = QString( node.selectFirst (".//img").attr ("src").as_string ());
            anime.link = QString( anchor.attr ("href").as_string ());
            anime.latestTxt = QString(node.selectFirst(".//span[@class='ep-status total']/span").node ().child_value ());
            anime.provider = Providers::NineAnimeHQ;
            animes.push_back (anime);
        }
        return animes;
    };
    //    Status parseStatus(std::string statusString){
    //        if (statusString=="Releasing") {
    //            return Status::Ongoing;}
    //        else if(statusString=="Completed"){
    //            return Status::Completed;
    //        }
    //        return Status::Completed;
    //    }

public:
    QVector<ShowData> fetchMore() override {
        return QVector<ShowData>();
    };
};

#endif // NINEANIMEHQ_H
