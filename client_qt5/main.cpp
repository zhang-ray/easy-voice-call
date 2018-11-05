#include "mainwindow.hpp"
#include <QApplication>
#include <QtGlobal>
#include "evc/Logger.hpp"

#ifdef _WIN32
#include "resources/resource.h"
#endif


auto initLogger() {
    namespace logging = boost::log;
    namespace expr = boost::log::expressions;

    /// TODO
    ///   boost::log::keywords::format= "[%TimeStamp%]: %Message%" ;// DOSE NOT WORK!
    auto myFileSink = boost::log::add_file_log(
        boost::log::keywords::file_name = "EasyVoiceCall.%Y.%m.%d.%H.%M.%S.log"
        );
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);

    return myFileSink;
}




int main(int argc, char *argv[]) {
    auto myFileSink = initLogger();


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

    //// TODO: how to guarantee flush successfully in any case?
    myFileSink->flush();
    return a.exec();
}