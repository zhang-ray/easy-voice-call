#include "mainwindow.hpp"
#include "Logger.hpp"

#ifdef LINUX_CLIENT
#include <QtWidgets/QApplication>
#include <QtCore/QtGlobal>
#include <QtCore/QStandardPaths>
#else
#include <QApplication>
#include <QtGlobal>
#include <QStandardPaths>
#endif

#ifdef _WIN32
#include "resources/resource.h"
#endif


int main(int argc, char *argv[]) {

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication a(argc, argv);



    auto qStringFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(qStringFolder);
    if (!dir.exists()) {
        dir.mkdir(qStringFolder);
    }
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm* now = std::localtime(&t);
    TrickyBoostLog trickyBoostLog(qStringFolder.toStdString()
        .append("/")
        .append("EasyVoiceCall")
        .append(".")
        .append(std::to_string(now->tm_year + 1900))
        .append(".")
        .append(std::to_string(now->tm_mon + 1))
        .append(".")
        .append(std::to_string(now->tm_mday))
        .append(".")
        .append(std::to_string(now->tm_hour))
        .append(".")
        .append(std::to_string(now->tm_min))
        .append(".")
        .append(std::to_string(now->tm_sec))
        .append(".log")
    );



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
