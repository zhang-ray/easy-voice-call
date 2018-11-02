#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <fstream>
#include <functional>
#include <QDesktopWidget>
#include <QStyle>
#include <QDir>
#include <QDebug>
#include <memory>
#include <QKeyEvent>
#include <QDesktopServices>
#include "git_info.hpp"


QEvent::Type AudioVolumeEvent::sType = (QEvent::Type)QEvent::registerEventType();
QEvent::Type VadEvent::sType = (QEvent::Type)QEvent::registerEventType();
QEvent::Type NetworkStateEvent::sType = (QEvent::Type)QEvent::registerEventType();


MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::MainWindow)
    ,vertical_bar_full(":/vertical_bar_full.png")
    ,vertical_bar_empty(":/vertical_bar_empty.png")
    ,vertical_bar_half_full(":/vertical_bar_half_full.png")
{
    ui->setupUi(this);


    // load setting
    try {
        QFile file("setting.txt");
        if (file.open(QIODevice::ReadWrite)){
            char buf[1024];
            qint64 lineLength = -1;
            lineLength = file.readLine(buf, sizeof(buf));
            if (lineLength != -1) {
                // the line is available in buf
                if (buf[lineLength-1]=='\n'){
                    buf[lineLength-1]=0;
                }
                ui->lineEdit_serverHost->setText(buf);
            }
        }
    }
    catch (std::exception &e){
        std::cerr << "Exception: " << e.what() << "\n";
    }



    /// Init UI
    {
        /// -> center of screen, and fix window Size
        {
            setGeometry(QStyle::alignedRect(
                            Qt::LeftToRight,
                            Qt::AlignCenter,
                            size(),
                            qApp->desktop()->availableGeometry()
                            )
                        );
            setFixedSize(width(), height());

        }



        /// insert images to qlabels
        {
            ui->label_img_ain->setPixmap(QPixmap(":/microphone.png"));
            ui->label_img_ain->setScaledContents(true);

            ui->label_img_aout->setPixmap(QPixmap(":/speaker.png"));
            ui->label_img_aout->setScaledContents(true);
        }



        /// FIX ME.... ugly enough
        {
            {
                auto type = 0;
                auto index = 0;
                label_img_[type][index++]=ui->label_img_ain_bar0;
                label_img_[type][index++]=ui->label_img_ain_bar1;
                label_img_[type][index++]=ui->label_img_ain_bar2;
                label_img_[type][index++]=ui->label_img_ain_bar3;
                label_img_[type][index++]=ui->label_img_ain_bar4;
                label_img_[type][index++]=ui->label_img_ain_bar5;
                label_img_[type][index++]=ui->label_img_ain_bar6;
                label_img_[type][index++]=ui->label_img_ain_bar7;
                label_img_[type][index++]=ui->label_img_ain_bar8;
                label_img_[type][index++]=ui->label_img_ain_bar9;
            }

            {
                auto type = 1;
                auto index = 0;
                label_img_[type][index++]=ui->label_img_aout_bar0;
                label_img_[type][index++]=ui->label_img_aout_bar1;
                label_img_[type][index++]=ui->label_img_aout_bar2;
                label_img_[type][index++]=ui->label_img_aout_bar3;
                label_img_[type][index++]=ui->label_img_aout_bar4;
                label_img_[type][index++]=ui->label_img_aout_bar5;
                label_img_[type][index++]=ui->label_img_aout_bar6;
                label_img_[type][index++]=ui->label_img_aout_bar7;
                label_img_[type][index++]=ui->label_img_aout_bar8;
                label_img_[type][index++]=ui->label_img_aout_bar9;
            }
            for (int i = 0; i <AudioIoVolume::MAX_VOLUME_LEVEL; i++){
                label_img_[0][i]->setScaledContents(true);
                label_img_[1][i]->setScaledContents(true);
            }
        }



        /// tool tip
        {
            ui->lineEdit_serverHost->setToolTip("example: your.server.com\n or:      12.45.67.89");
        }


        {
            connect(ui->lineEdit_serverHost, SIGNAL(returnPressed()), ui->pushButton_connecting, SLOT(click()));
            connect(ui->lineEdit_serverPort, SIGNAL(returnPressed()), ui->pushButton_connecting, SLOT(click()));
        }



        /// init state
        {
            onVolumeChanged({AudioInOut::In, 0u, 0u});
            onVolumeChanged({AudioInOut::Out, 0u, 0u});
            onNetworkChanged(NetworkState::Disconnected);
            toggleAdvancedMode(true);
            showMessage("F1: help    F2: toggle mode (advanced/easy mode)");
        }


        {
            std::string title("EasyVoiceCall");
#ifdef GIT_TAG
            title.append(" ");
            title.append(GIT_TAG);
#endif //#ifdef GIT_TAG
            setWindowTitle(title.c_str());
        }
    }

}

MainWindow::~MainWindow()
{
    try {
        if (worker_){
            worker_->syncStop();
        }

        // save setting
        QFile file("setting.txt");
        if (file.open(QIODevice::ReadWrite)){
            file.write(ui->lineEdit_serverHost->text().toStdString().c_str());
            file.write("\n");
            file.close();
        }
    }
    catch (std::exception &e){
        std::cerr << "Exception: " << e.what() << "\n";
    }

    delete ui;
}

void MainWindow::on_pushButton_connecting_clicked(){
    switch (currentUiState_){
    case NetworkState::Disconnected: {
        gotoWork();
        break;
    }
    case NetworkState::Connected:{
        {
            if (worker_){
                worker_->syncStop();
                worker_=nullptr;
            }
            onNetworkChanged(NetworkState::Disconnected);
        }
        break;
    }
    }
}

