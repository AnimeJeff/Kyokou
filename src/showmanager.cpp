#include "showmanager.h"
#include "Components/errorhandler.h"
#include "Providers/showprovider.h"

ShowManager::ShowManager(QObject *parent) : QObject{parent} {
    QObject::connect (&m_watcher, &QFutureWatcher<void>::finished, this, [this](){

        if (!m_watcher.future().isValid()) {
            //future was cancelled
            ErrorHandler::instance().show ("Operation cancelled");
            m_episodeListModel.setPlaylist (nullptr);
        } else {
            try {
                m_episodeListModel.setPlaylist(m_show.playlist);
                m_episodeListModel.setIsReversed(true);


            } catch (QException& ex) {
                ErrorHandler::instance().show (ex.what ());
                m_episodeListModel.setPlaylist (nullptr);
            }
        }
        emit showChanged();
    });



}

ShowManager::~ShowManager() {}

void ShowManager::setShow(const ShowData &show, ShowData::LastWatchInfo lastWatchInfo) {
    if (m_watcher.isRunning())
        return;
    if (m_show.link == show.link) {
        emit showChanged();
        return;
    }

    m_show = show;
    m_show.lastWatchedIndex = lastWatchInfo.lastWatchedIndex;
    m_show.listType = lastWatchInfo.listType;


    m_watcher.setFuture(QtConcurrent::run([this, lastWatchInfo]() {
        if (m_show.provider) {
            qInfo() << "Log (ShowManager)： Loading details for" << m_show.title
                    << "with" << m_show.provider->name()
                    << "using the link:" << m_show.link;

            m_show.provider->loadDetails(m_show);
            if (m_show.playlist) {
                qDebug() << "Setting last watch info for" << m_show.title
                         << lastWatchInfo.lastWatchedIndex << lastWatchInfo.lastPlayTime;
                m_show.playlist->setLastPlayAt(lastWatchInfo.lastWatchedIndex, lastWatchInfo.lastPlayTime);
            }


            qInfo() << "Log (ShowManager)： Successfully loaded details for" << m_show.title;
        } else {
            throw MyException(
                QString("Error: Unable to find a provider for %1").arg(m_show.title));
        }
    }));
}

void ShowManager::updateLastWatchedIndex(int index) {
    Q_ASSERT(m_show.playlist);
    if (index == m_show.lastWatchedIndex) return;

    // m_show.playlist->setLastPlayAt (index, 0);
    m_show.lastWatchedIndex = index;
    // m_show.playlist->setLastPlayAt (index, 0);
    m_episodeListModel.updateLastWatchedIndex();
}

int ShowManager::correctIndex(int index) const {
    int correctedIndex = m_episodeListModel.getIsReversed () ? m_show.playlist->size () - index - 1: index;
    Q_ASSERT(correctedIndex > -1 && correctedIndex < m_show.playlist->size ());
    return correctedIndex;
}

int ShowManager::getContinueIndex() const {
    int index = m_episodeListModel.getContinueIndex ();
    return index;
}

void ShowManager::setListType(int listType) {
    m_show.listType = listType;
    emit listTypeChanged ();
}
