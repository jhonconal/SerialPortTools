#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mhelper.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mHelper::FormInCenter(this);
    this->initWnd();
#ifdef Q_OS_WIN
     this->setWindowTitle(QString("串口助手"));
     this->resize(1100,620);
#else
     #ifdef Q_OS_LINUX
         this->setWindowTitle(QString("串口助手"));
         this->resize(1300,750);
    #else
        this->setWindowTitle(QString("串口助手(macOS)"));
        this->resize(1100,620);
    #endif
#endif
}

MainWindow::~MainWindow()
{    
    delete ui;
}

//监听回车键
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==ui->txtSend)
    {
        if (event->type()==QEvent::KeyPress)
        {
            QKeyEvent *keyEvent=static_cast<QKeyEvent *>(event);
            if (keyEvent->key()==Qt::Key_Return || keyEvent->key()==Qt::Key_Enter)
            {
                WriteMyCom();
                return true;
            }
        }
    }

    return QObject::eventFilter(obj,event);
}

void MainWindow::initWnd()
{    
    ReceiveCount=0;
    SendCount=0;
    IsShow=true;
    IsAutoClear=false;
    IsHexSend=true;
    IsHexReceive=true;
    IsDebug=false;

    QStringList comList;//串口号
    QStringList baudList;//波特率
    QStringList parityList;//校验位
    QStringList dataBitsList;//数据位
    QStringList stopBitsList;//停止位

    ui->txtSend->setView(new QListView);
    ui->txtSend->setStyleSheet("QComboBox{min-height: 30px;min-width: 120px;background-color: rgb(75,75,75);font-size:18px;"
                                "color: white; border-radius: 3px; padding: 1px 5px 1px 3px;	font: 75 15pt \"Times\";} "
                                "QComboBox::drop-down{width: 10px;background-color: rgb(0, 105, 217,150) }"
                                "QComboBox QAbstractItemView{background-color: rgb(255,255,255);color:black;font-size:15px;}"
                                "QComboBox QAbstractItemView:item{min-height: 30px; min-width: 120px; }");


    enumerator = new QextSerialEnumerator(this);
    enumerator->setUpNotifications();

    connect(enumerator, SIGNAL(deviceDiscovered(QextPortInfo)), SLOT(onPortAddedOrRemoved()));
    connect(enumerator, SIGNAL(deviceRemoved(QextPortInfo)), SLOT(onPortAddedOrRemoved()));

    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
        comList.append(info.portName);

    ui->cboxPortName->addItems(comList);
    ui->cboxPortName->setCurrentIndex(0);

    baudList<<"1200"<<"1800"<<"2400"<<"4800"<<"9600"
         <<"19200"<<"38400"<<"56000"<<"57600"
         <<"115200"<<"128000"<<"921600";

    ui->cboxBaudRate->addItems(baudList);
    ui->cboxBaudRate->setCurrentIndex(4);

    parityList<<"无"<<"奇"<<"偶";

#ifdef Q_OS_WIN//如果是windows系统
    parityList<<"标志";
#endif

    parityList<<"空格";

    ui->cboxParity->addItems(parityList);
    ui->cboxParity->setCurrentIndex(0);

    dataBitsList<<"5"<<"6"<<"7"<<"8";
    ui->cboxDataBit->addItems(dataBitsList);
    ui->cboxDataBit->setCurrentIndex(3);

    stopBitsList<<"1";

#ifdef Q_OS_WIN//如果是windows系统
    stopBitsList<<"1.5";
#endif

    stopBitsList<<"2";

    ui->cboxStopBit->addItems(stopBitsList);
    ui->cboxStopBit->setCurrentIndex(0);

    //读取数据(采用定时器读取数据，不采用事件，方便移植到linux)
    myReadTimer=new QTimer(this);
    myReadTimer->setInterval(300);
    connect(myReadTimer,SIGNAL(timeout()),this,SLOT(ReadMyCom()));

    //发送数据
    mySendTimer=new QTimer(this);
    mySendTimer->setInterval(5000);
    connect(mySendTimer,SIGNAL(timeout()),this,SLOT(WriteMyCom()));

    //保存数据
    mySaveTimer=new QTimer(this);
    mySaveTimer->setInterval(5000);
    connect(mySaveTimer,SIGNAL(timeout()),this,SLOT(SaveMyCom()));

    //显示日期时间
    myTimer=new QTimer(this);
    myTimer->start(1000);
    connect(myTimer,SIGNAL(timeout()),this,SLOT(SetTime()));

    QDate dateNow=QDate::currentDate();
    ui->labDate->setText(QString("日期: %1").arg(dateNow.toString("yyyy年MM月dd日 dddd")));

    for (int i=1;i<=60;i++)
    {
        ui->cboxSend->addItem(QString::number(i)+"秒");
        ui->cboxSave->addItem(QString::number(i)+"秒");
    }

    ui->cboxSave->setCurrentIndex(4);
    ui->cboxSend->setCurrentIndex(4);

    ui->cboxSend->setEnabled(false);
    ui->cboxSave->setEnabled(false);

    this->ChangeEnable(false);
    this->ReadConfigData();//读取发送数据加载到下拉框
    this->ReadSendData();//读取数据转发文件

    ui->txtSend->installEventFilter(this);//安装监听器监听发送数据框回车响应
}

