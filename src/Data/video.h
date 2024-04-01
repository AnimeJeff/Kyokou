#pragma once

#include <QString>
#include <QStringList>
#include <QUrl>

struct Video {
    Video(QUrl videoUrl) : videoUrl(videoUrl) {}
    QString resolution = "N/A";
    QUrl videoUrl;
    QList<QUrl> subtitles;
    QUrl audioUrl;
    std::unordered_map<std::string, std::string> headers;
};

