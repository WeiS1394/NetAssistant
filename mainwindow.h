#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QLabel>
#include <QPushButton>
#include "commonhelper.h"
#include "udp.h"

#define TIMEMS qPrintable (QTime::currentTime().toString("HH:mm:ss zzz"))

class QTcpSocket;
class TcpServer;
class QUdpSocket;

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
    /* UDP服务 */
    void on_btnSendUdpServer_released();  //发送按钮
    void updateReceiveText(qint8 type, QString msg);  //接收数据
    void sendDataUdpServer();

    /* TCP服务端 */
    void sendDataTcpServer();  //发送数据
    void sendDataTcpServer(QString data);
    void sendDataTcpServer(QString ip, int port, QString data);
    void appendTcpServer(quint8 type, QString msg);  //打印数据

    /* TCP客户端 */
    void sendDataTcpClient();
    void sendDataTcpClient(QString data);
    void appendTcpClient(quint8 type, QString msg);

private slots:
     /* TCP客户端 */
    void tcpClientReadData();
    void tcpClientReadError();

     /* TCP服务端 */
    void clientReadData(int, QString ip, int port, QByteArray data);
    void clientConnect(int, QString ip, int port);
    void clientDisConnect(int, QString ip, int port);

private slots:
    /* 状态栏 */
    void clearCounterPushBtn_released();  //清零 按钮
    void updateStateBar(QString state, QVariant inNum, QVariant outNum);  //更新状态栏

    /* UDP服务 */
    void on_connectPushBtn_released();  //连接 按钮
    void on_clearReceivePushBtn_released();  //清空接收区
    void on_clearSendPushBtn_released();  //清空发送区
    void on_ckHexReceiveUdpServer_toggled(bool checked);  //十六进制接收
    void on_ckHexSendUdpServer_toggled(bool checked);  //十六进制发送
    void on_ckAutoSendUdpServer_toggled(bool checked);  //定时发送
    void on_btnOpenFileUdpServer_clicked();  //打开文件
    void on_btnSaveFileUdpServer_clicked();  //保存数据
    void on_ckAutoClearUdpServer_toggled(bool checked);  //自动清空
    void on_ckStopAcceptUdpServer_toggled(bool checked);  //暂停接收
    void on_ckAutoNewLineUdpServer_toggled(bool checked);  //换行显示

    /* TCP服务端 */
    void on_btnTcpListen_clicked();
    void on_btnClearAcceptTcpServer_clicked();
    void on_btnClearSendTcpServer_clicked();
    void on_ckHexReceiveTcpServer_toggled(bool checked);
    void on_ckHexSendTcpServer_toggled(bool checked);
    void on_ckAutoSendTcpServer_toggled(bool checked);
    void on_btnOpenFileTcpServer_clicked();
    void on_btnSaveFileTcpServer_clicked();
    void on_ckAutoClearTcpServer_toggled(bool checked);
    void on_ckStopAcceptTcpServer_toggled(bool checked);
    void on_ckAutoNewLineTcpServer_toggled(bool checked);

    /* TCP客户端 */
    void on_btnTcpConnect_clicked();
    void on_btnClearAcceptTcpClient_clicked();
    void on_btnClearSendTcpClient_clicked();
    void on_ckHexReceiveTcpClient_toggled(bool checked);
    void on_ckHexSendTcpClient_toggled(bool checked);
    void on_ckAutoSendTcpClient_toggled(bool checked);
    void on_btnOpenFileTcpClient_clicked();
    void on_btnSaveFileTcpClient_clicked();
    void on_ckAutoClearTcpClient_toggled(bool checked);
    void on_ckStopAcceptTcpClient_toggled(bool checked);
    void on_ckAutoNewLineTcpClient_toggled(bool checked);

private:
    Ui::MainWindow *ui;

    int msgMaxCount;
    int countTcpServer;
    int countTcpClient;

    QUdpSocket *udpSocket;
    TcpServer *tcpServer;
    QTcpSocket *tcpClient;

    QTimer *timerTcpServer;
    QTimer *timerTcpClient;
    QTimer *timerUdpServer;

    bool isConnect;  //连接 标志
    void connectNet();  //连接网络
    void disConnectNet();  //断开网络

    UDP client;  //UDP服务

    CommonHelper chelper;  //获取本地IPv4地址
    QString uTargetIP;  //目标IP
    int uTargetPort;  //udp 目标端口号
    int uLocalPort; //udp 本地端口号

    /* 状态栏 */
    QLabel *statusLabel;  //状态标签
    QLabel *receiveLabel;  //接收 数量 标签
    QLabel *sendLabel;  //发送 数量 标签
    QPushButton *clearCounterButton;  // 计数清零 按钮
    QLabel *timeLabel;  // 时间 标签
    quint64 uReceiveNum;  // 接收总数
    quint64 uSendNum;  //发送总数

    void init();  //初始化
};

#endif // MAINWINDOW_H
