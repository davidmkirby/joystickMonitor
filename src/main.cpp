#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Set application information
    QApplication::setApplicationName("Joystick Mirror Controller");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("Your Organization");
    
    MainWindow w;
    w.show();
    
    return a.exec();
}