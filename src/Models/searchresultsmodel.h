#pragma once
#include <QAbstractListModel>
#include <QtConcurrent>

#include "Data/showdata.h"



class SearchResultsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
public:
    explicit SearchResultsModel(QObject *parent = nullptr);
    ShowData &at(int index) { return m_list[index]; }

    void search(const QString& query,int page,int type, ShowProvider* provider);
    void latest(int page, int type, ShowProvider* provider);
    void popular(int page, int type, ShowProvider* provider);
    Q_INVOKABLE bool canLoadMore() const;
signals:
    void isLoadingChanged(void);
public slots:
    void cancel();
    void reload();
    void loadMore();
private:
    bool m_isLoading = false;
    QFutureWatcher<QList<ShowData>> m_watcher;
    QList<ShowData> m_list;
    QString m_cancelReason;
    int m_currentPageIndex;
    bool m_canFetchMore = false;
    std::function<void(int)> lastSearch;
    void setIsLoading(bool b) {
        m_isLoading = b;
        emit isLoadingChanged();
    }

    bool isLoading() { return m_isLoading; }

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

