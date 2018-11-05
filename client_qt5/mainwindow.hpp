#pragma once


#include <QMainWindow>
#include <memory>
#include "evc/Worker.hpp"
#include <QLabel>
#include <QEvent>
#include <QTemporaryDir>
#include <QFile>


namespace Ui {
class MainWindow;
}


class AudioVolumeEvent : public QEvent, public AudioIoVolume {
public:
    // char test[1<<20]; //for memory test
    static QEvent::Type sType;
    AudioVolumeEvent(const AudioIoVolume aiv)
        : QEvent(AudioVolumeEvent::sType)
        , AudioIoVolume(aiv)
    {
    }
};


class VadEvent : public QEvent {
private:
public:
    bool isActive_;
    static QEvent::Type sType;
    VadEvent(const bool isActive):isActive_(isActive), QEvent(sType){}
};


class NetworkStateEvent : public QEvent {
public:
    static QEvent::Type sType;
    NetworkState state_;
    NetworkStateEvent(const NetworkState state):state_(state), QEvent(sType){}
};

class ShowTextMessageEvent : public QEvent {
public:
    static QEvent::Type sType;
    std::string message_;
    ShowTextMessageEvent(const std::string &m):message_(m), QEvent(sType){}
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_connecting_clicked();
    void onNetworkChanged(const NetworkState networkState);
    void onVolumeChanged(const AudioIoVolume);
    void onDeviceNameChanged(const std::string &newMic, const std::string &newSpk){}
    void onVad(bool isActive);
    void toggleAdvancedMode(){toggleAdvancedMode(!advancedMode_);}
    void toggleAdvancedMode(bool newMode);
    void gotoWork();
    void on_toggleButton_micMute_clicked(bool checked);

private:
    Ui::MainWindow *ui = nullptr;

    std::shared_ptr<Worker> worker_ = nullptr;
private:

    NetworkState currentUiState_ = NetworkState::Disconnected;

private:
    void showMessage(const std::string &message);
    QPixmap vertical_bar_empty;
    QPixmap vertical_bar_full;
    QPixmap vertical_bar_half_full;
    QLabel* label_img_[2][AudioIoVolume::MAX_VOLUME_LEVEL];
    QString confFileName_ = "evc.config.txt";
    bool advancedMode_ = true;

    std::thread::id mainThreadId_ = std::this_thread::get_id();


    // QObject interface
public:
    virtual bool event(QEvent *event) override;
};
