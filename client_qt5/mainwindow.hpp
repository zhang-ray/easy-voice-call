#pragma once


#include <QMainWindow>
#include <memory>
#include "Worker.hpp"



namespace Ui {
class MainWindow;
}





class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_connecting_clicked();
    void updateUiState(const NetworkState networkState);

private:
    Ui::MainWindow *ui = nullptr;

    Worker worker_;
private:

    NetworkState currentUiState_ = NetworkState::Disconnected;

private:
    void showMessage(const std::string &message);

};
