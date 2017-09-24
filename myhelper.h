#ifndef MYHELPER_H
#define MYHELPER_H

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

class myHelper : public QObject
{
public:
    /*
     * QByteArray转十六进制，返回值QString
     */
    static QString byteArrayToHexStr(QByteArray data)
    {
        QString temp = "";
        QString hex = data.toHex();

        for (int i = 0; i < hex.length(); i = i + 2) {
            temp += hex.mid(i, 2) + " ";
        }

        return temp.trimmed().toUpper();
    }

    /*
     * QByteArray转Asicii码，返回值类型为QString
     */
    static QString byteArrayToAsciiStr(QByteArray data)
    {
        QString temp;
        int len = data.size();

        for (int i = 0; i < len; i++) {
            char b = data.at(i);

            if (0x00 == b) {
                temp += QString("\\NUL");
            } else if (0x01 == b) {
                temp += QString("\\SOH");
            } else if (0x02 == b) {
                temp += QString("\\STX");
            } else if (0x03 == b) {
                temp += QString("\\ETX");
            } else if (0x04 == b) {
                temp += QString("\\EOT");
            } else if (0x05 == b) {
                temp += QString("\\ENQ");
            } else if (0x06 == b) {
                temp += QString("\\ACK");
            } else if (0x07 == b) {
                temp += QString("\\BEL");
            } else if (0x08 == b) {
                temp += QString("\\BS");
            } else if (0x09 == b) {
                temp += QString("\\HT");
            } else if (0x0A == b) {
                temp += QString("\\LF");
            } else if (0x0B == b) {
                temp += QString("\\VT");
            } else if (0x0C == b) {
                temp += QString("\\FF");
            } else if (0x0D == b) {
                temp += QString("\\CR");
            } else if (0x0E == b) {
                temp += QString("\\SO");
            } else if (0x0F == b) {
                temp += QString("\\SI");
            } else if (0x10 == b) {
                temp += QString("\\DLE");
            } else if (0x11 == b) {
                temp += QString("\\DC1");
            } else if (0x12 == b) {
                temp += QString("\\DC2");
            } else if (0x13 == b) {
                temp += QString("\\DC3");
            } else if (0x14 == b) {
                temp += QString("\\DC4");
            } else if (0x15 == b) {
                temp += QString("\\NAK");
            } else if (0x16 == b) {
                temp += QString("\\SYN");
            } else if (0x17 == b) {
                temp += QString("\\ETB");
            } else if (0x18 == b) {
                temp += QString("\\CAN");
            } else if (0x19 == b) {
                temp += QString("\\EM");
            } else if (0x1A == b) {
                temp += QString("\\SUB");
            } else if (0x1B == b) {
                temp += QString("\\ESC");
            } else if (0x1C == b) {
                temp += QString("\\FS");
            } else if (0x1D == b) {
                temp += QString("\\GS");
            } else if (0x1E == b) {
                temp += QString("\\RS");
            } else if (0x1F == b) {
                temp += QString("\\US");
            } else if (0x7F == b) {
                temp += QString("\\x7F");
            } else if (0x5C == b) {
                temp += QString("\\x5C");
            } else if (0x20 >= b) {
                temp += QString("\\x%1").arg(decimalToStrHex((quint8)b));
            } else {
                temp += QString("%1").arg(b);
            }
        }

        return temp.trimmed();
    }

    static QString decimalToStrHex(int decimal)
    {
        QString temp = QString::number(decimal, 16);

        if (temp.length() == 1) {
            temp = "0" + temp;
        }
        return temp.toUpper();
    }

    static QByteArray StringToHex(QString str)
    {
        QByteArray senddata;
        int hexdata, lowhexdata;
        int hexdatalen = 0;
        int len = str.length();
        senddata.resize(len/2);
        char lstr, hstr;
        for (int i = 0; i <len; ) {
            hstr = str[i].toLatin1();
            if (hstr == ' ') {
                i++;
                continue;
            }
            i++;
            if (i >= len)
                break;
            lstr = str[i].toLatin1();
            hexdata = ConvertHexChar(hstr);
            lowhexdata = ConvertHexChar(lstr);
            if ((hexdata == 16) || (lowhexdata == 16))
                break;
            else
                hexdata = hexdata*16 + lowhexdata;
            i++;
            senddata[hexdatalen] = (char)hexdata;
            hexdatalen++;
        }
        senddata.resize(hexdatalen);
        return senddata;
    }

    static char ConvertHexChar(char ch)
    {
        if ((ch >= '0') && (ch <= '9'))  //ASCII码的“41”，其编码值为52，48
            return ch - 0x30;
        else if ((ch >= 'A') && (ch <= 'F'))
            return ch - 'A' + 10;
        else if ((ch >= 'a') && (ch <= 'f'))
            return ch - 'a' + 10;
        else
            return(-1);
    }

    static QString byteArraytoStr(QByteArray buf)
    {
        buf.replace(QByteArray("\n"), QByteArray("\r\n"));
        return QString(buf);
    }
};

#endif // MYHELPER_H
