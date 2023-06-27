
#ifndef VIDEOSERVER_H
#define VIDEOSERVER_H

#include <QString>
#include <QMap>
struct VideoServer{
    QString name;
    std::string link;
    QMap<QString,QString> headers;
    QString source;
    struct SkipData{
        unsigned int introBegin;
        unsigned int introEnd;
        unsigned int outroBegin;
        unsigned int outroEnd;
    };
    std::optional<SkipData> skipData;
};
#endif // VIDEOSERVER_H
