#pragma once
#include "Player/playlistitem.h"
#include <QAbstractListModel>
class EpisodeListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(bool reversed READ getIsReversed WRITE setIsReversed NOTIFY reversedChanged)
    Q_PROPERTY(QString continueEpisodeName READ getContinueEpisodeName NOTIFY continueIndexChanged)
    Q_PROPERTY(int continueIndex READ getContinueIndex NOTIFY continueIndexChanged)
    Q_PROPERTY(int lastWatchedIndex READ getLastWatchedIndex NOTIFY continueIndexChanged)

    int getLastWatchedIndex() const {
        if (!m_playlist || m_playlist->currentIndex == -1) return -1;
        int lastWatchedIndex = isReversed ? m_playlist->count() - 1 -  m_playlist->currentIndex : m_playlist->currentIndex;
        return lastWatchedIndex;
    }

    bool isReversed = false;
    int continueIndex = -1;
    PlaylistItem *m_playlist = nullptr;
    QString m_continueEpisodeName = "";
    QString getContinueEpisodeName() { return m_continueEpisodeName; };

signals:
    void continueIndexChanged(void);
    void reversedChanged(void);

public:
    void setPlaylist(PlaylistItem *playlist) {
        if (!playlist) return;
        m_playlist = playlist;
        updateLastWatchedIndex ();
    }
    int getContinueIndex() const { return continueIndex; }
    void updateLastWatchedIndex() {
        // If the index in second to last of the latest episode then continue from latest episode
        continueIndex = m_playlist->currentIndex == m_playlist->count () - 2 ? m_playlist->currentIndex + 1 : m_playlist->currentIndex;
        if (continueIndex > -1) {
            const PlaylistItem *episode = m_playlist->at(continueIndex);
            m_continueEpisodeName = episode->name.isEmpty () ? QString::number (episode->number) : episode->number < 0 ? episode->name : QString::number (episode->number) + "\n" + episode->name;
        }
        emit continueIndexChanged();
    }

    void setIsReversed(bool isReversed) {
        if (this->isReversed == isReversed)
            return;
        this->isReversed = isReversed;
        emit layoutChanged();
        emit reversedChanged();
    }
    bool getIsReversed() const { return isReversed; }
    explicit EpisodeListModel(QObject *parent = nullptr)
        : QAbstractListModel(parent){};

    enum { TitleRole = Qt::UserRole, NumberRole, FullTitleRole };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
};


