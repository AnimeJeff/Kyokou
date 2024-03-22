#include "application.h"
#include <QString>
#include <QTextCodec>
#include "Providers/testprovider.h"
#include "Providers/kimcartoon.h"
#include "Providers/gogoanime.h"
#include "Providers/nivod.h"
#include "Providers/haitu.h"
#include "Providers/allanime.h"


Application::Application(QObject *parent): QObject(parent)
{
    NetworkClient::init();
}




