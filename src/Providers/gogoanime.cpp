#include "gogoanime.h"
#include "Extractors/gogocdn.h"


QList<ShowData> Gogoanime::search(QString query, int page, int type)
{
    QList<ShowData> animes;
    if (query.isEmpty ())
        return animes;
    std::string url = hostUrl + "/search.html?keyword=" + Functions::urlEncode (query.toStdString ())+ "&page="+ std::to_string (page);
    auto animeNodes = NetworkClient::get(url).document().select("//ul[@class='items']/li");
    if (animeNodes.empty ())
        return animes;

    for (pugi::xpath_node_set::const_iterator it = animeNodes.begin(); it != animeNodes.end(); ++it)
    {
        auto anchor = it->selectFirst(".//p[@class='name']/a");
        auto title = anchor.attr("title").as_string();
        auto coverUrl = it->selectFirst(".//img").attr("src").as_string();
        auto link = anchor.attr ("href").as_string ();
        animes.emplaceBack(title, link, coverUrl, this);
    }

    return animes;
}

QList<ShowData> Gogoanime::popular(int page, int type)
{
    QList<ShowData> animes;
    std::string url = "https://ajax.gogocdn.net/ajax/page-recent-release-ongoing.html?page=" + std::to_string (page);
    auto animeNodes = NetworkClient::get (url).document ().select ("//div[@class='added_series_body popular']/ul/li");
    for (pugi::xpath_node_set::const_iterator it = animeNodes.begin(); it != animeNodes.end(); ++it)
    {
        pugi::xpath_node anchor = it->selectFirst ("a");
        std::string link = anchor.attr("href").as_string ();
        QString coverUrl = QString(anchor.selectFirst(".//div[@class='thumbnail-popular']").attr ("style").as_string ());
        coverUrl = coverUrl.split ("'").at (1);
        QString title = anchor.attr ("title").as_string ();
        animes.push_back (ShowData (title, link, coverUrl, this));
        animes.last ().latestTxt = it->selectText (".//p[last()]/a");
    }

    return animes;
}

QList<ShowData> Gogoanime::latest(int page, int type)
{
    QList<ShowData> animes;
    std::string url = "https://ajax.gogocdn.net/ajax/page-recent-release.html?page=" + std::to_string (page) + "&type=1";
    auto response = NetworkClient::get(url);
    pugi::xpath_node_set animeNodes = response.document().select("//ul[@class='items']/li");
    if (animeNodes.empty()) return animes;

    for (pugi::xpath_node_set::const_iterator it = animeNodes.begin(); it != animeNodes.end(); ++it) {
        QString coverUrl = it->selectFirst(".//img").attr("src").as_string();
        static QRegularExpression re{R"(([\w-]*?)(?:-\d{10})?\.)"};
        auto lastSlashIndex =  coverUrl.lastIndexOf("/");
        auto id = re.match (coverUrl.mid(lastSlashIndex + 1));
        if (!id.hasMatch ()) {
            qDebug() << "Unable to extract Id from" << coverUrl.mid(lastSlashIndex+1);
            continue;
        }
        QString title = QString(it->selectFirst (".//p[@class='name']/a").node ().child_value ()).trimmed ().replace("\n", " ");
        std::string link = "/category/" + id.captured (1).toStdString ();
        animes.emplaceBack (title, link, coverUrl, this, it->selectText (".//p[@class='episode']"));
    }


    return animes;
}

