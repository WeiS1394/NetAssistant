#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myhelper.h"
#include <QUdpSocket>
#include <udp.h>
#include <QLabel>
#include <QDate>
#include "commonhelper.h"
#include <QSettings>
#include "tcp.h"
#include "app.h"
#include "udp.h"
#include <QTcpSocket>
#include <Qtcore/qtextcodec.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* 状态栏标签 */
    statusLabel = new QLabel;
    statusLabel->setMinimumSize(260, 20);  //设置标签最小的大小
    statusLabel->setFrameShape(QFrame::WinPanel);  //设置标签形状
    statusLabel->setFrameShadow(QFrame::Sunken);  //设置标签阴影
    ui->statusBar->addWidget(statusLabel);  //状态栏中添加状态标签
    statusLabel->setText(trUtf8("UDP通信停止"));
    statusLabel->setAlignment(Qt::AlignHCenter);

    /* 接收数量标签 */
    receiveLabel = new QLabel;
    receiveLabel->setMinimumSize(150, 20);
    receiveLabel->setFrameShape(QFrame::WinPanel);
    receiveLabel->setFrameShadow(QFrame::Sunken);
    ui->statusBar->addWidget(receiveLabel);
    receiveLabel->setAlignment(Qt::AlignHCenter);

    /* 发送数量标签 */
    sendLabel = new QLabel;
    sendLabel->setMinimumSize(150, 20);
    sendLabel->setFrameShape(QFrame::WinPanel);
    sendLabel->setFrameShadow(QFrame::Sunken);
    ui->statusBar->addWidget(sendLabel);
    sendLabel->setAlignment(Qt::AlignHCenter);
    updateStateBar(QString(), 0, 0);  //修改标签函数

    /* 计数器清零button */
    clearCounterButton = new QPushButton();
    ui->statusBar->addWidget(clearCounterButton);
    clearCounterButton->setText(trUtf8("计数器清零"));
    connect(clearCounterButton, SIGNAL(released()), this, SLOT(clearCounterPushBtn_released()));

    /* 时间标签：要进行更新 */
    timeLabel = new QLabel;
    timeLabel->setMinimumSize(90, 20);
    timeLabel->setMaximumWidth(90);
    timeLabel->setFrameShape(QFrame::WinPanel);
    timeLabel->setFrameShadow(QFrame::Sunken);
    ui->statusBar->addWidget(timeLabel);
    timeLabel->setText(QDate::currentDate().toString("yyyy-MM-dd"));

    /* 更新接收到的数据 */
    connect(&client, SIGNAL(valueChanged(qint8,QString)), this, SLOT(updateReceiveText(qint8,QString)));
    connect(&client, SIGNAL(updateState(QString,QVariant,QVariant)),
            this, SLOT(updateStateBar(QString,QVariant,QVariant)));

    uReceiveNum = uSendNum = 0;
    this->init();
}

/*
 * 初始化
 */
