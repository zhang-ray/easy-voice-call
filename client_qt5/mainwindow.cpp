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

            lineLength = file.readLine(buf, sizeof(buf));
            if (lineLength != -1) {
                // the line is available in buf
                if (buf[lineLength-1]=='\n'){
                    buf[lineLength-1]=0;
                }
                ui->lineEdit_serverPort->setText(buf);
            }

            lineLength = file.readLine(buf, sizeof(buf));
            if (lineLength != -1) {
                // the line is available in buf
                if (buf[lineLength-1]=='\n'){
                    buf[lineLength-1]=0;
                }
                ui->lineEdit_userName->setText(buf);
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
    }


    try{
        if (ui->lineEdit_userName->text().isEmpty()){
            // won't work well on Ubuntu 14.04, Qt 5.2.1
            // error: ‘prettyProductName’ is not a member of ‘QSysInfo’
            // ui->lineEdit_userName->setText(QSysInfo::prettyProductName());
        }
    }
    catch (std::exception &e){
        std::cerr << "Exception: " << e.what() << "\n";
    }



    try {
        if (!worker_.initCodec()){
            throw "worker_.initCodec failed";
        }
        if (!worker_.initDevice(
                    [this](const std::string &micInfo,
                                       const std::string &spkInfo){
                                ui->label_micTitle->setVisible(!micInfo.empty());
                                ui->label_spkTitle->setVisible(!spkInfo.empty());
                                ui->label_micInfo->setText(micInfo.c_str());
                                ui->label_spkInfo->setText(spkInfo.c_str());
    },[this](const double newMicVolume){
                    qDebug() << newMicVolume;
//                    ui->progressBar_volumeMic->setValue(newMicVolume*100);

    }, [this](const double newSpkVolume){
                    qDebug() << newSpkVolume;
//                    ui->progressBar_volumeSpk->setValue(newSpkVolume*100);
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
            file.write(ui->lineEdit_serverPort->text().toStdString().c_str());
            file.write("\n");
            file.write(ui->lineEdit_userName->text().toStdString().c_str());
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
            worker_.asyncStart(
                        ui->lineEdit_serverHost->text().toStdString(),
                        ui->lineEdit_serverPort->text().toStdString(),
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
        ui->lineEdit_userName->setEnabled(true);
        ui->lineEdit_serverHost->setEnabled(true);
        ui->lineEdit_serverPort->setEnabled(true);

        ui->pushButton_connecting->setEnabled(true);
        ui->pushButton_connecting->setText("Connect!");
        //showMessage("");
        break;
    }
    case NetworkState::Connecting:{
        ui->lineEdit_userName->setEnabled(false);
        ui->lineEdit_serverHost->setEnabled(false);
        ui->lineEdit_serverPort->setEnabled(false);
        ui->pushButton_connecting->setEnabled(false);
        ui->pushButton_connecting->setText("Connecting...");
        break;
    }
    case NetworkState::Connected:{
        ui->lineEdit_userName->setEnabled(false);
        ui->lineEdit_serverHost->setEnabled(false);
        ui->lineEdit_serverPort->setEnabled(false);

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
            ui->statusBar->showMessage(message.c_str());
        }
    }
    catch (std::exception &e){
        qDebug() << e.what();
    }
}
