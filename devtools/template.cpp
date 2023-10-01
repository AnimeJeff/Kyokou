#pragma once
#include "template.h"

QVector<ShowData> NewProvider::search(QString query, int page, int type)
{
    QVector<ShowData> shows;
    if(query.isEmpty ()){
        m_canFetchMore = false;
        return shows;
    }
    std::string url = hostUrl + "/search.html?keyword=" + Functions::urlEncode (query.toStdString ())+ "&page="+ std::to_string (page);
    auto showNodes = NetworkClient::get(url).document().select("//ul[@class='items']/li");

    if(showNodes.empty ())
    {
        m_canFetchMore = false;
        return shows;
    }

    for (pugi::xpath_node_set::const_iterator it = showNodes.begin(); it != showNodes.end(); ++it)
    {
        auto anchor = it->selectFirst(".//ul[@class='items']/a");
        QString title = anchor.attr("title").as_string();
        QString coverUrl = it->selectFirst(".//img").attr("src").as_string();
        std::string link = anchor.attr ("href").as_string ();
        shows.emplaceBack(ShowData(title,link,coverUrl,Providers::NEWPROVIDER));
    }
    m_canFetchMore = true;
    m_currentPage = page;
    lastSearch = [query,this,type]{
        return search(query,++m_currentPage,type);
    };
    return shows;
}

QVector<ShowData> NewProvider::popular(int page, int type)
{
    QVector<ShowData> shows;
    std::string url = "";
    auto showNodes = NetworkClient::get(url).document().select("//ul[@class='items']/li");

    if(showNodes.empty ()){
        m_canFetchMore = false;
        return shows;
    }

    for (pugi::xpath_node_set::const_iterator it = showNodes.begin(); it != showNodes.end(); ++it)
    {
        auto anchor = it->selectFirst(".//ul[@class='items']/a");
        QString title = anchor.attr("title").as_string();
        QString coverUrl = it->selectFirst(".//img").attr("src").as_string();
        std::string link = anchor.attr ("href").as_string ();
        shows.emplaceBack(ShowData(title,link,coverUrl,Providers::NEWPROVIDER));
    }
    m_canFetchMore = true;
    m_currentPage = page;
    lastSearch = [this,type]{
        return popular(++m_currentPage,type);
    };
    return shows;
}
QVector<ShowData> NewProvider::latest(int page, int type)
{
    QVector<ShowData> shows;
    std::string url = "";
    auto showNodes = NetworkClient::get(url).document().select("//ul[@class='items']/li");
    if(showNodes.empty ()){
        m_canFetchMore = false;
        return shows;
    }

    for (pugi::xpath_node_set::const_iterator it = showNodes.begin(); it != showNodes.end(); ++it)
    {
        auto anchor = it->selectFirst(".//ul[@class='items']/a");
        QString title = anchor.attr("title").as_string();
        QString coverUrl = it->selectFirst(".//img").attr("src").as_string();
        std::string link = anchor.attr ("href").as_string ();
        shows.emplaceBack(ShowData(title,link,coverUrl,Providers::NEWPROVIDER));
    }
    m_canFetchMore = true;
    m_currentPage = page;
    lastSearch = [this,type]{
        return latest(++m_currentPage,type);
    };

    return shows;
}

void NewProvider::loadDetails(ShowData& show)
{
    NetworkClient::get(show.link).document();
}
int NewProvider::getTotalEpisodes(const ShowData &show)
{
    return 0;
}
QVector<VideoServer> NewProvider::loadServers(const PlaylistItem *episode)
{
    QVector<VideoServer> servers;
    NetworkClient::get(episode->link).document();
    return servers;
}
QString NewProvider::extractSource(VideoServer &server)
{
    return "";
}