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

#include <QFontDatabase>
#include "Player/Mpv/mpvObject.h"
#include "application.h"
#include "Explorer/showmanager.h"
#include <QtPlugin>

//qputenv("QT_DEBUG_PLUGINS", QByteArray("1"));
int parseArgs(int argc, char *argv[]);
void setOneInstance();void testNetwork();

int main(int argc, char *argv[]){
    //setOneInstance ();

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);
    QGuiApplication app(argc, argv);
    if (!Application::instance ().parseArgs(argc,argv))
    {
        std::cout << "Failed to parse args";
        return 1;
    }
    auto dllPath = QCoreApplication::applicationDirPath().toStdString () + "/dll";

    QQmlApplicationEngine engine;

    app.setWindowIcon(QIcon(u":/resources/images/icon.png"_qs));
    qint32 fontId = QFontDatabase::addApplicationFont(":/resources/app-font.ttf");
    QStringList fontList = QFontDatabase::applicationFontFamilies(fontId);

    QString family = fontList.first();
    QGuiApplication::setFont(QFont(family, 16));

    //    app.setFont (QFont("Microsoft Yahei UI", 16));
    std::setlocale(LC_NUMERIC, "C");

    qputenv("LC_NUMERIC", QByteArrayLiteral("C"));
    QQuickStyle::setStyle("Universal");

    qmlRegisterType<MpvObject>("MpvPlayer", 1, 0, "MpvObject");
    engine.rootContext ()->setContextProperty("showManager",&ShowManager::instance ());
    engine.rootContext ()->setContextProperty("app", &Application::instance ()); //remove singleton?
    engine.rootContext ()->setContextProperty("errorHandler", &ErrorHandler::instance ());

    // Parsing the arguments

    const QUrl url(QStringLiteral("qrc:qml/src/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
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


