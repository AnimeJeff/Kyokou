#pragma once
#include <QFutureWatcher>
#include <QNetworkReply>
#include <QString>
#include "Explorer/Data/showdata.h"
#include "Components/network.h"
#include "Components/ErrorHandler.h"
#include "Components/MyException.h"
#include "Components/functions.h"



class ShowProvider: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT);
protected:
    bool m_canFetchMore = false;
    int m_currentPage = 0;
    std::function<QVector<ShowData>(int)> lastSearch;
public:
    ShowProvider(QObject* parent = nullptr): QObject(parent) {};
    virtual QString name() const = 0;
    std::string hostUrl = "";

    virtual QVector<ShowData> search(QString query,int page,int type) = 0;
    virtual QVector<ShowData> popular(int page,int type) = 0;
    virtual QVector<ShowData> latest(int page,int type) = 0;

    virtual QVector<ShowData> reload() const final { return lastSearch(m_currentPage); }

    virtual bool canFetchMore() const final { return m_canFetchMore; }
    virtual QVector<ShowData> fetchMore() const final { return lastSearch(m_currentPage + 1); }

    virtual void loadDetails(ShowData& show) const  = 0;
    virtual int getTotalEpisodes(const ShowData& show) const = 0;
    virtual QVector<VideoServer> loadServers(const PlaylistItem* episode) const = 0;
    virtual QString extractSource(VideoServer& server) const = 0;

    virtual QList<int> getAvailableTypes() const {return {};};
};


