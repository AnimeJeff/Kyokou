#pragma once

#include <QString>
#include <QStringList>
#include <QUrl>
#include <string>
#include <sstream>

struct Video {
    Video(QUrl videoUrl) : videoUrl(videoUrl) {}
    QString resolution = "N/A";
    QUrl videoUrl;
    QList<QUrl> subtitles;
    QUrl audioUrl;
    void addHeader(const std::string &key, const std::string &value);

    std::string headers() const;

private:
    std::unordered_map<std::string, std::string> m_headers;


};