void MainWindow::init()
{
    this->setWindowTitle(trUtf8("网络调试助手 本机IP[%1]").arg(chelper.getLocalHostIP().toString()));  //窗口标题

    /*UDP 服务*/
    isConnect = false;
    ui->targetIPEdit->setText(chelper.getLocalHostIP().toString());   //初始化目标IP
    ui->targetPortsEdit->setText(tr("8080"));  //初始化目标端口号
    ui->localPortsEdit->setText(tr("8080"));  //初始化本地端口号
    ui->txtIntervalUdpServer->setText("1000");
    ui->ckAutoSendUdpServer->setEnabled(false);
    ui->targetIPEdit->setEnabled(true);
    ui->targetPortsEdit->setEnabled(true);
    ui->localPortsEdit->setEnabled(true);
    ui->btnSendUdpServer->setEnabled(false);
    updateStateBar(trUtf8("本地IP：%1 无连接").arg(chelper.getLocalHostIP().toString()),
                   QVariant(QVariant::Int), QVariant(QVariant::Int));

    timerUdpServer = new QTimer(this);
    timerUdpServer->setInterval(App::IntervalUdpServer);
    connect(timerUdpServer, SIGNAL(timeout()), this, SLOT(sendDataUdpServer()));

    /*TCP服务端*/
    msgMaxCount = 50;
    countTcpServer = 0;
    ui->txtTcpListenPort->setText("8080");
    ui->txtIntervalTcpServer->setText("1000");
    ui->btnSendTcpServer->setEnabled(false);
    ui->ckAutoSendTcpServer->setEnabled(false);

    tcpServer = new TcpServer(this);
    connect(tcpServer, SIGNAL(clientConnect(int, QString, int)), this, SLOT(clientConnect(int, QString, int)));
    connect(tcpServer, SIGNAL(clientDisConnect(int, QString, int)), this, SLOT(clientDisConnect(int, QString, int)));
    connect(tcpServer, SIGNAL(clientReadData(int,QString,int,QByteArray)), this, SLOT(clientReadData(int,QString,int,QByterArray)));

    timerTcpServer = new QTimer(this);
    timerTcpServer->setInterval(App::IntervalTcpServer);
    connect(timerTcpServer, SIGNAL(timeout()), this, SLOT(sendDataTcpServer()));  //定时发送
    connect(ui->btnSendTcpServer, SIGNAL(clicked()), this, SLOT(sendDataTcpServer()));  //发送

    /* TCP客户端 */
    countTcpClient = 0;
    ui->txtTcpServerIP->setText("192.168.10.71");
    ui->txtTcpServerPort->setText("8080");
    ui->txtIntervalTcpClient->setText("1000");
    ui->btnSendTcpClient->setEnabled(false);
    ui->ckAutoSendTcpClient->setEnabled(false);

    tcpClient = new QTcpSocket(this);
    tcpClient->abort();
    connect(tcpClient, SIGNAL(readyRead()), this, SLOT(tcpClientReadData()));
    connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpClientReadError()));

    timerTcpClient = new QTimer(this);
    timerTcpClient->setInterval(App::IntervalTcpClient);
    connect(timerTcpClient, SIGNAL(timeout()), this, SLOT(sendDatatTcpClient()));  //定时发送
    connect(ui->btnSendTcpClient, SIGNAL(clicked()), this, SLOT(sendDatatTcpClient()));  //发送
}

/*
 * 更新状态栏中的参数
 */
void MainWindow::updateStateBar(QString state, QVariant inNum, QVariant outNum)
{
    if (!state.isNull()) {
        statusLabel->setText(state);
    }

    if (!inNum.isNull()) {
        if (inNum.toInt() == 0) {
            uReceiveNum = 0;
        } else {
            uReceiveNum += inNum.toInt();
        }

        receiveLabel->setText(trUtf8("接收：") + QString::number(uReceiveNum));
    }

    if (!outNum.isNull()) {
        if (outNum.toInt() == 0) {
            uSendNum = 0;
        } else {
            uSendNum += outNum.toInt();
        }

        sendLabel->setText(trUtf8("发送：") + QString::number(uSendNum));
    }
}

/*
 * 将计数器置零
 */
