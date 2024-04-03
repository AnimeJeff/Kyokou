#include "playlistitem.h"
#include <cmath>

PlaylistItem::PlaylistItem(float number, const QString &link, const QString &name, PlaylistItem *parent, bool isLocal)
    : number(number), name(name), link(link), m_parent(parent), type(isLocal ? LOCAL : ONLINE)
{
    // qDebug() << number << name;
    if (number > -1) {
        int dp = number == floor(number) ? 0 : 1;
        fullName = QString::number(number, 'f', dp) + (name.isEmpty() ? "" : ". ") + name;
    } else {
        fullName = name.isEmpty () ? "[Unnamed Episode]" : name;
    }

}

PlaylistItem *PlaylistItem::fromUrl(const QUrl &pathUrl, PlaylistItem *parent) {

    if (!pathUrl.isLocalFile ()) {
        PlaylistItem *playlist = new PlaylistItem("Pasted Link", nullptr, "", parent);
        if (parent) parent->append (playlist);
        auto pathString = pathUrl.toString ();
        playlist->emplaceBack (-1, pathString, pathString, true);
        playlist->currentIndex = 0;
        return playlist;
    }

    // QFileInfo path = QFileInfo(pathUrl.toLocalFile ());
    // if (!path.exists ()) {
    //     qDebug() << "Log (PlaylistItem): Path" << path << "doesn't exist";
    //     return nullptr;
    // }
    // QDir playlistDir = path.isDir () ? QDir(pathUrl.toLocalFile ()) : path.dir ();

    PlaylistItem *playlist = new PlaylistItem("", nullptr, "", parent);
    playlist->m_isLoadedFromFolder = true;
    playlist->m_children = std::make_unique<QList<PlaylistItem*>>();

    if (!playlist->loadFromFolder (pathUrl)) {
        delete playlist;
        return nullptr;
    }

    return playlist;
}

void PlaylistItem::emplaceBack(float number, const QString &link, const QString &name, bool isLocal) {
    if (!m_children) m_children = std::make_unique<QList<PlaylistItem*>>();
    m_children->emplaceBack(new PlaylistItem(number, link, name, this, isLocal));
    m_children->last ()->useCount++;
}

QUrl PlaylistItem::loadLocalSource(int index) {
    if (!isValidIndex (index)) return QUrl();

    auto playlistItem = m_children->at(currentIndex);
    if (playlistItem->type != LOCAL) return QUrl();

    if (index != currentIndex)
        updateHistoryFile (0);

    currentIndex = index;
    return QUrl::fromLocalFile(playlistItem->link);
}

void PlaylistItem::clear() {
    if (m_children) {
        for (auto &playlist : *m_children) {
            if (--playlist->useCount == 0)
                delete playlist;
        }
    }
}

QString PlaylistItem::getDisplayNameAt(int index) const {
    if (!isValidIndex(index))
        return "";
    auto currentItem = m_children->at(index);
    QString itemName = "%1\n[%2/%3] %4";
    itemName = itemName.arg(name)
                   .arg(index + 1)
                   .arg(m_children->count())
                   .arg(currentItem->fullName);
    return itemName;
}

bool PlaylistItem::isValidIndex(int index) const {
    if (!m_children || m_children->isEmpty ()) return false;
    return index >= 0 && index < m_children->size();
}