void MainWindow::onPortAddedOrRemoved()
{
    QString current = ui->cboxPortName->currentText();

    ui->cboxPortName->blockSignals(true);
    ui->cboxPortName->clear();
    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
        ui->cboxPortName->addItem(info.portName);

    ui->cboxPortName->setCurrentIndex(ui->cboxPortName->findText(current));

    ui->cboxPortName->blockSignals(false);
}


void MainWindow::ReadSendData()
{
    QString fileName="SendData.txt";
    QFile file(fileName);
    if (!file.exists()){return;}//如果文件不存在则返回

    file.open(QFile::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    QString line;

    do { line=in.readLine();if (line!="") { SendDataList.append(line); }}
    while(!line.isNull());

    file.close();
}

void MainWindow::ChangeEnable(bool b)
{
    ui->cboxBaudRate->setEnabled(!b);
    ui->cboxDataBit->setEnabled(!b);
    ui->cboxParity->setEnabled(!b);
    ui->cboxPortName->setEnabled(!b);
    ui->cboxStopBit->setEnabled(!b);

    ui->gbox2->setEnabled(b);
    ui->gbox3->setEnabled(b);
    ui->gbox5->setEnabled(b);
    ui->gbox6->setEnabled(b);
    ui->btnOpen->setEnabled(true);
}

void MainWindow::SetTime()
{
    QTime timeNow=QTime::currentTime();
    ui->labTime->setText(QString("时间: %1").arg(timeNow.toString()));
}

void MainWindow::ReadMyCom()
{
    //这个判断尤为重要,否则的话直接延时再接收数据,空闲时和会出现高内存占用
    if (myCom->bytesAvailable()<=0){return;}

    mHelper::Sleep(100);//延时100毫秒保证接收到的是一条完整的数据,而不是脱节的
    QByteArray buffer=myCom->readAll();

    if (IsShow)
    {
        if (IsHexReceive)
        {
            QString tempDataHex=mHelper::ByteArrayToHexStr(buffer);
            ui->txtDataHex->append(QString("接收:%1   (时间:%2)")
                                   .arg(tempDataHex)
                                   .arg(QTime::currentTime().toString("HH:mm:ss")));

            if (IsDebug)//2013-8-6增加接收数据后转发数据，模拟设备
            {
                foreach(QString tempData,SendDataList)
                {
                    QStringList temp=tempData.split(';');
                    if (tempDataHex==temp[0])
                    {
                        //这里没有跳出循环，有可能一条数据会对应多条数据需要转发
                        myCom->write(mHelper::HexStrToByteArray(temp[1]));
                    }
                }
            }
        }
        else
        {
            QString tempDataNormal=QString(buffer);
            ui->txtDataHex->append(QString("接收:%1 时间:%2")
                                   .arg(tempDataNormal)
                                   .arg(QTime::currentTime().toString("HH:mm:ss")));

            if (IsDebug)//2013-8-6增加接收数据后转发数据，模拟设备
            {
                foreach(QString tempData,SendDataList)
                {
                    QStringList temp=tempData.split(';');
                    if (tempDataNormal==temp[0])
                    {
                        //这里没有跳出循环，有可能一条数据会对应多条数据需要转发
                        #if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
                        myCom->write(temp[1].toAscii());
                        #else
                       myCom->write(temp[1].toLatin1());
                       #endif
                    }
                }
            }
        }

        ReceiveCount=ReceiveCount+buffer.size();
        ui->labReceive->setText(QString("接收:%1 字节").arg(ReceiveCount));
    }
}

void MainWindow::WriteMyCom()
{
    QString str=ui->txtSend->currentText();
    if (str==""){ui->txtSend->setFocus();return;}//发送数据为空
    if (!myCom->isOpen()) { return; }//串口没有打开

#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
    QByteArray outData=str.toAscii();
#else
    QByteArray outData=str.toLatin1();
#endif
    int size=outData.size();

    if (IsHexSend)//转化为16进制发送
    {
        outData=mHelper::HexStrToByteArray(str);
        size=outData.size();
        myCom->write(outData);
    }
    else
    {
        size=outData.size();
        myCom->write(outData);
    }

    ui->txtDataHex->append(QString("发送:%1 时间:%2")
                           .arg(str)
                           .arg(QTime::currentTime().toString("HH:mm:ss")));

    SendCount=SendCount+size;
    ui->labSend->setText(QString("发送: %1 字节").arg(SendCount));

    if (IsAutoClear)
    {
        ui->txtSend->setCurrentIndex(-1);
        ui->txtSend->setFocus();
    }
}

void MainWindow::SaveMyCom()
{
    QString tempData=ui->txtDataHex->toPlainText();
    if (tempData==""){return;}//如果没有内容则不保存

    QDateTime now=QDateTime::currentDateTime();
    QString name=now.toString("yyyyMMddHHmmss");
    QString fileName=name+".txt";

    QFile file(fileName);
    file.open(QFile::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out<<tempData;
    file.close();
}

void MainWindow::on_btnOpen_clicked()
{
    if (ui->btnOpen->text()=="打开串口")
    {
        QString portName=ui->cboxPortName->currentText();
#if 0
#ifdef Q_OS_WIN//如果是windows系统
        myCom = new QextSerialPort(portName);
#else
        myCom = new QextSerialPort("/dev/" + portName);
#endif
#else
       myCom = new QextSerialPort(portName);
#endif
        qDebug()<<"++++++++++++++++++++++++++++++++++++++++++++";
        qDebug()<<"Option on device: "<<myCom->portName();
        qDebug()<<"++++++++++++++++++++++++++++++++++++++++++++";
        if (myCom->open(QIODevice::ReadWrite))
        {
            //清空缓冲区
            myCom->flush();
            //设置波特率
            myCom->setBaudRate((BaudRateType)ui->cboxBaudRate->currentText().toInt());
            //设置数据位
            myCom->setDataBits((DataBitsType)ui->cboxDataBit->currentText().toInt());
            //设置校验位
            myCom->setParity((ParityType)ui->cboxParity->currentIndex());
            //设置停止位
            myCom->setStopBits((StopBitsType)ui->cboxStopBit->currentIndex());
            myCom->setFlowControl(FLOW_OFF);
            myCom->setTimeout(10);

            this->ChangeEnable(true);
            ui->btnOpen->setText("关闭串口");
            ui->labIsOpen->setText("串口状态：打开");
            this->myReadTimer->start();
        }
    }
    else
    {
        myCom->close();
        this->ChangeEnable(false);
        ui->btnOpen->setText("打开串口");
        ui->labIsOpen->setText("串口状态：关闭");
        this->myReadTimer->stop();

        //这样的话保证每次关闭串口后,自动发送和自动保存定时器不会空转
        ui->ckIsAutoSend->setChecked(false);
        ui->ckIsAutoSave->setChecked(false);
    }
}

void MainWindow::on_ckHexSend_stateChanged(int arg1)
{
    IsHexSend=(arg1==0?false:true);
}

void MainWindow::on_ckHexReceive_stateChanged(int arg1)
{
    IsHexReceive=(arg1==0?false:true);
}

void MainWindow::on_ckIsAutoSend_stateChanged(int arg1)
{    
    bool IsAutoSend=(arg1==0?false:true);
    if (IsAutoSend)
    {
        this->mySendTimer->start();
    }
    else
    {
        this->mySendTimer->stop();
    }
    ui->cboxSend->setEnabled(IsAutoSend);
}

void MainWindow::on_ckIsAutoSave_stateChanged(int arg1)
{
    bool IsAutoSave=(arg1==0?false:true);
    if (IsAutoSave)
    {
        this->mySaveTimer->start();
    }
    else
    {
        this->mySaveTimer->stop();
    }
    ui->cboxSave->setEnabled(IsAutoSave);
}

void MainWindow::on_ckIsAutoClear_stateChanged(int arg1)
{
    IsAutoClear=(arg1==0?false:true);
}

void MainWindow::on_ckIsDebug_stateChanged(int arg1)
{
    IsDebug=(arg1==0?false:true);
}

void MainWindow::on_btnClearSend_clicked()
{
    SendCount=0;
    ui->labSend->setText("发送:0 字节");
}

void MainWindow::on_btnClearReceive_clicked()
{
    ReceiveCount=0;
    ui->labReceive->setText("接收:0 字节");
}

void MainWindow::on_cboxSend_currentIndexChanged(int index)
{
    mySendTimer->setInterval((index+1)*1000);
}

void MainWindow::on_cboxSave_currentIndexChanged(int index)
{
    mySaveTimer->setInterval((index+1)*1000);
}

void MainWindow::on_btnStopShow_clicked()
{
    if (ui->btnStopShow->text()=="停止显示")
    {
        IsShow=false;
        ui->btnStopShow->setText("开始显示");
    }
    else
    {
        IsShow=true;
        ui->btnStopShow->setText("停止显示");
    }
}

void MainWindow::on_btnClearAll_clicked()
{    
    ui->txtDataHex->clear();
}

void MainWindow::on_btnSend_clicked()
{
    this->WriteMyCom();
}

void MainWindow::on_btnSave_clicked()
{
    this->SaveMyCom();
}

void MainWindow::ReadConfigData()
{    
    QString fileName="ConfigData.txt";
    QFile file(fileName);
    if (!file.exists()){return;}

    ui->txtSend->clear();
    file.open(QFile::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    QString line;

    do{ line=in.readLine();if (line!="") {ui->txtSend->addItem(line);}}
    while(!line.isNull());

    file.close();
}

void MainWindow::on_btnData_clicked()
{
    QString fileName="ConfigData.txt";
    QFile file(fileName);

    if (!file.exists()){ mHelper::ShowMessageBoxError("数据文件不存在！"); return;}

    if (ui->btnData->text()=="管理数据")
    {
        ui->txtDataHex->setReadOnly(false);
        ui->gbox2->setTitle("管理数据");
        ui->txtDataHex->clear();

        file.open(QFile::ReadOnly | QIODevice::Text);
        QTextStream in(&file);
        ui->txtDataHex->setText(in.readAll());
        file.close();

        ui->btnData->setText("保存数据");
    }
    else
    {
        ui->txtDataHex->setReadOnly(true);
        ui->gbox2->setTitle("接收数据");

        file.open(QFile::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out<<ui->txtDataHex->toPlainText();
        file.close();

        ui->txtDataHex->clear();
        ui->btnData->setText("管理数据");

        this->ReadConfigData();
    }
}
