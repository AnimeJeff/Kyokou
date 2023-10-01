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

    QVector<ShowData> search(QString query, int page, int type)
    {
        QVector<ShowData> shows;
        if(query.isEmpty ()){
            m_canFetchMore = false;
            return shows;
        }
        shows.push_back (ShowData("One Piece","https://gogocdn.net/images/anime/One-piece.jpg","https://gogocdn.net/images/anime/One-piece.jpg",TEST));
        shows.push_back (ShowData("Detective Conan","","https://gogocdn.net/cover/detective-conan.png",TEST));


        m_canFetchMore = true;
        m_currentPage = page;
        lastSearch = [query,this,type]{
            return search(query,++m_currentPage,type);
        };
        return shows;
    }

    QVector<ShowData> popular(int page, int type)
    {
        QVector<ShowData> shows;
        std::string url = "";
        shows.push_back (ShowData("One Piece","https://gogocdn.net/images/anime/One-piece.jpg","https://gogocdn.net/images/anime/One-piece.jpg",TEST));
        shows.push_back (ShowData("Detective Conan","","https://gogocdn.net/cover/detective-conan.png",TEST));

        m_canFetchMore = true;
        m_currentPage = page;
        lastSearch = [this,type]{
            return popular(++m_currentPage,type);
        };
        return shows;
    }

    QVector<ShowData> latest(int page, int type)
    {
        QVector<ShowData> shows;
        std::string url = "";
        shows.push_back (ShowData("One Piece","https://gogocdn.net/images/anime/One-piece.jpg","https://gogocdn.net/images/anime/One-piece.jpg",TEST));
        shows.push_back (ShowData("Detective Conan","","https://gogocdn.net/cover/detective-conan.png",TEST));
        m_canFetchMore = true;
        m_currentPage = page;
        lastSearch = [this,type]{
            return latest(++m_currentPage,type);
        };

        return shows;
    }

    void loadDetails(ShowData& show)
    {
        //NetworkClient::get(show.link).document();
        show.description = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
        qDebug() << "loading" << show.title << "from test provider";
        show.addEpisode (1,"lold","fds");
        show.addEpisode (2,"lold","fds");
    }

    int getTotalEpisodes(const ShowData &show)
    {
        return 0;
    }

    QVector<VideoServer> loadServers(const PlaylistItem *episode)
    {
        QVector<VideoServer> servers;
        servers.push_back (VideoServer(episode->name,episode->link));
        return servers;
    }
    QString extractSource(VideoServer &server)
    {
        return "D:\\TV\\batman.mp4";
    }
};

#endif
