#include <QQuickStyle>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QGuiApplication>
#include <QQmlContext>
#include <QFont>
#include <iostream>
#include <QTextStream>
#include <QImageReader>

#include "mpv/mpvObject.h"
#include "application.h"
#include "showmanager.h"
#include "tools/cursorposprovider.hpp"
#include <QtPlugin>


bool parseArgs(int argc, char *argv[]);
//void print(const QString& str){
//    qDebug()<<str;
//}
//void print(const std::string& str){
//    print(QString::fromStdString (str));
//}


int main(int argc, char *argv[]){
    if(!parseArgs(argc,argv)) return 6;
//    qputenv("QT_DEBUG_PLUGINS", QByteArray("1"));
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(u":/resources/images/icon.png"_qs));
    app.setFont (QFont("Microsoft Yahei UI", 16));
    std::setlocale(LC_NUMERIC, "C");
    qputenv("LC_NUMERIC", QByteArrayLiteral("C"));
    qputenv("PYTHONIOENCODING", QByteArrayLiteral("utf-8"));
    qmlRegisterType<MpvObject>("MpvPlayer", 1, 0, "MpvObject");
    QQuickStyle::setStyle("Universal");

    QQmlApplicationEngine engine;
    engine.rootContext ()->setContextProperty("app",&Application::instance ());
    engine.rootContext ()->setContextProperty("showManager",&ShowManager::instance ());
//    engine.rootContext ()->setContextProperty("global",&Global::instance ());
    engine.rootContext ()->setContextProperty("cursor",&CursorPosProvider::instance ());

    const QUrl url(QStringLiteral("qrc:qml/src/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    qDebug() << "QCoreApplication::libraryPaths(): " << QCoreApplication::libraryPaths();
    qDebug() << QImageReader::supportedImageFormats();
    return app.exec();
}

bool cleanPath(std::string& pathStr,bool checkIsDir){
    auto path = std::filesystem::path(pathStr).make_preferred ();
    if(path.is_relative ()){
        path = (std::filesystem::current_path()/std::filesystem::path(path));
    }
    if (!std::filesystem::exists(path)) {
        printf("Invalid path \"%s\"",path.c_str ());
        return false;
    }
    pathStr = path.string();
    if(checkIsDir){
        if(!std::filesystem::is_directory(path)){
            std::cerr << "Not a directory" << std::endl;
            return false;
        }
    }else{
        std::set<std::string> validExtensions{".mp4", ".mkv", ".avi", ".mp3", ".flac", ".wav", ".ogg", ".webm"};
        if (!validExtensions.count(path.extension().string())){
            printf("File %s does not have a valid extension",path.c_str ());
            return false;
        }
    }

    return true;
}

bool parseArgs(int argc, char *argv[]){
    if(argc < 2)return true;
    CLI::App app{"app"};
    std::string dir, play;
    app.add_option("-d,--dir", dir, "Directory to use");
    app.add_option("-p,--play", play, "File to play");
    app.allow_extras(false);
    app.parse (argc, argv);

    if (!dir.empty() && !play.empty()) {
        std::cerr << "Please specify only one of --dir or --play" << std::endl;
        return false;
    }else if(!play.empty()){
        if(!cleanPath (play,false))return false;
        Application::instance ().playlistModel ()->setOnLaunchFile (play.c_str ());
    }else if(!dir.empty()){
        if(!cleanPath (dir,true))return false;
        Application::instance ().playlistModel ()->setOnLaunchPlaylist (dir.c_str ());
    }
    return true;
}







