
#include "kimcartoon.h"

#include <QTextDocument>

QVector<ShowData> Kimcartoon::search(QString query, int page, int type) {
    QVector<ShowData> shows;
    if (page > 1 || query.isEmpty())
        return shows;
    auto url = "https://kimcartoon.li/Search/Cartoon";
    auto showNodes = NetworkClient::post (url, {}, { {"keyword", query.toStdString ()}})
                         .document ().select("//div[@class='list-cartoon']/div/a[1]");

    for (pugi::xpath_node_set::const_iterator it = showNodes.begin(); it != showNodes.end(); ++it) {
        QString title = QString(it->selectText (".//span")).replace ('\n'," ").trimmed ();
        QString coverUrl = it->selectFirst (".//img").attr ("src").as_string ();
        if (coverUrl.startsWith ('/')) coverUrl = QString::fromStdString (hostUrl) + coverUrl;
        std::string link = it->attr ("href").as_string ();
        shows.emplaceBack(title, link, coverUrl, this, "", ShowData::ANIME);
    }

    return shows;
}

QVector<ShowData> Kimcartoon::popular(int page, int type) {
    std::string url =
        hostUrl + "CartoonList/MostPopular" + "?page=" + std::to_string(page);


    return filterSearch(url);
}

QVector<ShowData> Kimcartoon::latest(int page, int type) {
    std::string url = hostUrl + "CartoonList/LatestUpdate" + "?page=" + std::to_string(page);
    return filterSearch(url);
}

QVector<ShowData> Kimcartoon::filterSearch(std::string url) {
    QVector<ShowData> shows;
    auto showNodes = NetworkClient::get(url).document().select("//div[@class='list-cartoon']/div/a[1]");
    if (showNodes.empty()) {
        return shows;
    }

    for (pugi::xpath_node_set::const_iterator it = showNodes.begin(); it != showNodes.end(); ++it) {
        QString title = QString(it->selectText (".//span")).replace ('\n'," ").trimmed ();
        QString coverUrl = it->selectFirst("./img").attr("src").as_string();
        if (coverUrl.startsWith ('/')) coverUrl = QString::fromStdString (hostUrl) + coverUrl;
        std::string link = it->attr("href").as_string();
        shows.emplaceBack(title, link, coverUrl, this);
    }

    return shows;
    ;
}

void Kimcartoon::loadDetails(ShowData &show) const {
    auto doc = NetworkClient::get(hostUrl + show.link).document();
    auto infoDiv = doc.selectFirst("//div[@class='barContent']");

    if (pugi::xml_node descriptionParagraph
        = infoDiv.selectFirst("//span[contains(text() ,'Summary')]/parent::p/following-sibling::p[1]").node ())
        show.description = QString(descriptionParagraph.child_value ()).replace ('\n'," ").replace ("&nbsp"," ").trimmed ();


    if (pugi::xml_node dateAiredTextNode
        = doc.selectFirst ("//span[contains(text() ,'Date')]/following-sibling::text()").node ();
        dateAiredTextNode.type() == pugi::node_pcdata) {
        show.releaseDate = dateAiredTextNode.value();
    }

    if (pugi::xml_node statusTextNode =
        infoDiv.selectFirst (".//span[contains(text() ,'Status')]/following-sibling::text()[1]").node ();
        statusTextNode.type() == pugi::node_pcdata)
        show.status = statusTextNode.value();

    if (pugi::xml_node viewsTextNode =
        infoDiv.selectFirst (".//span[contains(text() ,'Views')]/following-sibling::text()[1]").node ();
        viewsTextNode.type() == pugi::node_pcdata)
        show.views = viewsTextNode.value();

    pugi::xpath_node_set genreNodes = infoDiv.node ().select_nodes ("//span[contains(text(),'Genres')]/following-sibling::a");
    for (pugi::xpath_node_set::const_iterator it = genreNodes.begin(); it != genreNodes.end(); ++it) {
        QString genre = it->node().child_value();
        show.genres.push_back(genre.trimmed ());
    }

    pugi::xpath_node_set episodeNodes = doc.select("//table[@class='listing']//a");
    for (int i = episodeNodes.size() - 1; i >= 0; --i) {
        const pugi::xpath_node *it = &episodeNodes[i];
        QStringList fullEpisodeName;
        if (auto episodeNameString = QString(it->node().child_value()).replace (show.title, "").trimmed ();
            episodeNameString.startsWith ("Episode")){
            fullEpisodeName = episodeNameString.remove (0, 8).split (" - ");
        } else {
            fullEpisodeName << episodeNameString;
        }

        int number = -1;
        QString title;
        if (fullEpisodeName.size () == 2) {
            bool ok;
            if (int intTitle = fullEpisodeName.first ().toInt (&ok); ok)
                number = intTitle;
            else title = fullEpisodeName.last ().replace ("\n", "").trimmed ();
        } else {
            title = fullEpisodeName.last ().replace ("\n", "").trimmed ();
        }

        QString link = it->attr("href").value();
        show.addEpisode(number, link, title);
    }

}

QVector<VideoServer>
Kimcartoon::loadServers(const PlaylistItem *episode) const {
    auto doc = NetworkClient::get(hostUrl + episode->link.toStdString ()).document();
    auto serverNodes = doc.select("//select[@id='selectServer']/option");
    QList<VideoServer> servers;
    for (pugi::xpath_node_set::const_iterator it = serverNodes.begin(); it != serverNodes.end(); ++it) {
        QString serverName = QString(it->node ().child_value ()).trimmed ();
        QString serverLink = it->attr ("value").as_string ();
        servers.emplaceBack (serverName, serverLink);
    }
    return servers;
}
QList<Video> Kimcartoon::extractSource(const VideoServer &server) const {
    auto doc = NetworkClient::get(hostUrl + server.link.toStdString ()).document();
    auto iframe = doc.select ("//iframe[@id='my_video_1']");
    if (iframe.empty ()) return {};
    std::string serverUrl = iframe.first ().attr ("src").as_string ();
    Functions::httpsIfy (serverUrl);
    auto response = NetworkClient::get(serverUrl, {{"sec-fetch-dest", "iframe"}}).body;
    std::smatch matches;
    if (std::regex_search(response, matches, sourceRegex)) {
        if (matches.size() > 1) {
            QString source = QString::fromStdString (matches[1].str ());
            Video video(source);
            video.addHeader ("Referer", "https://" + Functions::getHostFromUrl(serverUrl));
            return { video };
        }
    } else {
        qDebug() <<"Log (KimCartoon): No source found.";
    }


    return {};
}