void MainWindow::clearCounterPushBtn_released()
{
    qDebug("%s", __func__);
    updateStateBar(QString(), 0, 0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/* UDP服务 */
/*
 * 连接
 */
void MainWindow::connectNet()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    uTargetIP = ui->targetIPEdit->text();  //获取IP
    uTargetPort = ui->targetPortsEdit->text().toInt();
    uLocalPort = ui->localPortsEdit->text().toInt();

    updateStateBar(trUtf8("UDP通信 ") + uTargetIP + ":" + QString().number(uTargetPort),
                   QVariant(QVariant::Int), QVariant(QVariant::Int));    //状态栏内容

    isConnect = true;
    ui->connectPushBtn->setText(trUtf8("断开网络"));
    ui->targetIPEdit->setEnabled(false);
    ui->targetPortsEdit->setEnabled(false);
    ui->localPortsEdit->setEnabled(false);
    ui->btnSendUdpServer->setEnabled(true);
    ui->ckAutoSendUdpServer->setEnabled(true);

    client.udpStart(chelper.getLocalHostIP(), uLocalPort, QHostAddress(uTargetIP), uTargetPort);   //启动UDP服务
}

/*
 * 当网络断开后的一些状态
 */
void MainWindow::disConnectNet()
{
    isConnect = false;
    ui->connectPushBtn->setText(trUtf8("连接"));
    ui->targetIPEdit->setEnabled(true);
    ui->targetPortsEdit->setEnabled(true);
    ui->localPortsEdit->setEnabled(true);
    ui->btnSendUdpServer->setEnabled(false);
    ui->ckAutoSendUdpServer->setEnabled(false);

    client.udpStop(NULL, NULL, 0);   //停止UDP服务

    updateStateBar(trUtf8("UDP通信停止 "), QVariant(QVariant::Int), QVariant(QVariant::Int));
}

/*
 * 串口打印数据
 */
void MainWindow::updateReceiveText(qint8 type, QString msg)
{
    QString str;
    if (type == 0) {
        str = trUtf8(">>发送：");
        ui->browDataUdpServer->setTextColor(QColor("dodgerblue"));
    }
    else if (type == 1) {
        str =trUtf8("<<接收：");
        ui->browDataUdpServer->setTextColor(QColor("red"));
    }

    if (App::NewLineUdpServer) {
        ui->browDataUdpServer->append("\n");
    }

    if (!App::StopAcceptUdpServer) {
        ui->browDataUdpServer->append(trUtf8("时间[%1] %2 %3").arg(TIMEMS).arg(str).arg(msg));
    }
}

/*
 * 清除接收区内容
 */
void MainWindow::on_clearReceivePushBtn_released()
{
    ui->browDataUdpServer->clear();
}

/*
 * 清除发送区内容
 */
void MainWindow::on_clearSendPushBtn_released()
{
    ui->txtSendUdpServer->clear();
}

/*
 * 连接按钮
 */
void MainWindow::on_connectPushBtn_released()
{
    if (isConnect) {
        disConnectNet();
    }
    else {
        connectNet();
    }
}

/*
 * 发送数据 按钮
 */
void MainWindow::on_btnSendUdpServer_released()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    QByteArray buf = ui->txtSendUdpServer->document()->toPlainText().toAscii();
    QString string = myHelper::byteArraytoStr(buf);

    if (string.length() != 0) {
        client.sendData(string, uTargetIP, uTargetPort);

        if (App::AutoClearUdpServer) {
            ui->txtSendUdpServer->clear();
        }
    }
}

/*
 * 发送数据
 */
void MainWindow::sendDataUdpServer()
{
     QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
     QString string = ui->txtSendUdpServer->toPlainText();
     client.sendData(string, uTargetIP, uTargetPort);
}

/*
 * UDP中，十六进制接收
 */
void MainWindow::on_ckHexReceiveUdpServer_toggled(bool checked)
{
    App::HexReceiveUdpServer = checked;  //十六进制接收
}

/*
 * UDP中，十六进制发送
 */
void MainWindow::on_ckHexSendUdpServer_toggled(bool checked)
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    QByteArray buf = ui->txtSendUdpServer->document()->toPlainText().toAscii();
    QString str = myHelper::byteArraytoStr(buf);

    if (!str.isEmpty()) {
        if (checked) {
            QString strtoHex = str.toAscii().toHex();
            QString strDisplay;
            strtoHex = strtoHex.toUpper();

            for (int i = 0; i < strtoHex.length(); i += 2) {
                QString st = strtoHex.mid(i, 2);
                strDisplay += st;
                strDisplay += " ";
            }
            ui->txtSendUdpServer->setText(strDisplay);
        }
        else {
            QByteArray buf = myHelper::StringToHex(str);
            ui->txtSendUdpServer->setText(buf);
        }
    }
    App::HexSendUdpServer = checked;
}

/*
 * 定时发送
 */
void MainWindow::on_ckAutoSendUdpServer_toggled(bool checked)
{
    App::AutoSendUdpServer = checked;

    App::IntervalUdpServer = ui->txtIntervalUdpServer->text().toInt();
    timerUdpServer->setInterval(App::IntervalUdpServer);

    if (App::AutoSendUdpServer) {
        timerUdpServer->start();
    }
    else {
        timerUdpServer->stop();
    }
}

/*
 * 打开文件
 */
