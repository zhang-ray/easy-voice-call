#include "mainwindow.hpp"
#include <QApplication>
#include <QtGlobal>
#include <QStandardPaths>
#include "evc/Logger.hpp"
#include <cstdio>
#include <iostream>
#include <cstring>
#include <ctime>

#ifdef _WIN32
#include "resources/resource.h"
#endif


class TrickyBoostLog {
/*
HINT: 
  - USE BOOST_LOG_DYN_LINK
    - https://www.boost.org/doc/libs/1_67_0/libs/log/doc/html/log/detailed/sink_frontends.html say that:
        If asynchronous logging is used in a multi-module application, one should decide carefully when to unload dynamically loaded modules that write logs.
  - redirecting stdout instead of boost::log::add_file_log
    - when I use boost::log::add_file_log, the formatter is missing... 
    - relevant ticket: https://svn.boost.org/trac10/ticket/8840
*/
public:
    TrickyBoostLog(const std::string &filePath) {
        freopen(filePath.c_str(), "w", stdout);
    }
    ~TrickyBoostLog(){
        fclose(stdout);
    }
};


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
    std::cout << (now->tm_year + 1900) << '-'
        << (now->tm_mon + 1) << '-'
        << now->tm_mday
        << "\n";

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