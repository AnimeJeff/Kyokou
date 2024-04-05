#ifndef CLIENT_H
#define CLIENT_H

#include "curl/curl.h"
#include "CSoup.h"
#include "qjsonobject.h"
#include <QDebug>
#include <QJsonDocument>
#include <QMutex>
#include <QUrlQuery>


class NetworkClient
{
private:
    static constexpr int maxCurls = 10;

    static void createHandles()
    {
        for (int i = 0; i < maxCurls;)
        {
            CURL *curl = curl_easy_init ();
            if (curl)
            {
                initHandle(curl);
                curls.push_back(curl);
                i++;
            }
        }
    };

    static void initHandle(CURL* curl){
        // Set the timeouts
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

        // Set the user agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");

        // Set SSL options
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
    }

    static CURL* getCurl(){
        QMutexLocker locker(&mutex);
        CURL* curl = curls.back ();
        curls.pop_back ();
        return curl;
    }

    static void returnCurl(CURL* curl){
        QMutexLocker locker(&mutex);
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
    static void init(){
        if (initialised) return;
        curl_global_init(CURL_GLOBAL_ALL);
        createHandles();
        initialised = true;
    }

    static void cleanUp(){
        if (!initialised) return;
        for (auto& curl: curls){
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup ();
        curls.clear ();
        initialised = false;
    }

public:
    struct Response{
        long code;
        QUrl url;
        std::string redirectUrl;
        //        QMap<QString, QString> headers;
        std::string headers;
        std::string body;
        QMap<QString, QString> cookies;
        CSoup document(){
            return CSoup(body);
        }

        QJsonObject toJson(){
            QJsonParseError error;
            QJsonDocument jsonData = QJsonDocument::fromJson(body.c_str (), &error);
            if (error.error != QJsonParseError::NoError) {
                qWarning() << "JSON parsing error:" << error.errorString();
                return QJsonObject{};
            }
            return jsonData.object ();
        }

        ~Response(){
        }
        friend QDebug operator<<(QDebug debug, const Response& response)
        {
            debug << QString::fromStdString (response.body);
            return debug;
        }
    };

    static Response get(QUrl url, const  QMap<QString, QString>& headers={}, const QMap<QString, QString>& params = {})
    {
        if (!params.isEmpty ()) {
            QUrlQuery query;

            for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
                query.addQueryItem(it.key(), it.value());
            }

            url.setQuery(query);
        }

        return request(GET, url, headers);
    }

    static Response post(const QUrl& url, const QMap<QString, QString>& headers={}, const QMap<QString, QString>& data={}){
        // Set the post data
        QString postData;
        for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
            postData += it.key() + "=" + it.value() + "&";
        }
        return request(POST, url, headers, postData);
    }

    static Response request(int type, const QUrl& url, const QMap<QString, QString>& headersMap={}, const QString &data = ""){
        CURL* curl = getCurl();
        Response response;
        std::string urlData = url.toString().toStdString();
        curl_easy_setopt(curl, CURLOPT_URL, urlData.c_str ());

        struct curl_slist* headers = nullptr;

        for (auto it = headersMap.constBegin(); it != headersMap.constEnd(); ++it) {
            auto entry = QString(it.key() + ": " + it.value()).toStdString ();
            headers = curl_slist_append(headers, entry.c_str ());
        }


        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        std::string dataString = data.toStdString ();
        if (type == POST) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataString.c_str ());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
        } else if (type == GET) {
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        }
        // Set the response callback function
        std::string readBuffer;

        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);



        // Perform the GET request
        CURLcode res = curl_easy_perform(curl);

        // Get the response code
        long code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

        // Check for errors
        if (res != CURLE_OK){
            qDebug() << "curl_easy_perform() failed: " << curl_easy_strerror(res);
        }

        // Clean up
        curl_slist_free_all(headers);

        // Parse the response
        response.url = url;
        response.body = readBuffer;
        response.code = code;
        returnCurl (curl);
        return response;
    }

private:
    NetworkClient() = default;
    ~NetworkClient() = default;
    // WriteCallback function to handle the response body
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp){
        ((std::string*) userp)->append((char*) contents, size * nmemb);
        return size * nmemb;
    }

    // HeaderCallback function to handle the response headers
    static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata){
        size_t numbytes = size * nitems;
        std::string* header = static_cast<std::string*>(userdata);
        header->append(buffer, numbytes);
        return numbytes;
    }

private:
    inline static QList<CURL*> curls {};
    inline static QMutex mutex {};
    inline static bool initialised = false;

};


#endif // CLIENT_H