void MainWindow::on_btnOpenFileUdpServer_clicked()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    QString fileName = QFileDialog::getOpenFileName(this, trUtf8("打开"), "", trUtf8("所有文件(*.*)"));
    QFile file(fileName);
    QString fileDisplay;

    if (file.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        QTextStream textStream(&file);
        while (!textStream.atEnd())
        {
            if (App::HexReceiveUdpServer) {  //十六进制显示
                QString str = textStream.readAll().toAscii().toHex();
                str = str.toUpper();
                for (int i = 0; i < str.length(); i += 2) {
                    QString st = str.mid(i, 2);
                    fileDisplay += st;
                    fileDisplay += " ";
                }
            }
            else {
                fileDisplay = textStream.readAll();
            }
            ui->txtSendUdpServer->append(fileDisplay.toAscii());
        }
    }
    else {
        QMessageBox::information(NULL, NULL, "open file error");
    }
}

/*
 * 保存数据
 */
void MainWindow::on_btnSaveFileUdpServer_clicked()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    QString curFile;
    QString fileName = QFileDialog::getSaveFileName(this, trUtf8("另存为"), curFile, trUtf8("TXT文档(*.txt);;所有文件(*.*)"));

    if (fileName.isEmpty()) {
        return;
    }

    if (!(fileName.endsWith(".txt", Qt::CaseInsensitive))) {
        fileName += ".txt";
    }

    QFile file(fileName);

    if (!file.open(QFile::WriteOnly | QFile::Unbuffered)) {
        QMessageBox::critical(this, tr("critical"), trUtf8("无法写入数据！"), QMessageBox::Yes);
        return;
    }
    else {
        QTextStream out(&file);
        out << ui->browDataUdpServer->toPlainText();
        return;
    }
}

/*
 * 自动清空
 */
void MainWindow::on_ckAutoClearUdpServer_toggled(bool checked)
{
    App::AutoClearUdpServer = checked;
}

/*
 * 暂停接收
 */
void MainWindow::on_ckStopAcceptUdpServer_toggled(bool checked)
{
    App::StopAcceptUdpServer = checked;

    if (checked) {
        ui->browDataUdpServer->setEnabled(false);
    }
    else {
        ui->browDataUdpServer->setEnabled(true);
    }
}

/*
 * 换行显示
 */
void MainWindow::on_ckAutoNewLineUdpServer_toggled(bool checked)
{
    App::NewLineUdpServer = checked;
}

/* TCP 服务端 */

/*
 * 监听
 */
void MainWindow::on_btnTcpListen_clicked()
{
    if (ui->btnTcpListen->text() == trUtf8("监听")) {
        App::TcpListenPort = ui->txtTcpListenPort->text().toInt();
        bool ok = tcpServer->listen(QHostAddress::Any, App::TcpListenPort);
        if (ok) {
            ui->btnTcpListen->setText(trUtf8("停止"));
            ui->btnSendTcpServer->setEnabled(true);
            ui->ckAutoSendTcpServer->setEnabled(true);
            ui->txtTcpListenPort->setEnabled(false);
            appendTcpServer(0, trUtf8("监听成功"));

            updateStateBar(trUtf8("TCP服务器开启") +
                           chelper.getLocalHostIP().toString() + ":" +
                           QString().number(App::TcpListenPort),
                           QVariant(QVariant::Int), QVariant(QVariant::Int));
        }
        else {
            appendTcpServer(1, trUtf8("监听失败，请检查端口是否被占用"));
        }
    }
    else {
        ui->listTcpClient->clear();
        tcpServer->closeAll();
        ui->btnTcpListen->setText(trUtf8("监听"));
        ui->btnSendTcpServer->setEnabled(false);
        ui->ckAutoSendTcpServer->setEnabled(false);
        ui->txtTcpListenPort->setEnabled(true);
        appendTcpServer(0, trUtf8("停止监听成功"));

        updateStateBar(trUtf8("TCP停止监听"), QVariant(QVariant::Int), QVariant(QVariant::Int));
    }
}

/*
 * 清空接收区
 */
void MainWindow::on_btnClearAcceptTcpServer_clicked()
{
    ui->browDataTcpServer->clear();
    countTcpServer = 0;
}

/*
 * 清空发送区
 */
void MainWindow::on_btnClearSendTcpServer_clicked()
{
    ui->txtSendTcpServer->clear();
}

/*
 * 输出打印
 */
