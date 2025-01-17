#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include "mhelper.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //设置字符编码和外观样式
    mHelper::SetUTF8Code();
    mHelper::SetStyle();

    //加载中文字符
    QTranslator translator;
    translator.load(":/qt_zh_CN.qm");
    a.installTranslator(&translator);

    MainWindow w;
    w.show();

    return a.exec();
}
