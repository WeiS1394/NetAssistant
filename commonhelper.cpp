#include "commonhelper.h"
#include <QHostAddress>
#include <QNetworkInterface>

CommonHelper::CommonHelper(QObject *parent) :
    QObject(parent)
{
}

/*
 * 获取本地的IPv4地址
 */
QHostAddress CommonHelper::getLocalHostIP()
{
    QList<QHostAddress> AddressList = QNetworkInterface::allAddresses();
    QHostAddress result;
    foreach(QHostAddress address, AddressList) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
                address != QHostAddress::Null &&
                address != QHostAddress::LocalHost) {
            if (address.toString().contains("127.0")) {
                continue;
            }

            result = address;
            break;
        }
    }
    return result;
}
