#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <server.h>
#include <client.h>

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
    void on_startServerButton_clicked();
    void on_stopServerButton_clicked();

    void on_requestButton_clicked();

    void on_cancelButton_clicked();

    void on_clearServerMsgButton_clicked();

    void on_clearClientMsgButton_clicked();

    void on_serverCheckBox_stateChanged(int arg1);

protected:
    void timerEvent(QTimerEvent*);

private:
    void update_server_msglist();
    void update_client_msglist();
    Ui::MainWindow *ui;
    int timerId;
    size_t server_msg_size;
    server *s;
    client *c;
};

#endif // MAINWINDOW_H
