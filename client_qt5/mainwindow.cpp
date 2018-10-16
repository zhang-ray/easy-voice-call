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

#include "evc/Factory.hpp"
#include "evc/TcpClient.hpp"
#include "evc/Log.hpp"


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
        onDisconnected();
    }


    try{
        decoder = &(Factory::get().createAudioDecoder());
        encoder = &(Factory::get().createAudioEncoder());
        device = &(Factory::get().create());


        if (ui->lineEdit_userName->text().isEmpty()){
            // won't work well on Ubuntu 14.04, Qt 5.2.1
            // error: ‘prettyProductName’ is not a member of ‘QSysInfo’ 
            // ui->lineEdit_userName->setText(QSysInfo::prettyProductName());
        }

        if(! initEndpointAndCodec()){
            std::cerr << "initEndpointAndCodec return false;" << std::endl;
        }
    }
    catch (std::exception &e){
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

MainWindow::~MainWindow()
{
    fromUi_gotoDisconnect();


    // save setting
    try {
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
    if (ui_connected_){
        fromUi_gotoDisconnect();
    }
    else {
        fromUi_gotoConnect();
    }
}

void MainWindow::fromUi_gotoConnect()
{
    if (bgThread_ == nullptr){
        bgThread_ = new std::thread([this](){
            gotoStop_=false;
            onWorking();
        });
    }
    onConnected();

}

void MainWindow::fromUi_gotoDisconnect()
{
    gotoStop_=true;
    if (bgThread_){
        bgThread_->join();
        bgThread_=nullptr;
    }
    onDisconnected();
}

bool MainWindow::initEndpointAndCodec(){
    bool ok = false;
    if (encoder->reInit()){
        if (device->init()){
            if (decoder->reInit()) {
                return true;
            }
        }
    }
    return false;
}

void MainWindow::onConnected(){
    ui_connected_ = true;

    ui->lineEdit_userName->setEnabled(false);
    ui->lineEdit_serverHost->setEnabled(false);
    ui->lineEdit_serverPort->setEnabled(false);

    ui->pushButton_connecting->setText("Disconnect!");
}

void MainWindow::onDisconnected(){
    ui_connected_ = false;


    ui->lineEdit_userName->setEnabled(true);
    ui->lineEdit_serverHost->setEnabled(true);
    ui->lineEdit_serverPort->setEnabled(true);

    ui->pushButton_connecting->setText("Connect!");
}

void MainWindow::onWorking() {
    try{
        auto host = (QLineEdit*)(ui->lineEdit_serverHost);
        auto port = (QLineEdit*)(ui->lineEdit_serverPort);
        TcpClient client(
                    host->text().toStdString().c_str(),
                    port->text().toStdString().c_str(),
                    [&](const NetPacket& netPacket){
            // on Received Data
            if (netPacket.payloadType()==NetPacket::PayloadType::HeartBeatRequest){
                // MSVC could not capture `client` reference...
                // client.send(NetPacket(NetPacket::PayloadType::HeartBeatResponse));
            }
            else if (netPacket.payloadType()==NetPacket::PayloadType::HeartBeatResponse){
                //throw;
            }
            else if (netPacket.payloadType()==NetPacket::PayloadType::AudioMessage){
                std::vector<char> netBuff;
                netBuff.resize(netPacket.payloadLength());
                memcpy(netBuff.data(), netPacket.payload(), netPacket.payloadLength());
                std::vector<short> decodedPcm;
                decoder->decode(netBuff, decodedPcm);
                auto ret = device->write(decodedPcm);
                if (!ret) {
                    std::cout << ret.message() << std::endl;
                }
            }
        });


        // sending work
        for (;;){
            if (gotoStop_){
                break;
            }


            const auto blockSize = 1920;
            std::vector<short> micBuffer(blockSize);
            // TODO: use RingBuffer?
            auto ret = device->read(micBuffer);
            if (!ret){
                break;
            }
            std::vector<char> outData;
            auto retEncode = encoder->encode(micBuffer, outData);
            if (!retEncode){
                std::cout << retEncode.message() << std::endl;
                break;
            }

            client.send(NetPacket(NetPacket::PayloadType::AudioMessage, outData));



            // send heartbeat
            {
                static auto lastTimeStamp = std::chrono::system_clock::now();
                auto now = std::chrono::system_clock::now();
                auto elapsed = now - lastTimeStamp;
                if (elapsed > std::chrono::seconds(10)){
                    client.send(NetPacket(NetPacket::PayloadType::HeartBeatRequest));
                    lastTimeStamp = std::chrono::system_clock::now();
                }
            }
        }
    }
    catch (std::exception& e){
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return ;
}
