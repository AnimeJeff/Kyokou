#include "Data/showdata.h"
#include "Data/playlistitem.h"
#include "Providers/showprovider.h"

void ShowData::addEpisode(float number, const QString &link, const QString &name) {
    if (!playlist) {
        playlist = new PlaylistItem(title, provider, this->link, nullptr);
    }
    playlist->emplaceBack (number, link, name, false);
}

QJsonObject ShowData::toJson() const {
    QJsonObject object;
    object["title"] = title;
    object["cover"] = coverUrl;
    object["link"] = link;
    object["provider"] = provider->name();
    object["lastWatchedIndex"] = playlist ? playlist->currentIndex : -1;
    object["type"] = type;
    return object;
}

QString ShowData::toString() const {
    QStringList stringList;
    stringList << "Title: " << title  << "\n"
               << "Link: " << link << "\n"
               << "Cover URL: " << coverUrl << "\n"
               << "Provider: " << provider->name() << "\n";
    if (!latestTxt.isEmpty ()) {
        stringList << "Latest Text: " << latestTxt << "\n";
    }
    if (!description.isEmpty ())
    {
        stringList << "Description: " << description << "\n";
    }
    if (!views.isEmpty ())
    {
        stringList << "Views: " << views << "\n";
    }
    if (!score.isEmpty ())
    {
        stringList << "Score: " << score << "\n";
    }
    if (!releaseDate.isEmpty ())
    {
        stringList << "Release Date: " << releaseDate << "\n";
    }
    if (!status.isEmpty ())
    {
        stringList << "Status: " << status << "\n";
    }
    if (!genres.isEmpty ())
    {
        stringList << "Genres: " << genres.join (',')<< "\n";
    }
    if (!updateTime.isEmpty ())
    {
        stringList << "Update Time: " << updateTime << "\n";
    }

    return stringList.join ("");
}
