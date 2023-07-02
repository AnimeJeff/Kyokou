
#ifndef TANGRENJIE_H
#define TANGRENJIE_H
#include <QString>
#include <regex>
#include "parsers/showprovider.h"
#include "tools/MyException.h"
#include "tools/functions.h"
class Tangrenjie:public ShowProvider
{
    std::regex player_aaaa_regex{R"(player_aaaa=(\{.*?\})</script>)"};
    enum{
        MOVIE,
        TVSERIES,
        VARIETY,
        ANIME
    };
public:
    Tangrenjie();

    // ShowProvider interface
public:
    int providerEnum() override {return Providers::Tangrenjie;}
    QString name() override {return "唐人街影院";}
    std::string hostUrl() override {return "https://www.tangrenjie.tv";}
    QString q_hostUrl() {return "https://www.tangrenjie.tv";}

    QVector<ShowData> selectShow(const std::string& url){
        pugi::xpath_node_set results = client.get(url).document().select("//a[@class='vodlist_thumb lazyload']");
        QVector<ShowData> shows;
        for(const pugi::xpath_node& result:results){
            QString title = result.attr ("title").as_string ();
            QString link = QString(result.attr ("href").as_string ());
            QString coverUrl = result.attr("data-original").as_string ();
            if(coverUrl.startsWith ("/")){
                coverUrl = QString::fromStdString (hostUrl()) + coverUrl;
            }
            shows.emplace_back(ShowData(title,link,coverUrl,Providers::Tangrenjie));
            shows.last ().latestTxt = result.selectFirst (".//span[@class='pic_text text_right']").node ().child_value ();
        }
        return shows;
    }

    QVector<ShowData> search(QString query, int page, int type) override {
        auto url = QString("https://www.tangrenjie.tv/vod/search/page/%1/wd/%2.html").arg (page).arg (QUrl::toPercentEncoding (query));
        auto results = selectShow(url.toStdString ());
        m_currentPage = page;
        lastSearch = [query,type,this]{
            return search(query,++m_currentPage,type);
        };
        return results;
    };

    QVector<ShowData> popular(int page, int type) override {
        return filterSearch(page,type,"hits");
    }

    QVector<ShowData> latest(int page, int type) override {
        return filterSearch(page,type,"time");
    }

    QVector<ShowData> filterSearch(int page, int type,const std::string& sortBy,const std::string& area="",const std::string& year="",const std::string& language=""){
        std::string url = "https://www.tangrenjie.tv/vod/show";
        if(!area.empty ()){
            url += "/area/" + area;
        }
        url += "/by/" + sortBy;
        std::string id;
        if(type == ShowData::DOCUMENTARY){
            id = "28";
        }else{
            id = std::to_string (type);
        }
        url += "/id/" + std::to_string (type);
        if(!language.empty ()){
            url += "/lang/" + language;
        }
        url += "/page/" + std::to_string (page);
        if(!year.empty ()){
            url += "/year/" + year;
        }
        url += ".html";
        auto results = selectShow (url);
        m_currentPage = page;
        lastSearch = [sortBy,type,area,year,language,this]{
            return filterSearch(++m_currentPage,type, sortBy, area,year,language);
        };
        return results;
    }

    QVector<ShowData> fetchMore() override {
        return QVector<ShowData>();
    };
    ShowData loadDetails(ShowData show) override {
        CSoup doc = client.get(hostUrl() + show.link.toStdString ()).document ();
        show.description = doc.selectText("//div[@class='content']");
        int count = 1;
        for (const auto& element:doc.select("//ul[@class='content_playlist list_scroll clearfix']/li/a")){
            bool isInt;
            QString title = element.node ().child_value();
            int number = title.toInt (&isInt);
            QString link = element.attr ("href").as_string ();
            if(!isInt){
                number = count;
            }else{
                title.clear ();
            }
            count++;
            show.episodes.emplace_back (Episode(number,link,title));
        }
        return show;
    };
    QVector<VideoServer> loadServers(const Episode &episode) override {
        VideoServer server;
        server.name = "player_aaaa";
        server.link = episode.link.toStdString ();
        return QVector<VideoServer>{server};
    };
    QString extractSource(VideoServer &server) override {
        qDebug()<<QString::fromStdString (hostUrl()+server.link);
        std::string response = client.get(hostUrl() + server.link).body;
        std::smatch match;
        if (!std::regex_search(response, match, player_aaaa_regex))
            throw MyException("Failed to extract m3u8");
        std::string matchedText = match[1].str();
        auto url = Functions::urlDecode (Functions::base64Decode (nlohmann::json::parse(matchedText)["url"].get<std::string>()));
        return QString::fromStdString (url);
    };
};

#endif // TANGRENJIE_H
