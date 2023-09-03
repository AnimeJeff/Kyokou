
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

//#include "CSoup.h"
//#include <QThread>
//#include <QNetworkAccessManager>
//#include <QNetworkRequest>
//#include <QNetworkReply>
//#include <QEventLoop>
//#include <QNetworkReply>
//#include <nlohmann/json.hpp>
#include "networkclient.h"

//class NetworkClient: public QObject
//{
//    struct Response{
//        QString url;
//        QByteArray body;
//        int code;
//        QList<QNetworkReply::RawHeaderPair> headers;

//        QString redirectUrl;
//        //        std::map<std::string, std::string> headers;
////        std::map<std::string, std::string> cookies;
//        CSoup document(){
//            return CSoup(body.toStdString ());
//        }
//        nlohmann::json json(){
//            return nlohmann::json::parse (body);
//        }
//    };
//    QNetworkAccessManager* m_manager;
//public:
//    NetworkClient(QObject *parent = nullptr): QObject(parent){
//        m_manager = new QNetworkAccessManager(nullptr);
//    };
//    ~NetworkClient(){
//        delete m_manager;
//    }
//    Response get(const QString& url, const QMap<QString, QString>& params={},
//                 const QMap<QString, QString>& headers={})
//    {
//        m_manager->moveToThread(QThread::currentThread());
//        // Build the query parameters
//        QString queryParams;
//        if (!params.empty()) {
//            QStringList queryItems;
//            for (auto const& key : params.keys()) {
//                queryItems << key + "=" + params.value(key);
//            }
//            queryParams = "?" + queryItems.join("&");
//        }

//        // Set the URL
//        QUrl fullUrl = QUrl(url + queryParams);
//        QNetworkRequest request(fullUrl);

//        // Set the headers
//        for (auto const& key : headers.keys()) {
//            request.setRawHeader(key.toUtf8(), headers.value(key).toUtf8());
//        }

//        // Perform the GET request synchronously

//        QNetworkReply *reply = m_manager->get(request);
//        QEventLoop loop;
//        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
//        loop.exec();
//        Response response;
//        response.url = fullUrl.toString();
//        response.code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//        response.headers = reply->rawHeaderPairs();
//        response.body = reply->readAll();
//        // Clean up
//        reply->deleteLater();
//        return response;
//    }
//    Response post(const QString& url, const QMap<QString, QString>& headers={},
//                                  const QMap<QString, QString>& data={})
//    {

//        QNetworkRequest request = createRequest(url, headers);
//        QByteArray postData;
//        for (auto it = data.begin(); it != data.end(); ++it) {
//            postData += it.key().toUtf8() + "=" + it.value().toUtf8() + "&";
//        }
//        postData.chop(1); // remove the last "&"

//        QNetworkReply *reply = m_manager->post(request, postData);

//        QEventLoop loop;
//        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
//        loop.exec();

//        QByteArray readBuffer = reply->readAll();
//        Response response;
//        response.url = url;
//        response.body = readBuffer;
//        response.code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//        response.headers = reply->rawHeaderPairs();
//        reply->deleteLater();

//        return response;
//    }
//    QNetworkRequest createRequest(const QString& url, const QMap<QString, QString>& headers)
//    {
//        QNetworkRequest request;
//        request.setUrl(QUrl(url));

//        for (auto it = headers.begin(); it != headers.end(); ++it) {
//            request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
//        }

//        return request;
//    }

//};

#endif // NETWORKMANAGER_H
