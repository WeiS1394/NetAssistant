#include "mainwindow.h"
#include <QApplication>
#include "udp.h"
#include "myhelper.h"
#include <Qtcore/qtextcodec.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
