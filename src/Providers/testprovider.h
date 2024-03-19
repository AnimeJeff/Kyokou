#ifdef QT_DEBUG
#pragma once
#include "showprovider.h"
class TestProvider : public ShowProvider
{
public:
    explicit TestProvider(QObject *parent = nullptr) : ShowProvider{parent}
        {

        };
public:
    QString name() const override { return "TestProvider"; }
    std::string hostUrl = "";

    QList<ShowData> search(QString query, int page, int type) override
    {
        QList<ShowData> shows;
        if (query.isEmpty ()){
            m_canFetchMore = false;
            return shows;
        }
        shows.push_back (ShowData("One Piece","https://gogocdn.net/images/anime/One-piece.jpg","https://gogocdn.net/images/anime/One-piece.jpg",name()));
        shows.push_back (ShowData("Detective Conan","https://gogocdn.net/cover/detective-conan.png","https://gogocdn.net/cover/detective-conan.png",name()));


        m_canFetchMore = true;
        m_currentPage = page;
        lastSearch = [query,this,type](int page){
            return search(query,page,type);
        };
        return shows;
    }

    QList<ShowData> popular(int page, int type) override
    {
        QList<ShowData> shows;
        std::string url = "";
        shows.push_back (ShowData("One Piece","https://gogocdn.net/images/anime/One-piece.jpg","https://gogocdn.net/images/anime/One-piece.jpg",name()));
        shows.push_back (ShowData("Detective Conan","https://gogocdn.net/cover/detective-conan.png","https://gogocdn.net/cover/detective-conan.png",name()));

        m_canFetchMore = true;
        m_currentPage = page;
        lastSearch = [this,type](int page){
            return popular(page,type);
        };
        return shows;
    }

    QList<ShowData> latest(int page, int type)override
    {
        QList<ShowData> shows;
        std::string url = "";
        shows.push_back (ShowData("One Piece","https://gogocdn.net/images/anime/One-piece.jpg","https://gogocdn.net/images/anime/One-piece.jpg",name()));
        shows.push_back (ShowData("Detective Conan","https://gogocdn.net/cover/detective-conan.png","https://gogocdn.net/cover/detective-conan.png",name()));
        m_canFetchMore = true;
        m_currentPage = page;
        lastSearch = [this,type](int page){
            return latest(page,type);
        };

        return shows;
    }

    void loadDetails(ShowData& show) const override
    {
        //NetworkClient::get(show.link).document();
        show.description = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
        qDebug() << "loading" << show.title << "from test provider";
        for (int i = 1; i< 101; ++i)
        {
            show.addEpisode (i,"","");
        }
    }

    int getTotalEpisodes(const std::string& link)const override
    {
        return 0;
    }

    QList<VideoServer> loadServers(const PlaylistItem *episode)const override
    {
        QList<VideoServer> servers;
        servers.push_back (VideoServer(episode->name,episode->link));
        return servers;
    }

    QString extractSource(const VideoServer &server) const override
    {
        return "D:\\TV\\batman.mp4";
    };
    QList<int> getAvailableTypes() const override
    {
        return {ShowData::ANIME};
    }
public:

};

#endif
