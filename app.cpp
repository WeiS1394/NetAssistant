#include "app.h"
#include "myhelper.h"

/* TCP服务端 */
bool App::HexSendTcpServer = false;
bool App::HexReceiveTcpServer = false;
bool App::AutoSendTcpServer = false;
bool App::AutoClearTcpServer = false;
bool App::StopAcceptTcpServer = false;
bool App::NewLineTcpServer = false;
int App::IntervalTcpServer = 1000;
int App::TcpListenPort = 6000;

/* TCP客户端 */
bool App::HexSendTcpClient = false;
bool App::HexReceiveTcpClient = false;
bool App::AutoSendTcpClient = false;
bool App::AutoClearTcpClient = false;
bool App::StopAcceptTcpClient = false;
bool App::NewLineTcpClient = false;
int App::IntervalTcpClient = 1000;
QString App::TcpServerIP = "127.0.0.1";
int App::TcpServerPort = 6000;

/* UDP服务 */
bool App::HexSendUdpServer = false;
bool App::HexReceiveUdpServer = false;
bool App::AutoSendUdpServer = false;
bool App::AutoClearUdpServer = false;
bool App::StopAcceptUdpServer = false;
bool App::NewLineUdpServer = false;
int App::IntervalUdpServer = 1000;





































