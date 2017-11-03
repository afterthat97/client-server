#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    timerId = startTimer(50);
    s = new server(6666, 100, SINGLE);
    server_msg_size = 0;
    c = new client("127.0.0.1", 6666, 8);
}

MainWindow::~MainWindow() {
    killTimer(timerId);
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent*) {
    update_server_msglist();
    update_client_msglist();
}

void MainWindow::on_startServerButton_clicked() {
    s->stop();
    s->setPort(ui->serverPortLineEdit->text().toInt());
    s->setQueue(ui->serverQueueSpinBox->text().toInt());
    if (ui->serverCheckBox->checkState() == Qt::Checked) {
        if (ui->serverComboBox->currentText() == "threads")
            s->setType(MT);
        else
            s->setType(MP);
    } else
        s->setType(SINGLE);
    s->start();
}

void MainWindow::on_stopServerButton_clicked() {
    s->stop();
}

void MainWindow::update_server_msglist() {
    if (server_msg.size() == server_msg_size) return;
    server_msg_size = server_msg.size();
    QStringListModel *model = new QStringListModel(this);
    QStringList list;
    for (size_t i = 0; i < server_msg.size(); i++)
        list << server_msg[i].c_str();
    model->setStringList(list);
    ui->serverListView->setModel(model);
    ui->serverListView->scrollToBottom();
}

void MainWindow::update_client_msglist() {
    if (new_message == 0) return; else new_message = 0;
    QStringListModel *model = new QStringListModel(this);
    QStringList list;
    for (size_t i = 0; i < client_msg.size(); i++)
        list << client_msg[i].c_str();
    model->setStringList(list);
    ui->clientListView->setModel(model);
}

void MainWindow::on_requestButton_clicked() {
    c->cancel();
    c->setHost(ui->clientHostLineEdit->text().toStdString());
    c->setPort(ui->clientPortLineEdit->text().toInt());
    c->setNum(ui->clientNumSpinBox->text().toInt());
    c->setX(ui->clientXSpinBox->text().toInt());
    if (c->start() == 0) {
        QMessageBox msgbox;
        msgbox.setText(("cannot resolve " +
            ui->clientHostLineEdit->text().toStdString() + ": Unknown host").c_str());
        msgbox.exec();
    }
}

void MainWindow::on_cancelButton_clicked() {
    c->cancel();
}

void MainWindow::on_clearServerMsgButton_clicked() {
    server_msg.clear();
}

void MainWindow::on_clearClientMsgButton_clicked() {
    client_msg.clear();
    new_message = 1;
}

void MainWindow::on_serverCheckBox_stateChanged(int arg1) {
    ui->serverComboBox->setEnabled(arg1 == Qt::Checked);
}
