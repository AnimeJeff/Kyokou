#pragma once
#include "Data/playlistitem.h"
#include <QAbstractListModel>
class EpisodeListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(bool reversed READ getIsReversed WRITE setIsReversed NOTIFY reversedChanged)
    Q_PROPERTY(QString continueText READ getContinueText NOTIFY continueIndexChanged)
    Q_PROPERTY(int continueIndex READ getContinueIndex NOTIFY continueIndexChanged)
    Q_PROPERTY(int lastWatchedIndex READ getLastWatchedIndex NOTIFY continueIndexChanged)

    int getLastWatchedIndex() const {
        if (!m_playlist || m_playlist->currentIndex == -1) return -1;
        int lastWatchedIndex = m_isReversed
                                   ? m_playlist->size() - 1 -  m_playlist->currentIndex
                                   : m_playlist->currentIndex;
        return lastWatchedIndex;
    }

    bool m_isReversed = false;
    int continueIndex = -1;
    PlaylistItem *m_playlist = nullptr;
    QString m_continueText = "";
    QString getContinueText() { return m_continueText; };

signals:
    void continueIndexChanged(void);
    void reversedChanged(void);

public:
    void setPlaylist(PlaylistItem *playlist) {
        // Delete the previous show playlist that is being held in info tab
        // if useCount < 0, it means that it has already been deleted
        if (m_playlist && --m_playlist->useCount == 0){
            delete m_playlist;
        }
        if (playlist) playlist->useCount++; //playlist could be nullptr
        m_playlist = playlist;

        updateLastWatchedIndex ();
    }

    void updateLastWatchedIndex() {
        if (m_playlist){
            // If the index in second to last of the latest episode then continue from latest episode
            continueIndex = m_playlist->currentIndex == m_playlist->size () - 2 ? m_playlist->currentIndex + 1 : m_playlist->currentIndex;

            if (continueIndex < 0) continueIndex = 0;

            const PlaylistItem *episode = m_playlist->at(continueIndex);
            m_continueText = m_playlist->currentIndex == -1 ? "Play " : "Continue from ";
            m_continueText += episode->name.isEmpty ()
                                  ? QString::number (episode->number)
                                  : episode->number < 0
                                        ? episode->name
                                        : QString::number (episode->number) + "\n" + episode->name;

        } else {
            continueIndex = -1;
            m_continueText = "";
        }

        emit continueIndexChanged();
    }

    int getContinueIndex() const {
        if (!m_playlist) return -1;
        return m_isReversed ? m_playlist->size() - 1 -  continueIndex : continueIndex;
    }


    void setIsReversed(bool isReversed) {
        if (m_isReversed == isReversed)
            return;
        m_isReversed = isReversed;
        emit layoutChanged();
        emit reversedChanged();
    }
    bool getIsReversed() const { return m_isReversed; }
    explicit EpisodeListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {};
    ~EpisodeListModel() {
        if (m_playlist && --m_playlist->useCount == 0) {
            delete m_playlist;
        }
    }
    enum { TitleRole = Qt::UserRole, NumberRole, FullTitleRole };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
};


