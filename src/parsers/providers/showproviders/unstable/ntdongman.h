//#ifndef NTDONGMAN_H
//#define NTDONGMAN_H

//#include <parsers/showparser.h>


//class NtDongMan: public ShowParser
//{
//public:
//    NtDongMan();

//    // ShowParser interface
//public:
//    int providerEnum(){
//        return Providers::e_NtDongMan;
//    };
//    QString name(){
//        return "NT动漫";
//    };
//    QString hostUrl {
//        return "http://www.ntdm8.com/";
//    };
//    QVector<ShowResponse> filterSearch(int page, std::string sortBy,std::string channel,std::string regionId = "0",std::string langId="0",std::string yearRange=" ") {
//        QVector<ShowResponse> animes;

//        //        NetworkClient::get("http://www.ntdm8.com"/show/riben---%E6%90%9E%E7%AC%91--------2023.html")


//        return animes;
//    };

//    QVector<ShowResponse> search(QString query, int page, int type){
//        QVector<ShowResponse> animes;



//        return animes;
//    };
//    QVector<ShowResponse> popular(int page, int type){
//        QVector<ShowResponse> animes;

//        return animes;
//    };
//    QVector<ShowResponse> latest(int page, int type){
//        QVector<ShowResponse> animes;
//        NetworkClient::get ("http://www.ntdm8.com/label/new/page/"+std::to_string (page)+".html").document ().select ("//li[@class='anime_icon2']").forEach([&](pugi::xpath_node element){
//            ShowResponse anime;
//            pugi::xpath_node anchor = element.selectFirst (".//a");
//            pugi::xpath_node img = anchor.selectFirst (".//img");
//            anime.link = QString::fromStdString (hostUrl+anchor.attr("href").as_string ());
//            anime.coverUrl = QString::fromStdString (img.attr ("src").as_string ());
//            anime.provider = Providers::e_NtDongMan;
//            anime.latestTxt = QString::fromStdString (anchor.attr("title").as_string ());
//            anime.title = QString::fromStdString(img.attr("alt").as_string ());
//            animes.push_back (std::move(anime));
//        });
//        return animes;
//    };

//    QVector<ShowResponse> fetchMore(){
//        QVector<ShowResponse> animes;

//        return animes;
//    };

//    ShowResponse loadDetails(ShowResponse show){
//        auto doc = NetworkClient::get (show.link.toStdString ()).document ();
//        show.description = doc.selectText ("//div[@class=detail_imform_desc_pre]/p");
//        auto count = 1;
//        doc.select ("//div[@id='main0']/div[1]/ul/li/a").forEach ([&](pugi::xpath_node element){
//            Episode episode;
//            episode.title = element.attr ("title").as_string ();
//            episode.link = hostUrl+element.attr ("href").as_string ();
//            episode.number = count++;
//            show.episodes.append (std::move(episode));
//        });
//        show.releaseDate = doc.selectText("//ul[@class='blockcontent']/li[@class='detail_imform_kv'][6]/span[2]/a");

//        return show;
//    };

//    QVector<VideoServer> loadServers(const Episode &episode){
//        VideoServer server;
//        server.name = "danmu";
//        server.link = NetworkClient::get (episode.link).document ().selectText ("//*[@id='ageframediv']/script[1]");
//        return QVector<VideoServer>{server};
//    };

//    QString extractSource(VideoServer& server){
//        return "";
//    };
//};

//#endif // NTDONGMAN_H
