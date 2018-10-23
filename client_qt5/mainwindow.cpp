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
#include "Worker.hpp"


/// TODO:
/// - display VAD result in real-time


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
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



    //        init UI
    {
        // -> center of screen
        setGeometry(QStyle::alignedRect(
                        Qt::LeftToRight,
                        Qt::AlignCenter,
                        size(),
                        qApp->desktop()->availableGeometry()
                        )
                    );
        setFixedSize(width(), height());
        updateUiState(NetworkState::Disconnected);

        connect(ui->lineEdit_serverHost, SIGNAL(returnPressed()), ui->pushButton_connecting, SLOT(click()));
    }


    try {
        if (!worker_.initCodec()){
            throw "worker_.initCodec failed";
        }
        if (!worker_.initDevice(
                    [this](const std::string &micInfo,
                           const std::string &spkInfo){
                            ui->label_micInfo->setText(micInfo.c_str());
                            ui->label_spkInfo->setText(spkInfo.c_str());
                           },[this](const uint8_t newVolume){
                                ui->label_volumeMic->setText(std::to_string(newVolume).c_str());
                            }, [this](const uint8_t newVolume){
                                ui->label_volumeSpk->setText(std::to_string(newVolume).c_str());
                            }, [this] (const bool isActive){
                                    if (isActive) { qDebug() << "isActive" ;} else {qDebug() << "";}
                            }
            )){
                throw "worker_.initDevice failed";
        }
    }
    catch (std::exception &e){
        qDebug() << "Exception: " << e.what() << "\n";
    }
}

MainWindow::~MainWindow()
{
    try {
        worker_.syncStop();


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
        {
            updateUiState(NetworkState::Connecting);
            worker_.asyncStart(
                        ui->lineEdit_serverHost->text().toStdString(),
                        [this](
                        const NetworkState newState,
                        const std::string extraMessage
                        )
            {
                showMessage(extraMessage);
                updateUiState(newState);
            }
            );
        }
        break;
    }
    case NetworkState::Connected:{
        {
            worker_.syncStop();
            updateUiState(NetworkState::Disconnected);
        }
        break;
    }
    }
}

void MainWindow::updateUiState(const NetworkState networkState){
    switch(networkState){
    case NetworkState::Disconnected:{
        ui->lineEdit_serverHost->setEnabled(true);

        ui->pushButton_connecting->setEnabled(true);
        ui->pushButton_connecting->setText("Connect!");
        //showMessage("");
        break;
    }
    case NetworkState::Connecting:{
        ui->lineEdit_serverHost->setEnabled(false);
        ui->pushButton_connecting->setEnabled(false);
        ui->pushButton_connecting->setText("Connecting...");
        break;
    }
    case NetworkState::Connected:{
        ui->lineEdit_serverHost->setEnabled(false);

        ui->pushButton_connecting->setEnabled(true);
        ui->pushButton_connecting->setText("Disconnect!");
        //        showMessage("Connected!");
        break;
    }
    default:
        break;
    }
    currentUiState_=networkState;
}

void MainWindow::showMessage(const std::string &message){
    try {
        if (message.empty()){
            ui->statusBar->clearMessage();
        }
        else {
            ui->statusBar->showMessage(message.c_str()); // TODO: thread-unsafety?
            qDebug() << message.c_str();
        }
    }
    catch (std::exception &e){
        qDebug() << e.what();
    }
}
