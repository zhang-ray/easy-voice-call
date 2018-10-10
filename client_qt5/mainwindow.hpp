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
    void on_pushButton_clicked(bool checked);


private:
    Ui::MainWindow *ui;
    std::thread *bgThread_ = nullptr;

    AudioDecoder* decoder = nullptr;
    AudioEncoder* encoder = nullptr;
    AudioDevice*  device = nullptr;


private:
    void initEndpointAndCodec(){

    }
};

