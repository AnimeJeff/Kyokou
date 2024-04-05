#include "showmanager.h"
#include "Components/errorhandler.h"
#include "Providers/showprovider.h"


ShowManager::ShowManager(QObject *parent) : QObject{parent} {
    connect (&m_watcher, &QFutureWatcher<void>::finished, this, [this](){
        if (!m_watcher.future().isValid()) {
            //future was cancelled
            ErrorHandler::instance().show ("Operation cancelled");
            m_show = ShowData("", "", "", nullptr);
            m_episodeListModel.setPlaylist(nullptr);
            return;
        }
        qInfo() << "Log (ShowManager)： Successfully loaded details for" << m_show.title;
        m_isLoading = false;
        emit isLoadingChanged ();
        emit showChanged();
    });
}

void ShowManager::loadShow(ShowData show, ShowData::LastWatchInfo lastWatchInfo) {
    // auto prevShow = m_show;
    m_show = show;
    m_show.listType = lastWatchInfo.listType;
    if (!m_show.provider) {
        throw MyException(QString("Error: Unable to find a provider for %1").arg(m_show.title));
        return;
    }

    qInfo() << "Log (ShowManager)： Loading details for" << m_show.title
            << "with" << m_show.provider->name()
            << "using the link:" << m_show.link;
    try {
        m_show.provider->loadDetails(m_show);
    } catch(...) {
        qDebug() << "Failed to load show";
    }

    if (m_show.playlist) {
        qDebug() << "Setting last watch info for" << m_show.title
                 << lastWatchInfo.lastWatchedIndex << lastWatchInfo.timeStamp;
        m_show.playlist->setLastPlayAt(lastWatchInfo.lastWatchedIndex, lastWatchInfo.timeStamp);
    }

    m_episodeListModel.setPlaylist(m_show.playlist);
    m_episodeListModel.setIsReversed(true);

}


void ShowManager::setShow(const ShowData &show, ShowData::LastWatchInfo lastWatchInfo) {
    if (m_watcher.isRunning())
        return;
    if (m_show.link == show.link) {
        emit showChanged();
        return;
    }
    m_isLoading = true;
    emit isLoadingChanged ();
    m_watcher.setFuture(QtConcurrent::run(&ShowManager::loadShow, this, show, lastWatchInfo));



}

int ShowManager::correctIndex(int index) const {
    int correctedIndex = m_episodeListModel.getIsReversed () ? m_show.playlist->size () - index - 1: index;
    Q_ASSERT(correctedIndex > -1 && correctedIndex < m_show.playlist->size ());
    return correctedIndex;
}



void ShowManager::setListType(int listType) {
    m_show.listType = listType;
    emit listTypeChanged ();
}