void MainWindow::appendTcpServer(quint8 type, QString msg)
{
    if (countTcpServer > msgMaxCount) {
        ui->browDataTcpServer->clear();
        countTcpServer = 0;
    }

    QString str;

    if (type == 0) {
        str = trUtf8(">>发送：");
        ui->browDataTcpServer->setTextColor(QColor("dodgerblue"));
    }
    else if (type == 1) {
        str = trUtf8("<<接收：");
        ui->browDataTcpServer->setTextColor(QColor("red"));
    }

    if (App::NewLineTcpServer) {
        ui->browDataTcpServer->append("\n");
    }

    if (App::StopAcceptTcpServer) {
        ui->browDataTcpServer->append(trUtf8("时间[%1] %2 %3").arg(TIMEMS).arg(str).arg(msg));
    }

    countTcpServer++;
}

/*
 * 连接
 */
void MainWindow::clientConnect(int, QString ip, int port)
{
    appendTcpServer(1, trUtf8("客户端[%1:%2]上线").arg(ip).arg(port));

    ui->listTcpClient->clear();
    ui->listTcpClient->addItems(tcpServer->getClientInfo());
    ui->labTcpClientCount->setText(trUtf8("已连接客户端共 % 1个").arg(tcpServer->getClientCount()));

    int count = ui->listTcpClient->count();

    if (count > 0) {
        ui->listTcpClient->setCurrentRow(count - 1);
    }
}

/*
 * 断开
 */
void MainWindow::clientDisConnect(int, QString ip, int port)
{
    appendTcpServer(1, trUtf8("客户端[%1:%2]下线").arg(ip).arg(port));

    ui->listTcpClient->clear();
    ui->listTcpClient->addItems(tcpServer->getClientInfo());
    ui->labTcpClientCount->setText(trUtf8("已连接客户端 %1 个").arg(tcpServer->getClientCount()));

    int count = ui->listTcpClient->count();

    if (count > 0) {
        ui->listTcpClient->setCurrentRow(count - 1);
    }
}

/*
 * 读取数据
 */
void MainWindow::clientReadData(int, QString ip, int port, QByteArray data)
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));

    QString buffer;
    if (App::HexReceiveTcpServer) {
        buffer = myHelper::byteArrayToHexStr(data);
    }
    else {
        buffer = QString(data);
    }

    appendTcpServer(1, QString("%1 [%2:%3]").arg(buffer).arg(ip).arg(port));

    //接收数量
    int num = 0;
    num = num + data.size();
    updateStateBar(QString(), num, QVariant(QVariant::Int));
}

/*
 * 发送数据
 */
void MainWindow::sendDataTcpServer()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    QByteArray buf = ui->txtSendTcpServer->document()->toPlainText().toAscii();
    QString data = myHelper::byteArraytoStr(buf);
    sendDataTcpServer(data);

    if (App::AutoClearTcpServer) {
        ui->txtSendTcpServer->clear();
    }
}

void MainWindow::sendDataTcpServer(QString data)
{
    if (!tcpServer->isListening()) {
        return;
    }

    if (data.isEmpty()) {
        return;
    }

    bool all = ui->ckAllTcpServer->isChecked();
    QString str = ui->listTcpClient->currentIndex().data().toString();

    if (str.isEmpty()) {
        return;
    }

    QStringList list = str.split(":");
    QString ip = list.at(0);
    int port = list.at(1).toInt();

    QByteArray buffer;
    if (App::HexSendTcpServer) {
        buffer = QByteArray::fromHex(data.toLatin1().data());
    }
    else {
        buffer.append(data);
    }

    if (!all) {
        tcpServer->sendData(ip, port, buffer);
    }
    else {
        tcpServer->sendData(buffer);
    }

    appendTcpServer(0, data);

    int num = 0;
    num = num + buffer.size();
    updateStateBar(QString(), QVariant(QVariant::Int), num);
}

void MainWindow::sendDataTcpServer(QString ip, int port, QString data)
{
    if (!tcpServer->isListening()) {
        return;
    }
    QByteArray buffer;
    if (App::HexSendTcpServer) {
        buffer = QByteArray::fromHex(data.toLatin1().data());
    }
    else {
        buffer.append(data);
    }

    tcpServer->sendData(ip, port, buffer);
    appendTcpServer(0, data);
}

/*
 * 十六进制显示
 */
void MainWindow::on_ckHexReceiveTcpServer_toggled(bool checked)
{
    App::HexReceiveTcpServer = checked;
}

/*
 * 十六进制发送
 */
