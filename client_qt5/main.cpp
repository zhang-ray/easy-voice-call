#include "mainwindow.hpp"
#include <QApplication>


#ifndef _WIN32
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

#else
#include "resources/resource.h"
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    // set default font for Windows
    {
        QFont font;
        font.setFamily("Arial");
        a.setFont(font);
    }

    return a.exec();
}
#endif
