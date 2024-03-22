#pragma once
#include <QAbstractListModel>
#include <QtConcurrent>

#include "Explorer/Data/showdata.h"



class SearchResultsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    bool fetchingMore = false;
    bool loading = false;
    QFutureWatcher<QList<ShowData>> m_watcher;
    QList<ShowData> m_list;
    QString m_cancelReason;
    int m_currentPageIndex;
    bool m_canFetchMore = false;
    std::function<void(int)> lastSearch;
    void setLoading(bool b)
    {
        loading = b;
        emit loadingChanged();
    }

public:
    explicit SearchResultsModel(QObject *parent = nullptr);

    ShowData &at(int index) { return m_list[index]; }
    bool isLoading() { return loading; }
    void search(const QString& query,int page,int type, ShowProvider* provider);
    void latest(int page, int type, ShowProvider* provider);
    void popular(int page, int type, ShowProvider* provider);

signals:
    void loadingChanged(void);
public slots:
    void cancel();
    void reload();
    void loadMore();
    bool canLoadMore() const;


private:
    enum
    {
        TitleRole = Qt::UserRole,
        CoverRole
    };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> names;
        names[TitleRole] = "title";
        names[CoverRole] = "cover";
        return names;
    };

};

