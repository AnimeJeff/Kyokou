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
public:
    explicit SearchResultsModel(QObject *parent = nullptr);
    bool isLoading()
    {
        return loading;
    }
    void setLoading(bool b)
    {
        loading = b;
        emit loadingChanged();
    }
    Q_INVOKABLE bool canLoadMore() const;
signals:
    void loadingChanged(void);
public slots:
    void search(const QString& query,int page,int type);
    void latest(int page,int type);
    void popular(int page,int type);
    void loadShow(int index);
    void cancel();
    void reload();
    void loadMore();

private:
    void setResults(QList<ShowData> results);
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

