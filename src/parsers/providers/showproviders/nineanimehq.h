
//#ifndef NINEANIMEHQ_H
//#define NINEANIMEHQ_H
//#include <QtConcurrent>
//#include "parsers/showprovider.h"
//#include "parsers/data/videoserver.h"
//#include "tools/functions.h"



//class NineanimeHQ : public ShowProvider
//{
//public:
//    NineanimeHQ();
//    std::string m_lastSearch;
//    bool m_widgetSearched = true;
//public:
//    int providerEnum()override{return Providers::NINEANIME;};
//    QString name() override {return "9anime";} ;
//    QString hostUrl()override{return "https://9anime.ph";};

//    QVector<ShowData> search(QString query, int page, int type) override {
//        Q_UNUSED (type);
//        m_widgetSearched = false;
//        //replace subbed with dubbed if ...
//        QVector<ShowData> animes;
//        QString searchLink = "https://9animehq.to/filter?keyword=" + QUrl::toPercentEncoding(query) + "&page="+std::to_string (page);
//        auto animeNodes = NetworkClient::get(searchLink).document ().select ("//div[@id='list-items']/div[@class='item']");
//        //todo other can fetch more
//        for (pugi::xpath_node_set::const_iterator it = animeNodes.begin(); it != animeNodes.end(); ++it)
//        {
//            pugi::xpath_node anchorNode = it->selectFirst(".//div[@class='ani poster tip']/a");
//            pugi::xpath_node imgNode = anchorNode.selectFirst(".//img");
//            QString title = QString(imgNode.attr("alt").as_string());
//            QString coverUrl = QString(imgNode.attr("src").as_string());
//            QString link = QString(anchorNode.attr("href").as_string());
//            QString latestTxt = it->selectText(".//left/span/span");
//            animes.emplaceBack(ShowData{title, link, coverUrl,
//                                        Providers::NINEANIME, latestTxt,
//                                        ShowData::ShowType::ANIME});
//        }
//        return animes;
//    };

//    QVector<ShowData> popular(int page,int type)override{
//        Q_UNUSED(type);
//        return widgetSearch("trending",page);
//    };

//    QVector<ShowData> latest(int page,int type)override{
//        Q_UNUSED(type);
//        return widgetSearch ("updated-sub",page);
//    };

//    QVector<ShowData> widgetSearch(std::string path,int page)
//    {
//        m_widgetSearched = true;
//        m_currentPage = page;
//        m_lastSearch = path;
//        QString url = "https://9anime.id/ajax/home/widget/" + path +"?page=" + std::to_string(page);
//        QVector<ShowData> animes = parseAnimes (url);
//        return animes;
//    };
//    ShowData loadDetails(ShowData anime) override {
//        CSoup document = NetworkClient::get(hostUrl() + anime.link).document();
//        std::string dataId = document.selectFirst("//div[@class='container watch-wrap']").attr("data-id").as_string ();
//        qDebug()<<QS (dataId);
//        auto episodesUrl = hostUrl()+ "/ajax/episode/list/" + dataId+"?vrf="+encodeVrf (dataId);
//        auto loadEpisodeJob = QtConcurrent::run ([&](){
//            loadEpisodes(anime,episodesUrl);
//        });
//        loadEpisodeJob.waitForFinished ();
//        anime.totalEpisodes = anime.episodes.count ();
//        return anime;
//    }
//    int getTotalEpisodes(const ShowData& anime) override {
//        return 0;
//    }
//    //        ShowData loadDetails(ShowData anime) override {
//    //            CSoup document = NetworkClient::get(hostUrl() + anime.link.toStdString ()).document();
//    //            std::string dataId = document.selectFirst("//div[@class='container watch-wrap']").attr("data-id").as_string ();
//    //            auto episodesUrl = hostUrl()+ "/ajax/episode/list/" + dataId+"?vrf="+encodeVrf (dataId);
//    //            auto loadEpisodeJob = QtConcurrent::run ([&anime,episodesUrl,this](){
//    //                loadEpisodes(anime,episodesUrl);
//    //            });
//    //            pugi::xpath_node element = document.selectFirst("//div[@class='info']");
//    //            anime.title = QString::fromStdString(element.selectFirst (".//h1[@class='title d-title']").attr ("data-jp").as_string ());
//    //    //        auto genres = element.select("//div[contains(text(), 'Genre']/span/a");
//    //    //        for(const auto& genre:genres){
//    //    //            anime.genres.push_back (genre.node ().child_value ());
//    //    //        }
//    //            anime.description = QString::fromStdString(element.selectFirst(".//div[@class='content']").node ().child_value ()).replace ("\n"," ");
//    //            anime.status = QString::fromStdString (element.selectFirst (".//div[contains(text(), 'Status')]/span").node ().child_value ());
//    //            anime.releaseDate = element.selectText (".//div[contains(text(), \"Date aired\")]/span");
//    //            anime.provider = Providers::NINEANIME;
//    //            if(anime.status!="Completed"){
//    //                anime.updateTime = QString(document.selectFirst(".//div[@class='alert next-episode']").node ().child_value ())
//    //                                       .replace ("\n"," ").mid(43);
//    //            }
//    //            anime.rating = QString(element.selectFirst("//div[contains(text(), 'Scores')]/span").node ().child_value ()).trimmed ();
//    //            anime.views = QString(element.selectFirst ("//div[contains(text(), 'Views')]/span").node ().child_value ());
//    //            loadEpisodeJob.waitForFinished ();
//    //            return anime;
//    //        };

