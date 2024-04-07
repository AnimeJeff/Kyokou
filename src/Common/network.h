#ifndef CLIENT_H
#define CLIENT_H

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QUrlQuery>
#include "curl/curl.h"
#include "Common/CSoup.h"

class NetworkClient
{
private:
    static CURL* getCurl(){
        QMutexLocker locker(&mutex);
        CURL* curl = curls.back ();
        curls.pop_back ();
        return curl;
    }

    static void returnCurl(CURL* curl){
        if (!curl) return;
        QMutexLocker locker(&mutex);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, NULL);
        curl_easy_setopt(curl, CURLOPT_POST, 0L);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
        curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 0L);
        curls.push_back(curl);
    }

    enum RequestType{
        GET,
        POST,
        HEAD,
        PUT,
        DELETE_,
        CONNECT,
        OPTIONS
    };
public:
    static void init(int maxCurls = 5);

    static void cleanUp();

public:
    struct Response{
        long code;
        // QUrl url;
        QString redirectUrl;
        //        QMap<QString, QString> headers;
        QString headers;
        QString body;
        QMap<QString, QString> cookies;
        inline CSoup document(){
            return CSoup(body);
        }

        QJsonObject toJson(){
            QJsonParseError error;
            QJsonDocument jsonData = QJsonDocument::fromJson(body.toUtf8 (), &error);
            if (error.error != QJsonParseError::NoError) {
                qWarning() << "JSON parsing error:" << error.errorString();
                return QJsonObject{};
            }
            return jsonData.object ();
        }

        ~Response(){}
    };

    static bool isUrlValid(const QString& url);



    static Response get(const QString &url, const  QMap<QString, QString>& headers={}, const QMap<QString, QString>& params = {});

    static Response post(const QString &url, const QMap<QString, QString>& headers={}, const QMap<QString, QString>& data={});

    static Response request(int type, const QString &url, const QMap<QString, QString>& headersMap={}, const QString &data = "");

private:
    NetworkClient() = default;
    ~NetworkClient() = default;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata);

private:
    inline static QList<CURL*> curls;
    inline static QMutex mutex;
    inline static bool initialised = false;

};


#endif // CLIENT_H
