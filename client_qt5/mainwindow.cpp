#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <fstream>
#include "evc/Factory.hpp"
#include "evc/TcpClient.hpp"
#include "evc/Log.hpp"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked(bool checked)
{
    static std::shared_ptr<std::thread> s_bgThread = nullptr;

    s_bgThread = std::make_shared<std::thread>([this](){
        try{
            auto &decoder = Factory::get().createAudioDecoder();
            auto &encoder = Factory::get().createAudioEncoder();
            auto &device = Factory::get().create();



            if (encoder.reInit()){
                if (device.init()){
                    if (decoder.reInit()) {
                        auto host = (QLineEdit*)(ui->server_host);
                        auto port = (QLineEdit*)(ui->server_port);
                        TcpClient client(
                                    host->text().toStdString().c_str(),
                                    port->text().toStdString().c_str(),
                                    [&](const char* pData, std::size_t length){
                            // on Received Data
                            std::vector<char> netBuff;
                            netBuff.resize(length);
                            memcpy(netBuff.data(), pData, length);
                            std::vector<short> decodedPcm;
                            decoder.decode(netBuff, decodedPcm);
                            auto ret = device.write(decodedPcm);
                            if (!ret) {
                                std::cout << ret.message() << std::endl;
                            }
                        });

                        // sending work
                        for (;;){
                            const auto blockSize = 1920;
                            std::vector<short> micBuffer(blockSize);
                            // TODO: use RingBuffer?
                            auto ret = device.read(micBuffer);
                            if (!ret){
                                break;
                            }
                            std::vector<char> outData;
                            auto retEncode = encoder.encode(micBuffer, outData);
                            if (!retEncode){
                                std::cout << retEncode.message() << std::endl;
                                break;
                            }
                            client.send(outData);
                        }
                    }
                }
            }

        }
        catch (std::exception& e){
            std::cerr << "Exception: " << e.what() << "\n";
        }
    });

}
