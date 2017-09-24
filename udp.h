#ifndef UDP_H
#define UDP_H

#include <QObject>
#include <QUdpSocket>

class UDP : public QObject
{
    Q_OBJECT
public:
    explicit UDP(QObject *parent = 0);

    void sendData(const QString string, const QString targetIP, const int port);  //发送数据
    void udpStart(const QHostAddress localIP, const int listnerPort,
                  const QHostAddress targetIP, const int targetPort);  //开启UDP服务
    void udpStop(const QString string, const QString targetIP, const int port);  //关闭UDPf服务
    void readyRead(QUdpSocket *socket);  //读取数据

signals:
    void valueChanged(qint8 type, QString newValue);
    void updateState(QString state, QVariant inNum, QVariant outNum);

public slots:
    void readyListnerRead();  //监听读取数据
    void readySendRead();  //发送得到回传数据
    void connection_error(QAbstractSocket::SocketError err);  //处理网络连接错误

private:
    QUdpSocket *udpSendSocket = nullptr;
    QUdpSocket *udpListnerSocket = nullptr;
    void udpListnerStart(const QHostAddress ip, const int port);  //开启监听
    void udpListnerStop();  //关闭监听

};

#endif // UDP_H
