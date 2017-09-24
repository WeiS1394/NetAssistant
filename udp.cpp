/*
 * 开启UDP协议，建立UDP服务器和UDP客户端
 */

#include "udp.h"
#include "myhelper.h"
#include "mainwindow.h"
#include <QMessageBox>
#include "app.h"


UDP::UDP(QObject *parent) :
    QObject(parent)
{

}

/*
 * 处理网络连接错误
 */
void UDP::connection_error(QAbstractSocket::SocketError err)
{
    switch (err)
    {
    case 0:QMessageBox::critical(0, "connection error", "The connect was refused by the peer (or timed out).", QMessageBox::Ok);
        break;
    case 2:QMessageBox::critical(0, "connection error", "The host address was not found.", QMessageBox::Ok);
        break;
    case QAbstractSocket::NetworkError:QMessageBox::critical(0, "connection error", "An error occurred with the network.", QMessageBox::Ok);
        break;
    case QAbstractSocket::RemoteHostClosedError:QMessageBox::critical(0, "connection error", "undefine error.", QMessageBox::Ok);
        break;
    default:QMessageBox::critical(0, "connection error", "undefine error", QMessageBox::Ok);
        qDebug() << "error is ..." << err;
       break;
    }
}

/*
 * 将数据内容发送给目标IP
 */
void UDP::sendData(const QString string, const QString targetIP, const int port)
{
    if (udpSendSocket == nullptr) {
        return;
    }

    QByteArray Data;
    if (App::HexSendUdpServer) {
        Data = QByteArray::fromHex(string.toLatin1().data());
    }
    else {
        Data.append(string);
    }

    QString str = trUtf8("目标：%1[%2] ").arg(targetIP).arg(port);

    if (!App::StopAcceptUdpServer) {
        emit valueChanged(0 ,str + string);
    }

    emit updateState(QString(), QVariant(QVariant::Int), udpSendSocket->writeDatagram(Data, QHostAddress(targetIP), port));
}

/*
 * 监听地址和端口
 */
void UDP::udpListnerStart(const QHostAddress ip, const int port)
{
    qDebug("%s", __func__);
    udpListnerSocket = new QUdpSocket(this);
    if (!udpListnerSocket->bind(ip, port)) {
        qWarning("NULL");
    }

    connect(udpListnerSocket, SIGNAL(readyRead()), this, SLOT(readyListnerRead()));
    connect(udpListnerSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(connection_error(QAbstractSocket::SocketError)));
}

/*
 * 停止监听
 */
void UDP::udpListnerStop()
{
    qDebug("%s", __func__);
    if (udpListnerSocket != nullptr) {
        udpListnerSocket->close();
        udpListnerSocket = nullptr;
    }
}

/*
 * 启动UDP服务
 */
void UDP::udpStart(const QHostAddress localIP, const int listnerPort, const QHostAddress targetIP, const int targetPort)
{
    if (udpSendSocket == nullptr) {  //开启发送
        udpSendSocket = new QUdpSocket(this);
        connect(udpSendSocket, SIGNAL(readyRead()), this, SLOT(readySendRead()));
        connect(udpSendSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(connection_error(QAbstractSocket::SocketError)));
    }
    udpListnerStart(localIP, listnerPort);   //开启监听
}

void UDP::udpStop(const QString string, const QString targetIP, const int port)
{
    if (udpSendSocket != nullptr) {
        udpSendSocket->close();
        udpSendSocket = nullptr;
    }
    udpListnerStop();
}

/*
 * 监听读取数据
 */
void UDP::readyListnerRead()
{
    readyRead(udpListnerSocket);
}

/*
 * 读取数据
 */
void UDP::readyRead(QUdpSocket *socket)
{
    QHostAddress sender;
    quint16 senderport;
    QByteArray Buffer;
    QString data;

    Buffer.resize(socket->pendingDatagramSize());
    socket->readDatagram(Buffer.data(), Buffer.size(), &sender, &senderport);

    qDebug() << "Message from:" << sender.toString();  // 调试信息
    qDebug() << "Message prot:" << senderport;
    qDebug() << "Message:" << Buffer;

    if (App::HexReceiveUdpServer) {    //十六进制接收
        data = myHelper::byteArrayToHexStr(Buffer);
    }
    else {
        data = QString(Buffer);
    }

    QString str = trUtf8("来自：%1[%2] ").arg(sender.toString()).arg(senderport);

    if (!App::StopAcceptUdpServer) {
        emit valueChanged(1, str + data);
    }

    emit updateState(QString(), Buffer.size(), QVariant(QVariant::Int));
}

/*
 * 发送得到回传数据
 */
void UDP::readySendRead()
{
    readyRead(udpSendSocket);
}











