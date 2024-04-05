#pragma once
#include <QString>
#include <QObject>
class ErrorHandler : public QObject
{
    Q_OBJECT
public:
    void show(const QString& errorMessage){
        emit showWarning (errorMessage);
    };
    static ErrorHandler& instance(){
        static ErrorHandler handler;
        return handler;
    }
signals:
    void showWarning(QString message);

private:
    ErrorHandler() = default;
    ErrorHandler(const ErrorHandler&) = delete; // Disable copy constructor.
    ErrorHandler& operator=(const ErrorHandler&) = delete; // Disable copy assignment.
    ~ErrorHandler(){} // Private destructor to prevent external deletion.


};
