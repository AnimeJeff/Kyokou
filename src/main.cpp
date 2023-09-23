#include <QQuickStyle>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QGuiApplication>
#include <QQmlContext>
#include <QFont>
#include <iostream>
#include <QTextStream>
#include <QImageReader>
#include <QNetworkProxyFactory>
#include <QtGlobal>

#include "mpv/mpvObject.h"
#include "application.h"
#include "showmanager.h"
#include "tools/cursorposprovider.hpp"
#include <QtPlugin>
#include "tools/ErrorHandler.h"
//qputenv("QT_DEBUG_PLUGINS", QByteArray("1"));
int parseArgs(int argc, char *argv[]);
void setOneInstance();void testNetwork();

int main(int argc, char *argv[]){
//    setOneInstance ();

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);
    QGuiApplication app(argc, argv);
    const char* dllPath = (QCoreApplication::applicationDirPath() + "/dll").toStdString ().data ();
    const char* currentPath = getenv("PATH");
    if (currentPath != nullptr) {
        std::string newPath = std::string(dllPath) + ";" + currentPath;
        qputenv("PATH", newPath.c_str());
        qDebug() << getenv("PATH");
    } else {
        qputenv("PATH", dllPath);
    }
    QQmlApplicationEngine engine;

    // setting up the application
    app.setWindowIcon(QIcon(u":/resources/images/icon.png"_qs));
    app.setFont (QFont("Microsoft Yahei UI", 16));
    std::setlocale(LC_NUMERIC, "C");

    // setting up the environment
    qputenv("LC_NUMERIC", QByteArrayLiteral("C"));
//    qputenv("http_proxy", "http://127.0.0.1:1080");
//    qputenv("https_proxy", "http://127.0.0.1:1080");

    QQuickStyle::setStyle("Universal");



    curl_global_init(CURL_GLOBAL_ALL);
    qmlRegisterType<MpvObject>("MpvPlayer", 1, 0, "MpvObject");
    engine.rootContext ()->setContextProperty("showManager",&ShowManager::instance ());
    engine.rootContext ()->setContextProperty("app", &Application::instance ()); //remove singleton?
    engine.rootContext ()->setContextProperty("cursor",&CursorPosProvider::instance ());
    engine.rootContext ()->setContextProperty("errorHandler",&ErrorHandler::instance ());

    // Parsing the arguments
    Application::instance ().parseArgs(argc,argv);


    const QUrl url(QStringLiteral("qrc:qml/src/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}

void testNetwork(){
    qDebug() << QString::fromStdString (NetworkClient::get("https://httpbin.org/ip").body);
//    qDebug() << QString::fromStdString (c.get("https://www.baidu.com/").body);
//    return
    QNetworkProxyFactory::setUseSystemConfiguration( true );

    QNetworkAccessManager manager;
    // Perform a simple GET request to a test URL (you can replace it with any desired URL)
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl("https://httpbin.org/ip")));
    QEventLoop loop;
    // Connect the signals to know when the request is finished
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    // Start the event loop to wait for the request to finish
    loop.exec();
    // Check the response status
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "Proxy works! Response:";
        qDebug() << reply->readAll(); // Display the response if needed
    } else {
        qDebug() << "Proxy failed. Error:" << reply->errorString();
    }
    reply->deleteLater();
}



void setOneInstance(){
    QSharedMemory shared("62d60669-bb94-4a94-88bb-b964890a7e04");
    if( !shared.create( 512, QSharedMemory::ReadWrite) )
    {
        qWarning() << "Can't start more than one instance of the application.";
        exit(0);
    }
    else {
        qDebug() << "Application started successfully.";
    }
}


