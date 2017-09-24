#ifndef APP_H
#define APP_H

class QString;

class App
{
public:
    //TCP服务器参数
    static bool HexSendTcpServer;  //16进制发送
    static bool HexReceiveTcpServer;  //16进制显示
    static bool AutoSendTcpServer;  //自动发送数据
    static int IntervalTcpServer;  //发送数据间隔
    static int TcpListenPort;  //监听端口
    static bool AutoClearTcpServer;  //自动清空
    static bool StopAcceptTcpServer;  //暂停接收
    static bool NewLineTcpServer;  //换行显示

    //UDP服务
    static bool HexSendUdpServer;
    static bool HexReceiveUdpServer;
    static bool AutoSendUdpServer;
    static int IntervalUdpServer;
    static bool AutoClearUdpServer;
    static bool StopAcceptUdpServer;
    static bool NewLineUdpServer;

    //TCP客户端参数
    static bool HexSendTcpClient;
    static bool HexReceiveTcpClient;
    static bool AutoSendTcpClient;
    static int IntervalTcpClient;
    static QString TcpServerIP;
    static int TcpServerPort;
    static bool AutoClearTcpClient;
    static bool StopAcceptTcpClient;
    static bool NewLineTcpClient;
};

#endif // APP_H