void MainWindow::on_ckHexSendTcpServer_toggled(bool checked)
{
    QByteArray buf = ui->txtSendTcpServer->document()->toPlainText().toAscii();
    QString str = myHelper::byteArraytoStr(buf);

    if (!str.isEmpty()) {
        if (checked) {
            QString strtoHex = str.toAscii().toHex().data();
            QString strDisplay;
            strtoHex = strtoHex.toUpper();

            for (int i = 0; i < strtoHex.length(); i += 2) {
                QString st = strtoHex.mid(i, 2);
                strDisplay += st;
                strDisplay += " ";
            }
            ui->txtSendTcpServer->setText(strDisplay);
        }
        else {
            QByteArray buf = myHelper::StringToHex(str);
            ui->txtSendTcpServer->setText(buf);
        }
    }

    App::HexSendTcpServer = checked;
}

/*
 * 定时发送
 */
void MainWindow::on_ckAutoSendTcpServer_toggled(bool checked)
{
    App::AutoSendTcpServer = checked;
    App::IntervalTcpServer = ui->txtIntervalTcpServer->text().toInt();
    timerTcpServer->setInterval(App::IntervalTcpServer);

    if (App::AutoSendTcpServer) {
        timerTcpServer->start();
    }
    else {
        timerTcpServer->stop();
    }
}

/*
 * 打开文件
 */
void MainWindow::on_btnOpenFileTcpServer_clicked()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    QString fileName = QFileDialog::getOpenFileName(this, trUtf8("打开"), "", trUtf8("所有文件(*.*)"));
    QFile file(fileName);
    QString fileDisplay;

    if (file.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        QTextStream textStream(&file);
        while (!textStream.atEnd()) {
            if (App::HexReceiveTcpServer) {
                QString str = textStream.readAll().toAscii().toHex();
                str = str.toUpper();
                for (int i = 0; i < str.length(); i += 2) {
                    QString st = str.mid(i, 2);
                    fileDisplay += st;
                    fileDisplay += " ";
                }
            }
            else {
                fileDisplay = textStream.readAll();
            }

            ui->txtSendTcpServer->append(fileDisplay.toAscii());
        }
    }
    else {
        QMessageBox::information(NULL, NULL, "open file error");
    }
}

/*
 * 保存数据
 */
void MainWindow::on_btnSaveFileTcpServer_clicked()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    QString curFile;
    QString fileName = QFileDialog::getSaveFileName(this, trUtf8("另存为"), curFile, trUtf8("TXT文档(*.txt);;所有文件(*.*)"));

    if (fileName.isEmpty()) {
        return;
    }
    if (!(fileName.endsWith(".txt", Qt::CaseInsensitive))) {
        fileName += ".txt";
    }

    QFile file(fileName);

    if (!file.open(QFile::WriteOnly | QFile::Unbuffered)) {
        QMessageBox::critical(this, tr("critical"), trUtf8("无法写入数据！"), QMessageBox::Yes);
        return;
    }
    else {
        QTextStream out(&file);
        out << ui->browDataTcpServer->toPlainText();
        return;
    }
}

/*
 * 自动清空
 */
void MainWindow::on_ckAutoClearTcpServer_toggled(bool checked)
{
    App::AutoClearTcpServer = checked;
}

/*
 * 暂停接收
 */
void MainWindow::on_ckStopAcceptTcpServer_toggled(bool checked)
{
    App::StopAcceptTcpServer = checked;

    if (checked) {
        ui->browDataTcpServer->setEnabled(false);
    }
    else {
        ui->browDataTcpServer->setEnabled(true);
    }
}

/*
 * 换行显示
 */
void MainWindow::on_ckAutoNewLineTcpServer_toggled(bool checked)
{
    App::NewLineTcpServer = checked;
}

/*** TCP客户端 ***/

/*
 * TCP连接
 */
