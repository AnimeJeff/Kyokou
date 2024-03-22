#ifndef APPLICATION_H
#define APPLICATION_H

#include "Download/downloadmodel.h"

#include "Player/mpv/mpvObject.h"


#include "Components/cursor.h"
#include "Explorer/showmanager.h"
#include <QAbstractListModel>

class Application : public QObject {
    Q_OBJECT
    Q_PROPERTY(Cursor *cursor READ cursor CONSTANT)
    Cursor *cursor() { return &m_cursor; }
    Cursor m_cursor{}; // todo
public:
    static Application &instance() {
        static Application s_instance;
        return s_instance;
    }
    ShowManager showManager();
private:
    explicit Application(QObject *parent = nullptr);
    ~Application() { NetworkClient::shutdown(); }
    Application(const Application &) = delete; // Disable copy constructor.
    Application &
    operator=(const Application &) = delete; // Disable copy assignment.

private:
};

#endif // APPLICATION_H