void Gogoanime::loadDetails(ShowData &anime) const
{
    CSoup doc = getInfoPage (anime.link);
    int lastEpisode = std::stoi(doc.selectFirst ("//ul[@id='episode_page']/li[last()]/a").attr ("ep_end").as_string ());
    std::string animeId = doc.selectFirst ("//input[@id='movie_id']").attr ("value").as_string ();
    std::string alias = doc.selectFirst ("//input[@id='alias_anime']").attr ("value").as_string ();
    // std::string epStart = lastEpisode > 1000 ? std::to_string(lastEpisode - 99) : "0";
    std::string epStart = "0";
    std::string link = "https://ajax.gogocdn.net/ajax/load-list-episode?ep_start="
                       + epStart + "&ep_end="  + std::to_string (lastEpisode)
                       + "&id=" + animeId + "&default_ep=0" + "&alias=" + alias;

    pugi::xpath_node_set episodeNodes = NetworkClient::get(link).document().select("//li/a");
    for (pugi::xpath_node_set::const_iterator it = episodeNodes.end() - 1; it != episodeNodes.begin() - 1; --it) {
        QString title = QString::fromStdString (it->selectText (".//div")).replace("EP", "").trimmed ();
        float number = -1;
        bool ok;
        float intTitle = title.toFloat (&ok);
        if (ok){
            number = intTitle;
            title = "";
        }
        std::string link = it->attr ("href").value ();
        anime.addEpisode(number, QString::fromStdString (link), title);
    }

    if (auto pNodes = doc.select ("//div[@class='description']/p"); !pNodes.empty ()) {
        for (pugi::xpath_node_set::const_iterator it = pNodes.begin(); it != pNodes.end(); ++it)
        {
            anime.description += QString(it->node ().child_value ()).replace ("\n"," ").trimmed () + "\n\n";
        }
        anime.description = anime.description.trimmed ();
    } else {
        auto descriptionNode = doc.selectFirst("//div[@class='description']");
        if (!descriptionNode.node().first_child() && std::string(descriptionNode.node().child_value()).empty()) {
            qDebug() << "The div is empty.\n";
        } else {
            anime.description = QString(descriptionNode.node ().child_value ()).replace ("\n"," ").trimmed ();
        }
    }
    anime.status = doc.selectFirst ("//span[contains(text(),'Status')]/following-sibling::a").node ().child_value ();

    if (pugi::xml_node statusTextNode =
        doc.selectFirst ("//span[contains(text(),'Status')]/following-sibling::a").node ();
        statusTextNode.type() == pugi::node_pcdata)
        anime.releaseDate = statusTextNode.child_value ();

    if (pugi::xml_node releasedTextNode =
        doc.selectFirst ("//span[contains(text() ,'Released')]/following-sibling::text()").node ();
        releasedTextNode.type() == pugi::node_pcdata)
        anime.releaseDate = releasedTextNode.value();

    pugi::xpath_node_set genreNodes = doc.select ("//span[contains(text(),'Genre')]/following-sibling::a");
    for (pugi::xpath_node_set::const_iterator it = genreNodes.begin(); it != genreNodes.end(); ++it)
    {
        QString genre = QString(it->attr ("title").as_string ()).replace ("\n"," ");
        anime.genres.push_back (genre);
    }
}

CSoup Gogoanime::getInfoPage(const std::string& link) const
{
    auto response = NetworkClient::get(hostUrl + link);
    if (response.code == 404)
    {
        QString errorMessage = "Invalid URL: '" + QString::fromStdString (hostUrl + link +"'");
        MyException(errorMessage).raise();
    }
    return response.document ();
}

std::string Gogoanime::getEpisodesLink(const CSoup &doc) const
{
    std::string lastEpisode = doc.selectFirst ("//ul[@id='episode_page']/li[last()]/a").attr ("ep_end").as_string ();
    std::string animeId = doc.selectFirst ("//input[@id='movie_id']").attr ("value").as_string ();
    return "https://ajax.gogocdn.net/ajax/load-list-episode?ep_start=0&ep_end=" + lastEpisode + "&id=" + animeId;
}

int Gogoanime::getTotalEpisodes(const std::string& link) const
{
    auto doc = getInfoPage (link);
    return NetworkClient::get(getEpisodesLink(doc)).document ().select("//ul/li/a").size ();
}

QList<VideoServer> Gogoanime::loadServers(const PlaylistItem *episode) const
{
    QList<VideoServer> servers;
    pugi::xpath_node_set serverNodes = NetworkClient::get(hostUrl + episode->link.toStdString ()).document ().select("//div[@class='anime_muti_link']/ul/li/a");
    for (pugi::xpath_node_set::const_iterator it = serverNodes.begin (); it != serverNodes.end(); ++it)
    {
        QString link = it->attr("data-video").as_string ();
        QString name = QString(it->node().child_value ()).trimmed ();
        if (link.startsWith ("//")){
            link = "https:" + link;
        }
        servers.emplaceBack (name, link);
    }
    return servers;
}

QList<Video> Gogoanime::extractSource(const VideoServer &server) const {
    auto serverName = server.name.toLower ();
    try {
        if (serverName.contains ("vidstreaming") ||
            serverName.contains ("gogo"))
        {
            GogoCDN extractor;
            auto source = extractor.extract(server.link.toStdString ());
            return { Video(source) };
        }
    }
    catch (QException &e)
    {
        ErrorHandler::instance ().show ("Cannot find extract " + server.link);
    }

    return {};
}
