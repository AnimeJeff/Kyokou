#pragma once
#include <QAbstractListModel>
class EpisodeListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool reversed READ getIsReversed WRITE setIsReversed NOTIFY reversedChanged)
    Q_PROPERTY(QString continueEpisodeName READ getContinueEpisodeName NOTIFY continueIndexChanged)
    Q_PROPERTY(int continueIndex READ getContinueIndex NOTIFY continueIndexChanged)

    bool isReversed = false;
    int continueIndex = -1;

    QString getContinueEpisodeName();
    int getContinueIndex()
    {
        return continueIndex;
    }
public slots:
    void updateLastWatchedName();

signals:
    void continueIndexChanged(void);
    void reversedChanged(void);
public:
    void setIsReversed(bool isReversed)
    {
        if (this->isReversed == isReversed) return;
        this->isReversed = isReversed;
        emit layoutChanged ();
        emit reversedChanged();
    }
    bool getIsReversed() const
    {
        return isReversed;
    }
    explicit EpisodeListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {};

    enum {
        TitleRole = Qt::UserRole,
        NumberRole,
        FullTitleRole
    };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;;
private:
};