void MainWindow::on_btnTcpConnect_clicked()
{
    App::TcpServerIP = ui->txtTcpServerIP->text();
    App::TcpServerPort = ui->txtTcpServerPort->text().toInt();
    if (ui->btnTcpConnect->text() == trUtf8("连接")) {
        tcpClient->connectToHost(App::TcpServerIP, App::TcpServerPort);

        if (tcpClient->waitForConnected(1000)) {
            ui->btnTcpConnect->setText(trUtf8("断开"));
            ui->btnSendTcpClient->setEnabled(true);
            ui->ckAutoSendTcpClient->setEnabled(true);
            ui->txtTcpServerIP->setEnabled(false);
            ui->txtTcpServerPort->setEnabled(false);
            appendTcpClient(0, trUtf8("连接服务器成功"));

            updateStateBar(trUtf8("TCP客户端开启") +
                           trUtf8("客户端IP[%1]").arg(chelper.getLocalHostIP().toString()),
                           QVariant(QVariant::Int), QVariant(QVariant::Int));
        }
        else {
            appendTcpClient(1, trUtf8("连接服务器失败"));
        }
    }
    else {
        tcpClient->disconnectFromHost();

        if (tcpClient->state() == QAbstractSocket::UnconnectedState || tcpClient->waitForDisconnected(1000)) {
            ui->btnTcpConnect->setText(trUtf8("连接"));
            ui->btnSendTcpClient->setEnabled(false);
            ui->ckAutoSendTcpClient->setEnabled(false);
            ui->txtTcpServerIP->setEnabled(true);
            ui->txtTcpServerPort->setEnabled(true);
            appendTcpClient(0, trUtf8("断开连接成功"));

            updateStateBar(trUtf8("TCP断开连接"), QVariant(QVariant::Int), QVariant(QVariant::Int));
        }
    }
}

/*
 * 清空接收区
 */
void MainWindow::on_btnClearAcceptTcpClient_clicked()
{
    ui->browDataTcpClient->clear();
    countTcpClient = 0;
}

/*
 * 清空发送区
 */
void MainWindow::on_btnClearSendTcpClient_clicked()
{
    ui->txtSendTcpClient->clear();
}

/*
 * 输出打印
 */
void MainWindow::appendTcpClient(quint8 type, QString msg)
{
    if (countTcpClient > msgMaxCount) {
        ui->browDataTcpClient->clear();
        countTcpClient = 0;
    }

    QString str;

    if (type == 0) {
        str = trUtf8(">>发送：");
        ui->browDataTcpClient->setTextColor(QColor("dodgerblue"));
    }
    else if (type == 1) {
        str = trUtf8("<<接收：");
        ui->browDataTcpClient->setTextColor(QColor("red"));
    }

    if (App::NewLineTcpClient) {
        ui->browDataTcpClient->append("\n");
    }

    if (App::StopAcceptTcpClient) {
        ui->browDataTcpClient->append(trUtf8("时间[%1] %2 %3").arg(TIMEMS).arg(str).arg(msg));
    }

    countTcpClient++;
}

/*
 * 读取数据
 */
void MainWindow::tcpClientReadData()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    if (tcpClient->bytesAvailable() <= 0) {
        return;
    }

    QByteArray data = tcpClient->readAll();
    QString buffer;

    if (App::HexReceiveTcpClient) {
        buffer = myHelper::byteArrayToHexStr(data);
    }
    else {
        buffer = QString(data);
    }

    appendTcpClient(1, buffer);

    int num = 0;
    num = num + buffer.size();
    updateStateBar(QString(), num, QVariant(QVariant::Int));
}

/*
 * 读取错误
 */
void MainWindow::tcpClientReadError()
{
    tcpClient->disconnectFromHost();
    ui->btnTcpConnect->setText(trUtf8("连接"));
    ui->btnSendTcpClient->setEnabled(false);
    ui->txtSendTcpClient->clear();
    appendTcpClient(1, trUtf8("连接服务器失败，原因：%1").arg(tcpClient->errorString()));
}

/*
 * 发送数据
 */
void MainWindow::sendDataTcpClient()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    QByteArray buf = ui->txtSendTcpClient->document()->toPlainText().toAscii();
    QString data = myHelper::byteArraytoStr(buf);

    sendDataTcpClient(data);
    if (App::AutoClearTcpClient) {
        ui->txtSendTcpClient->clear();
    }
}

void MainWindow::sendDataTcpClient(QString data)
{
    if (!tcpClient->isWritable()) {
        return;
    }

    if (data.isEmpty()) {
        return;
    }

    if (!ui->btnSendTcpClient->isEnabled()) {
        return;
    }

    QByteArray buffer;

    if (App::HexSendTcpClient) {
        buffer = QByteArray::fromHex(data.toLatin1().data());
    }
    else {
        buffer.append(data);
    }

    tcpClient->write(buffer);
    appendTcpClient(0, data);

    int num = 0;
    num  = num + buffer.size();
    updateStateBar(QString(), QVariant(QVariant::Int), num);
}