//    void loadEpisodes(ShowData& show,std::string episodeLink){
//        NetworkClient::Response episodeData = NetworkClient::get(episodeLink);
//        if (episodeData.body[0] != '{')return;
//        //emit error
//        std::string resultJson = episodeData.json()["result"].get<std::string>();
//        CSoup document { resultJson };
//        auto dataIdsNodes = document.select("//a[@data-ids]");
//        for (pugi::xpath_node_set::const_iterator it = dataIdsNodes.begin(); it != dataIdsNodes.end(); ++it)
//        {
//            QString title;
//            int number = it->attr("data-num").as_int();
//            //            std::string ids = Functions::substringBefore(
//            //                element.attr("data-ids").as_string(), ",");
//            QString ids = it->attr("data-ids").as_string();
//            ids = ids.mid (0,ids.indexOf (','));
//            QString _name = QString::fromStdString(it->selectText(".//span"));
//            QString namePrefix = QString("Episode %1").arg(number);
//            if (!_name.isEmpty() && _name != namePrefix) {
//                title = QString("%1: %2").arg(namePrefix).arg(_name);
//            } else {
//                title = namePrefix;
//            }
//            show.episodes.emplaceBack (Episode { number,ids,title } );
//        }
//    }

//    QVector<VideoServer> loadServers(const Episode &episode)override{
//        QVector<VideoServer> servers;
//        auto link = hostUrl() + "/ajax/server/list/" + episode.link.toStdString () + "?vrf=" + encodeVrf(episode.link);
//        auto body = NetworkClient::get(link).json ()["result"].get <std::string>();
//        auto document = CSoup(body);
//        auto serverNodes = document.select("//li");
//        for (pugi::xpath_node_set::const_iterator it = serverNodes.begin(); it != serverNodes.end(); ++it)
//        {
//            auto name = it->node ().child_value ();
//            std::string id = it->attr("data-link-id").as_string ();
//            qDebug()<<name<<id;
//            servers.emplaceBack ( VideoServer { name, id } );
//        }
//        return servers;
//    };

//    QString extractSource(VideoServer &server)override{
//        if(server.name == "Vidstream"|| server.name == "MyCloud"){
//            nlohmann::json result = NetworkClient::get(hostUrl ()+"/ajax/server/" + QString::fromStdString(server.link) + "?vrf="+encodeVrf (server.link)).json ()["result"];
//            //            auto skipData = decodeVrf (result["skip_data"].get<std::string>());
//            std::string serverUrl = decodeVrf (result["url"].get<std::string>());
//            std::string slug = Functions::findBetween (serverUrl,"e/","?");
//            bool isMcloud = server.name == "MyCloud";
//            QString serverName = isMcloud ? "Mcloud" : "Vizcloud";
//            QString url = "https://9anime.eltik.net/" + serverName + "?query="+slug + "&apikey=saikou";
//            server.source = QString::fromStdString (NetworkClient::get (url).json ()["data"]["media"]["sources"][0]["file"].get<std::string> ());
//            return server.source;
//        }else if(server.name == "StreamTape"){

//        }else if(server.name == "Mp4upload"){

//        }else if(server.name == "Filemoon"){

//        }
//        return "";
//    };

//private:
//    std::string encodeVrf(const std::string& text){
//        return NetworkClient::get("https://9anime.eltik.net/vrf?query="+text+"&apikey=saikou").json ()["url"].get <std::string>();
//    }
//    std::string encodeVrf(const QString& text) {
//        return encodeVrf(text.toStdString ());
//    }

//    std::string decodeVrf(const std::string& text){
//        return NetworkClient::get("https://9anime.eltik.net/decrypt?query="+text+"&apikey=saikou").json ()["url"].get <std::string>();
//    }
//    QVector<ShowData> parseAnimes(std::string url){
//        CSoup document { NetworkClient::get(url).json ()["result"] };
//        QVector<ShowData> animes;
//        pugi::xpath_node_set results = document.select("//div[@class='item']");
//        for(const auto& node:results){
//            pugi::xpath_node anchor = node.selectFirst (".//a[@class='name d-title']");
//            QString title = QString(anchor.node ().child_value ()).replace("\n"," ").trimmed ();
//            QString coverUrl = QString( node.selectFirst (".//img").attr ("src").as_string ());
//            QString link = QString( anchor.attr ("href").as_string ());
//            QString latestTxt = QString(node.selectFirst(".//span[@class='ep-status total']/span").node ().child_value ());
//            animes.push_back ( ShowData{ title,link,coverUrl,Providers::NINEANIME,latestTxt,ShowData::ShowType::ANIME } );
//        }
//        return animes;
//    };
//    //    Status parseStatus(std::string statusString){
//    //        if (statusString=="Releasing") {
//    //            return Status::Ongoing;}
//    //        else if(statusString=="Completed"){
//    //            return Status::Completed;
//    //        }
//    //        return Status::Completed;
//    //    }
//};

//#endif // NINEANIMEHQ_H
