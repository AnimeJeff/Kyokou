	
//#ifndef ALLANIME_H
//#define ALLANIME_H

//#include "showprovider.h"
//#include <regex>
//class AllAnime : public ShowProvider
//{
//private:


//public:
//    explicit AllAnime(QObject *parent = nullptr) : ShowProvider{parent} {

//    };
//public:
//    QString name(){ return "AllAnime"; }
//    std::string hostUrl = "https://allanime.to";
//private:
//    const std::string apiHost = "api.allanime.day";
//    const std::string ytAnimeCoversHost = "https://wp.youtube-anime.com/aln.youtube-anime.com";
//    const std::string referer = "https://embed.ssbcontent.site";
//    const std::regex idRegex {"^" + hostUrl + "/anime/(\\w+)$"};
//    const std::regex epNumRegex {"/\\[sd\\]ub/(\\d+)"};
//    const std::string idHash = "9d7439c90f203e534ca778c4901f9aa2d3ad42c06243ab2c5e6b79612af32028";
//    const std::string episodeInfoHash = "c8f3ac51f598e630a1d09d7f7fb6924cff23277f354a23e473b962a367880f7d";
//    const std::string searchHash = "06327bc10dd682e1ee7e07b6db9c16e9ad2fd56c1b769e47513128cd5c9fc77a";
//    const std::string videoServerHash = "5f1a64b73793cc2234a389cf3a8f93ad82de7043017dd551f38f65b89daa65e0";
//public:

//    QList<ShowData> search(QString query, int page, int type = 0){
//        QList<ShowData> shows;
//        if (query.isEmpty ()){
//            m_canFetchMore = false;
//            return shows;
//        }

//        auto variables =
//            "{\"search\":{\"allowAdult\":true,\"query\":\""+ query.toStdString ()+ "\"},\"translationType\":\"sub\"}";

//        graphqlQuery(variables, searchHash)["data"]["shows"]["edges"];
//        return shows;;

//        std::string url = hostUrl + "/search.html?keyword=" + Functions::urlEncode (query.toStdString ())+ "&page="+ std::to_string (page);
//        auto showNodes = NetworkClient::get(url).document().select("//ul[@class='items']/li");
//        if (showNodes.empty ()){
//            m_canFetchMore = false;
//            return shows;
//        }
//        for (pugi::xpath_node_set::const_iterator it = showNodes.begin(); it != showNodes.end(); ++it)
//        {
//            auto anchor = it->selectFirst(".//ul[@class='items']/a");
//            QString title = anchor.attr("title").as_string();
//            QString coverUrl = it->selectFirst(".//img").attr("src").as_string();
//            std::string link = anchor.attr ("href").as_string ();
//            shows.emplaceBack(ShowData(title,link,coverUrl,Providers::ALLANIME));
//        }
//        m_canFetchMore = true;
//        m_currentPage = page;
//        lastSearch = [query,this,type]{
//            return search(query,++m_currentPage);
//        };
//        return shows;
//    }
//    nlohmann::json graphqlQuery(std::string variables, std::string persistHash){

//        std::string extensions = "{\"persistedQuery\":{\"version\":1,\"sha256Hash\":\"" + persistHash + "\"}}";

//        std::map<std::string, std::string> headers = {{"origin", hostUrl}};
//        std::map<std::string, std::string> params = {{"variables", variables}, {"extensions", extensions}};

//        auto res = NetworkClient::get("https://" + apiHost + "/api", headers, params);


//        qDebug() << QString::fromStdString (res.body);
////        if (res.data.empty()){
////            throw std::exception("Var: " + variables + "\\nError: " + res.errors[0].message);
////        }

//        return "";//res;
//    }


//    QList<ShowData> popular(int page, int type){
//        QList<ShowData> shows;
//        std::string url = "";
//        auto showNodes = NetworkClient::get(url).document().select("//ul[@class='items']/li");
        
//        if (showNodes.empty ()){
//            m_canFetchMore = false;
//            return shows;
//        }

//        for (pugi::xpath_node_set::const_iterator it = showNodes.begin(); it != showNodes.end(); ++it)
//        {
//            auto anchor = it->selectFirst(".//ul[@class='items']/a");
//            QString title = anchor.attr("title").as_string();
//            QString coverUrl = it->selectFirst(".//img").attr("src").as_string();
//            std::string link = anchor.attr ("href").as_string ();
//            shows.emplaceBack(ShowData(title,link,coverUrl,Providers::ALLANIME));
//        }
//        m_canFetchMore = true;
//        m_currentPage = page;
//        lastSearch = [this,type]{
//            return popular(++m_currentPage,type);
//        };
//        return shows;
//    }
//    QList<ShowData> latest(int page, int type){
//        QList<ShowData> shows;
//        std::string url = "";
//        auto showNodes = NetworkClient::get(url).document().select("//ul[@class='items']/li");
//        if (showNodes.empty ()){
//            m_canFetchMore = false;
//            return shows;

//        }

//        for (pugi::xpath_node_set::const_iterator it = showNodes.begin(); it != showNodes.end(); ++it)
//        {
//            auto anchor = it->selectFirst(".//ul[@class='items']/a");
//            QString title = anchor.attr("title").as_string();
//            QString coverUrl = it->selectFirst(".//img").attr("src").as_string();
//            std::string link = anchor.attr ("href").as_string ();
//            shows.emplaceBack(ShowData(title,link,coverUrl,Providers::ALLANIME));
//        }
//        m_canFetchMore = true;
//        m_currentPage = page;
//        lastSearch = [this,type]{
//            return latest(++m_currentPage,type);
//        };

//        return shows;
//    }
//    ShowData loadDetails(ShowData show){
//        NetworkClient::get(show.link).document();
//        return show;
//    }
//    int getTotalEpisodes(const ShowData &show){
//        return 0;
//    }
//    QList<VideoServer> loadServers(const Episode &episode){
//        QList<VideoServer> servers;
//        NetworkClient::get(episode.link).document();
//        return servers;
//    }
//    QString extractSource(VideoServer &server){
//        return "";
//    }
//};

//#endif // ALLANIME_H
