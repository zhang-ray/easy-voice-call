#include "mainwindow.hpp"
#include <QApplication>
#include <QtGlobal>

#ifdef _WIN32
#include "resources/resource.h"
#endif


int main(int argc, char *argv[]) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication a(argc, argv);
    MainWindow w;

#ifdef _WIN32
    // set default font for Windows
    {
        QFont font;
        font.setFamily("Arial");
        a.setFont(font);
    }
#endif

    w.show();

    return a.exec();
}