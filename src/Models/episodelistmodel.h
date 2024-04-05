#pragma once
#include "Data/playlistitem.h"
#include <QAbstractListModel>
class EpisodeListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(bool reversed READ getIsReversed WRITE setIsReversed NOTIFY reversedChanged)
    Q_PROPERTY(QString continueText READ getContinueText NOTIFY continueIndexChanged)
    Q_PROPERTY(int continueIndex READ getContinueIndex NOTIFY continueIndexChanged)
    Q_PROPERTY(int lastWatchedIndex READ getLastWatchedIndex NOTIFY continueIndexChanged)


    bool m_isReversed = false;
    int continueIndex = -1;
    std::unique_ptr<PlaylistItem> m_rootItem = std::make_unique<PlaylistItem>("root", nullptr, "/");

    QString m_continueText = "";
    QString getContinueText() { return m_continueText; };

signals:
    void continueIndexChanged(void);
    void reversedChanged(void);

public:
    void setPlaylist(PlaylistItem *playlist) {
        if (!playlist) {
            m_rootItem->clear ();
        } else {
            if (!m_rootItem->isEmpty ()) {
                m_rootItem->replace (0, playlist);
            } else {
                m_rootItem->append(playlist);
                m_rootItem->currentIndex = 0;
            }
        }
        updateLastWatchedIndex ();
    }

    void updateLastWatchedIndex() {
        if (auto playlist = m_rootItem->getCurrentItem (); playlist){
            // If the index in second to last of the latest episode then continue from latest episode
            continueIndex = playlist->currentIndex == playlist->size () - 2 ? playlist->currentIndex + 1 : playlist->currentIndex;

            if (continueIndex < 0) continueIndex = 0;

            const PlaylistItem *episode = playlist->at(continueIndex);
            m_continueText = playlist->currentIndex == -1 ? "Play " : "Continue from ";
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
        auto playlist = m_rootItem->getCurrentItem ();
        if (!playlist) return -1;
        return m_isReversed ? playlist->size() - 1 -  continueIndex : continueIndex;
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
    ~EpisodeListModel() = default;
private:
    int getLastWatchedIndex() const {
        auto currentPlaylist = m_rootItem->getCurrentItem ();
        if (!m_rootItem->getCurrentItem ()) return -1;
        int lastWatchedIndex = m_isReversed
                                   ? currentPlaylist->size() - 1 -  currentPlaylist->currentIndex
                                   : currentPlaylist->currentIndex;
        return lastWatchedIndex;
    }

    enum { TitleRole = Qt::UserRole, NumberRole, FullTitleRole };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
};


