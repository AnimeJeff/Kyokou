#pragma once
#include <QAbstractListModel>
#include <QtConcurrent>

#include "Explorer/Data/showdata.h"



class SearchResultsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)

    bool fetchingMore = false;
    QTimer m_timeoutTimer;
    const int timeoutDuration = 500;
    bool loading = false;
    QFutureWatcher<QVector<ShowData>> m_watcher{};
    QVector<ShowData> m_list;
    QString m_cancelReason;
public:
    explicit SearchResultsModel(QObject *parent = nullptr)
        : QAbstractListModel(parent){
        m_timeoutTimer.setSingleShot (true);
        m_timeoutTimer.setInterval (5000);
        connect(&m_timeoutTimer, &QTimer::timeout, this,[this](){
            m_cancelReason = "Loading took too long";
            m_watcher.future ().cancel ();
        });
    }

    Q_INVOKABLE void search(const QString& query,int page,int type);
    Q_INVOKABLE void latest(int page,int type);
    Q_INVOKABLE void popular(int page,int type);
    Q_INVOKABLE void loadShow(int index);
    Q_INVOKABLE void cancel()
    {
        if(m_watcher.isRunning ())
        {
            m_watcher.cancel ();
            setLoading (false);
        }
    }

    Q_INVOKABLE void reload();
    bool isLoading()
    {
        return loading;
    }
    void setLoading(bool b)
    {
        loading = b;
        emit loadingChanged();
    }
public:
    Q_INVOKABLE void loadMore();;
    Q_INVOKABLE bool canLoadMore()const;
signals:
    void loadingChanged(void);

private:
    void setResults(QVector<ShowData> results);


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

