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
                if (m_show.playlist && m_show.lastWatchedIndex >= 0) {
                    m_show.playlist->currentIndex = m_show.lastWatchedIndex;
                    // If the last watched index is greater than the total number episodes
                    // then set it to the index of the max episode
                    if (m_show.playlist->currentIndex >= m_show.playlist->count ()){
                        m_show.playlist->currentIndex = m_show.playlist->count () - 1;
                        //TODO update in watch list?
                    }
                }
                m_episodeListModel.setPlaylist(m_show.playlist);
                m_episodeListModel.setIsReversed(true);

                emit showChanged();
            } catch (QException& ex) {
                ErrorHandler::instance().show (ex.what ());
                m_episodeListModel.setPlaylist (nullptr);
            }
        }
    });



}

ShowManager::~ShowManager() {}

void ShowManager::setShow(const ShowData &show) {
  if (m_watcher.isRunning())
    return;
  if (m_show.link == show.link) {
    emit showChanged();
    return;
  }


  m_show = show;
  m_watcher.setFuture(QtConcurrent::run([this]() {
    if (m_show.provider) {
      qDebug() << "Log (ShowManager)： Loading details for" << m_show.title
               << "with" << m_show.provider->name()
               << "using the link:" << m_show.link;
      m_show.provider->loadDetails(m_show);
      qDebug() << "Log (ShowManager)： Successfully loaded details for"
               << m_show.title;
    } else {
      throw MyException(
          QString("Error: Unable to find a provider for %1").arg(m_show.title));
    }
  }));
}

void ShowManager::updateLastWatchedIndex(int index) {
    Q_ASSERT(m_show.playlist);
    m_show.playlist->currentIndex = index;
    m_show.lastWatchedIndex = index;
    m_episodeListModel.updateLastWatchedIndex();
}

int ShowManager::correctIndex(int index) const {
    int correctedIndex = m_episodeListModel.getIsReversed () ? m_show.playlist->count () - index - 1: index;
    Q_ASSERT(correctedIndex > -1 && correctedIndex < m_show.playlist->count ());
    return correctedIndex;
}

int ShowManager::getContinueIndex() const {
    if (!m_show.playlist) return -1;
    int index = m_episodeListModel.getContinueIndex ();
    return index;
}

void ShowManager::setListType(int listType) {
    m_show.listType = listType;
    emit listTypeChanged ();
}