/*
 * 十六进制显示
 */
void MainWindow::on_ckHexReceiveTcpClient_toggled(bool checked)
{
    App::HexReceiveTcpClient = checked;
}

/*
 * 十六进制发送
 */
void MainWindow::on_ckHexSendTcpClient_toggled(bool checked)
{
    QByteArray buf = ui->txtSendTcpClient->document()->toPlainText().toAscii();
    QString str = myHelper::byteArraytoStr(buf);

    if (!str.isEmpty()) {
        if (checked) {
            QString strtoHex = str.toAscii().toHex().data();
            QString strDisplay;
            strtoHex = strtoHex.toUpper();

            for (int i = 0; i < strtoHex.length(); i+= 2) {
                QString st = strtoHex.mid(i ,2);
                strDisplay += st;
                strDisplay += " ";
            }
            ui->txtSendTcpClient->setText(strDisplay);
        }
        else {
            QByteArray buf = myHelper::StringToHex(str);
            ui->txtSendTcpClient->setText(buf);
        }
    }
    App::HexSendTcpClient = checked;
}

/*
 * 定时发送
 */
void MainWindow::on_ckAutoSendTcpClient_toggled(bool checked)
{
    App::AutoSendTcpClient = checked;
    App::IntervalTcpClient = ui->txtIntervalTcpClient->text().toInt();
    timerTcpClient->setInterval(App::IntervalTcpClient);

    if (App::AutoSendTcpClient) {
        timerTcpClient->start();
    }
    else {
        timerTcpClient->stop();
    }
}


/*
 * 打开文件
 */
void MainWindow::on_btnOpenFileTcpClient_clicked()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    QString fileName = QFileDialog::getOpenFileName(this, trUtf8("打开"), "", trUtf8("所有文件(*.*)"));
    QFile file(fileName);
    QString fileDisplay;

    if (file.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        QTextStream textStream(&file);
        while (!textStream.atEnd()) {
            if (App::HexReceiveTcpClient) {
                QString str = textStream.readAll().toAscii().toHex();
                str = str.toUpper();
                for (int i = 0; i < str.length(); i += 2) {
                    QString st = str.mid(i, 2);
                    fileDisplay += st;
                    fileDisplay += " ";
                }
            }
            else {
                fileDisplay = textStream.readAll();
            }

            ui->txtSendTcpClient->append(fileDisplay.toAscii());
        }
    }
    else {
        QMessageBox::information(NULL, NULL, "open file error");
    }
}

/*
 * 保存数据
 */
void MainWindow::on_btnSaveFileTcpClient_clicked()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
    QString curFile;
    QString fileName = QFileDialog::getSaveFileName(this, trUtf8("另存为"), curFile, trUtf8("TXT文档(*.txt);;所有文件(*.*)"));

    if (fileName.isEmpty()) {
        return;
    }
    if (!(fileName.endsWith(".txt", Qt::CaseInsensitive))) {
        fileName += ".txt";
    }

    QFile file(fileName);

    if (!file.open(QFile::WriteOnly | QFile::Unbuffered)) {
        QMessageBox::critical(this, tr("critical"), trUtf8("无法写入数据！"), QMessageBox::Yes);
        return;
    }
    else {
        QTextStream out(&file);
        out << ui->browDataTcpClient->toPlainText();
        return;
    }
}

/*
 * 自动清空
 */
void MainWindow::on_ckAutoClearTcpClient_toggled(bool checked)
{
    App::AutoClearTcpClient = checked;
}

/*
 * 暂停接收
 */
void MainWindow::on_ckStopAcceptTcpClient_toggled(bool checked)
{
    App::StopAcceptTcpClient = checked;

    if (checked) {
        ui->browDataTcpClient->setEnabled(false);
    }
    else {
        ui->browDataTcpClient->setEnabled(true);
    }
}

/*
 * 换行显示
 */
void MainWindow::on_ckAutoNewLineTcpClient_toggled(bool checked)
{
    App::NewLineTcpClient = checked;
}






