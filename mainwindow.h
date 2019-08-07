#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QListView>
#include <QKeyEvent>
#include "qextserialport.h"
#include "qextserialenumerator.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:    
    bool eventFilter(QObject *obj, QEvent *event);//监听事件

private:
    Ui::MainWindow *ui;

    QextSerialPort *myCom;
    QTimer *myTimer;

    QTimer *myReadTimer;//定时读取串口数据
    QTimer *mySendTimer;//定时发送串口数据
    QTimer *mySaveTimer;//定时保存串口数据

    int SendCount;//发送数据计数
    int ReceiveCount;//接收数据计数
    bool IsShow;//是否显示数据
    bool IsDebug;//是否启用调试,接收到数据后模拟发送数据    
    bool IsAutoClear;//是否自动清空
    bool IsHexSend;//是否16进制数据发送
    bool IsHexReceive;//是否16进制数据接收    

    QStringList SendDataList;//转发数据链表

    QextSerialEnumerator *enumerator;//串口设备名枚举

private:
    void initWnd();//初始化界面以及其他处理
    void ChangeEnable(bool b);//改变状态

private slots:
    void on_cboxSave_currentIndexChanged(int index);
    void on_cboxSend_currentIndexChanged(int index);
    void on_btnData_clicked();
    void on_btnSend_clicked();
    void on_btnSave_clicked();
    void on_btnClearAll_clicked();
    void on_btnStopShow_clicked();
    void on_btnClearReceive_clicked();
    void on_btnClearSend_clicked();
    void on_ckHexSend_stateChanged(int arg1);
    void on_ckHexReceive_stateChanged(int arg1);
    void on_ckIsAutoSave_stateChanged(int arg1);
    void on_ckIsAutoSend_stateChanged(int arg1);
    void on_ckIsAutoClear_stateChanged(int arg1);
    void on_ckIsDebug_stateChanged(int arg1);
    void on_btnOpen_clicked();

    void SetTime();//动态显示时间
    void ReadMyCom();//读取串口数据
    void WriteMyCom();//写串口数据
    void SaveMyCom();//保存串口数据    

    void ReadConfigData();//读取配置文件数据
    void ReadSendData();//读取转发文件数据

    void onPortAddedOrRemoved();//串口设备拔擦检测

};

#endif // MAINWINDOW_H
