
//#ifndef HUALE_H
//#define HUALE_H

//#include <parsers/showparser.h>


//class HuaLe: public ShowParser
//{
//public:
//    HuaLe(){};

//    // ShowParser interface
//public:
//    int providerEnum(){
//        return Providers::e_HuaLe;
//    };
//    QString name(){
//        return "华乐";
//    };
//    QString hostUrl {
//        return "https://www.huale.org";
//    };
//    QVector<ShowResponse> filterSearch(int page, QString sortBy = "time",int type = 1,QString subType = "", QString region = "",QString genre = "",QString language = "",QString letter= "",QString year= "" ) {
//        QVector<ShowResponse> shows;
//        // 全部类型 = 1 动作片 喜剧片 爱情片 科幻片 伦理片 恐怖片 剧情片 战争片 冒险片 动画电影 同性片 奇幻片 悬疑片 惊悚片 歌舞片 灾难片 犯罪片 经典片 网络电影
//        // regions 大陆 香港 台湾 美国 法国 英国 日本 韩国 德国 泰国 印度 意大利 西班牙 加拿大 其他
//        // 喜剧 爱情 恐怖 动作 科幻 剧情 战争 警匪 犯罪 动画 奇幻 武侠 冒险 枪战 恐怖 悬疑 惊悚 经典 青春 文艺 微电影 古装 历史 运动 农村 儿童 网络电影
//        // 动作片 喜剧片 爱情片 科幻片 伦理片 恐怖片 剧情片 战争片 冒险片 动画电影 同性片 奇幻片 悬疑片 惊悚片 歌舞片 灾难片 犯罪片 经典片 网络电影
//        // 国语 英语 粤语 闽南语 韩语 日语 法语 德语 其它
//        // 2022 2021 20202019201820172016201520142013201220112010
//        // a-z 0-9
//        // time
//        auto url = QString("https://www.huale.org/vodshow/%1%2-%3-%4-%5-%6-%7---%8---%9.html").arg(QString::number (type),subType,region,sortBy,genre,language,letter,QString::number (page),year).toStdString ();

//        NetworkClient::get(url).document ().select ("//div[@class='module-list']/div[@class='module-items']/div").forEach ([&](pugi::xpath_node node){
//            ShowResponse show;
//            auto anchor = node.selectFirst (".//div[@class='module-item-pic']/a");
//            show.title = anchor.attr ("title").as_string ();
//            show.coverUrl = node.selectFirst (".//div[@class='module-item-pic']/img").attr ("data-src").as_string ();
//            show.releaseDate = node.selectFirst (".//div[@class='module-item-caption']/span").toString ().c_str ();
//            show.latestTxt = node.selectFirst (".//div[@class='module-item-pic']/a").toString ().c_str ();
//            show.link = QString::fromStdString (hostUrl) + anchor.attr ("href").as_string ();
//            show.provider = Providers::e_HuaLe;
//            shows.push_back(std::move(show));
//        });
//        return shows;
//    };

//    QVector<ShowResponse> search(QString query, int page, int type){
//        QVector<ShowResponse> animes;



//        return animes;
//    };

//    QVector<ShowResponse> popular(int page, int type){
//        return filterSearch (page,"hits");
//    };
//    QVector<ShowResponse> latest(int page, int type){
//        return filterSearch (page,"time");
//    };

//    QVector<ShowResponse> fetchMore(){
//        QVector<ShowResponse> animes;

//        return animes;
//    };

//    ShowResponse loadDetails(ShowResponse show){
//        qDebug()<<show.link;
//        auto doc = NetworkClient::get (show.link.toStdString ()).document ();
//        auto count = 1;
//        auto infoItem = doc.selectFirst("//div[@class='video-info-main']/div[@class='video-info-items'][7]");
//        show.description = infoItem.selectFirst (".//div/span").node ().child_value ();
//        doc.select ("//div[@class='scroll-content']/a").forEach ([&](pugi::xpath_node element){
//            Episode episode;
//            episode.title = element.attr ("title").as_string ();
//            episode.title = episode.title.remove (0,2+show.title.length ());
//            episode.link = hostUrl+element.attr ("href").as_string ();
//            episode.number = count++;
//            show.episodes.append (std::move(episode));
//        });
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

//#endif // HUALE_H
