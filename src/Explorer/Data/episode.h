//#ifndef EPISODE_H
//#define EPISODE_H
//#include <QDebug>
//#include <QObject>
//#include <QString>

//#include <QVariant>
//#include <QMap>
//#include <QVector>
//#include <iostream>



//class PlaylistItem;
//struct Episode
//{
//    Q_GADGET
//    Q_PROPERTY(QString title READ getTitle);
//    Q_PROPERTY(QString fullTitle READ getFullTitle);
//    Q_PROPERTY(int number READ getNumber);
//    QString getTitle() const {return title;}
//    int getNumber() const {return number;}
//public:
//    Episode(int number,std::string link,QString title = "",QString fullName = "")
//        :number(number), title(title), link(link), fullName(fullName)
//    {
//        if (this->fullName.isEmpty ())
//        {
//            // the ordering of the episodes in vector will be according to the provider
//            if(number > 0)
//            {  // do not show negative episode number - episode is a movie quality;
//                this->fullName = QString::number(number);
//                if (!title.isEmpty ())
//                {
//                    this->fullName += QString(". ");
//                }
//            }
//            this->fullName += title;
//            if(this->fullName.isEmpty ())
//            {
//                this->fullName = "Unamed Episode";
//            };
//        }else{
//            this->fullName = fullName;
//        }
//    };
//    QString title;
//    int number = -1;
//    std::string link;

////    std::string dubLink;
////    QString thumbnail = "";
////    QString description;
////    bool isFiller = false;
////    bool hasDub = false;

//    QString getFullTitle() const {
//        return fullName;
//    }
//    PlaylistItem* parent = nullptr;
//    QVector<VideoServer> servers;
//private:
//    QString fullName;
//};
//#endif // EPISODE_H
