#ifndef EPISODE_H
#define EPISODE_H
#include <QDebug>
#include <QObject>
#include <QString>

#include <QVariant>
#include <QMap>
#include <QVector>
#include <iostream>

struct Episode
{
    Q_GADGET
    Q_PROPERTY(QString title READ getTitle);
    Q_PROPERTY(int number READ getNumber);
    QString getTitle() const {return title;}
    int getNumber() const {return number;}
public:
    Episode(int number,QString link,QString title = ""):number(number),title(title),link(link){
        fullTitle = "Episode " + QString::number(number);
        if(!title.isEmpty ()) fullTitle += QString(": %1").arg (title);
    };
    QString title;
    int number = -1;
    QString link;
    std::string dubLink;
    //    QString thumbnail = "";
    QString description;
    bool isFiller = false;
    bool hasDub = false;
    QString getFullTitle() const {
        return fullTitle;
    }
private:
    QString fullTitle;
};
#endif // EPISODE_H