void MainWindow::onNetworkChanged(const NetworkState networkState){
    if(mainThreadId_ == std::this_thread::get_id()){
        switch(networkState){
        case NetworkState::Disconnected:{
            ui->lineEdit_serverHost->setEnabled(true);
            ui->lineEdit_serverPort->setEnabled(true);
            ui->checkBox_needAec->setEnabled(true);
            ui->pushButton_connecting->setEnabled(true);
            ui->pushButton_connecting->setText("Connect!");
            ui->pushButton_connecting->setIcon(QPixmap(":/call_up.png"));
            //showMessage("");
            break;
        }
        case NetworkState::Connecting:{
            ui->lineEdit_serverHost->setEnabled(false);
            ui->lineEdit_serverPort->setEnabled(false);
            ui->checkBox_needAec->setEnabled(false);
            ui->pushButton_connecting->setEnabled(false);
            ui->pushButton_connecting->setText("Connecting...");
            break;
        }
        case NetworkState::Connected:{
            ui->lineEdit_serverHost->setEnabled(false);
            ui->lineEdit_serverPort->setEnabled(false);
            ui->checkBox_needAec->setEnabled(false);
            ui->pushButton_connecting->setEnabled(true);
            ui->pushButton_connecting->setIcon(QPixmap(":/hang_up.png"));
            ui->pushButton_connecting->setText("Disconnect!");
            break;
        }
        default:
            break;
        }
        currentUiState_=networkState;
    }
    else{
        QCoreApplication::postEvent(this, new NetworkStateEvent(networkState));
    }
}

void MainWindow::onVolumeChanged(const AudioIoVolume aivl) {
    if(mainThreadId_ == std::this_thread::get_id()){
        for (int i =0;i< AudioIoVolume::MAX_VOLUME_LEVEL;i++){
            label_img_[(uint8_t)aivl.io_][i]->setPixmap(i<aivl.level_? vertical_bar_half_full: vertical_bar_empty);
        }
        if (aivl.recentMaxLevel_> aivl.level_){
            label_img_[(uint8_t)aivl.io_][aivl.recentMaxLevel_]->setPixmap(vertical_bar_full);
        }
    }
    else{
        QCoreApplication::postEvent(this, new AudioVolumeEvent(aivl));
    }
}

void MainWindow::onVad(bool isActive){
    if(mainThreadId_ == std::this_thread::get_id()){
        ui->label_vad->setText(isActive? "active" : "inactive");
    }
    else{
        QCoreApplication::postEvent(this, new VadEvent(isActive));
    }
}

void MainWindow::toggleAdvancedMode(bool newMode)
{
    advancedMode_ = newMode;
    ui->groupBox_details->setVisible(advancedMode_);
    ui->lineEdit_serverPort->setVisible(advancedMode_);
    setFixedSize(advancedMode_?600:300,400);
}

void MainWindow::gotoWork(){
    try {
        onNetworkChanged(NetworkState::Connecting);


        worker_ = std::make_shared<Worker>(ui->checkBox_needAec->checkState()==Qt::CheckState::Checked);

        if (!worker_->initCodec()){
            throw "worker_.initCodec failed";
        }

        if (!worker_->initDevice(
                    std::bind(&MainWindow::onDeviceNameChanged, this, std::placeholders::_1,std::placeholders::_2),
                    std::bind(&MainWindow::onVolumeChanged, this, std::placeholders::_1),
                    std::bind(&MainWindow::onVad, this, std::placeholders::_1)
                    )){
            throw "worker_.initDevice failed";
        }


        ui->lineEdit_serverHost->setText(
                    ui->lineEdit_serverHost->text().trimmed()
                    );
        ui->lineEdit_serverPort->setText(
                    ui->lineEdit_serverPort->text().trimmed()
                    );
        worker_->asyncStart(
                    ui->lineEdit_serverHost->text().toStdString(),
                    ui->lineEdit_serverPort->text().toStdString(),
                    [this](
                    const NetworkState newState,
                    const std::string extraMessage
                    )
        {
            showMessage(extraMessage);
            onNetworkChanged(newState);
        }
        );

    }
    catch (std::exception &e){
        qDebug() << "Exception: " << e.what() << "\n";
    }

}


void MainWindow::showMessage(const std::string &message){
    try {
        if (message.empty()){
            ui->statusBar->clearMessage();
        }
        else {
            ui->statusBar->showMessage(message.c_str());
            qDebug() << message.c_str();
        }
    }
    catch (std::exception &e){
        qDebug() << e.what();
    }
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_F1){
            QDesktopServices::openUrl(QUrl("https://github.com/zhang-ray/easy-voice-call"));
        }
        else if (ke->key() == Qt::Key_F2){
            toggleAdvancedMode();
        }
        return true;
    } else if (event->type() == AudioVolumeEvent::sType) {
        AudioVolumeEvent *myEvent = static_cast<AudioVolumeEvent *>(event);
        onVolumeChanged({myEvent->io_, myEvent->level_, myEvent->recentMaxLevel_});
        return true;
    }
    else if (event->type() == VadEvent::sType){
        VadEvent *myEvent = static_cast<VadEvent *>(event);
        onVad(myEvent->isActive_);
        return true;
    }
    else if (event->type() == NetworkStateEvent::sType){
        NetworkStateEvent *myEvent = static_cast<NetworkStateEvent *>(event);
        onNetworkChanged(myEvent->state_);
        return true;
    }

    return QWidget::event(event);
}

