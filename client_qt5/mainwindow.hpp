#pragma once


#include <QMainWindow>
#include <memory>
#include <thread>


namespace Ui {
class MainWindow;
}



class AudioDecoder;
class AudioEncoder;
class AudioDevice;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_connecting_clicked();
private:
    void fromUi_gotoConnect();
    void fromUi_gotoDisconnect();
private:
    Ui::MainWindow *ui = nullptr;
    std::thread *bgThread_ = nullptr;

    AudioDecoder* decoder = nullptr;
    AudioEncoder* encoder = nullptr;
    AudioDevice*  device = nullptr;

    bool gotoStop_ = false;


private:
    bool ui_connected_ = false;
private:
    bool initEndpointAndCodec();

    void onConnecting(){

    }

    void onConnected();
    void onDisconnected();

    void onWorking();
};

