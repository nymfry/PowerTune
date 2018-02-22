/*
* Copyright (C) 2017 Bastian Gschrey & Markus Ippy
*
* Digital Gauges for Apexi Power FC for RX7 on Raspberry Pi
*
*
* This software comes under the GPL (GNU Public License)
* You may freely copy,distribute etc. this as long as the source code
* is made available for FREE.
*
* No warranty is made or implied. You use this program at your own risk.
*/

/*!
  \file Connect.cpp
  \brief Raspexi Viewer Power FC related functions
  \author Bastian Gschrey & Markus Ippy
*/

#include "datalogger.h"
#include "connect.h"
#include "sensors.h"
#include "udpreceiver.h"
#include "adaptronicselect.h"
#include "AdaptronicCAN.h"
#include "Apexi.h"
#include "HaltechCAN.h"
#include "Nissanconsult.h"
#include "obd.h"
#include "AdaptronicCAN.h"
#include "HaltechCAN.h"
#include "Apexi.h"
#include "AdaptronicSelect.h"
#include "dashboard.h"
#include "Serialport.h"
#include "appsettings.h"
#include "gopro.h"
#include "gps.h"
#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QQmlContext>
#include <QQmlApplicationEngine>
#include <QFile>
#include <QTextStream>
#include <QByteArrayMatcher>
#include <QProcess>







int ecu =3; //0=apex, 1=adaptronic;2= OBD; 3= Dicktator ECU
int logging; // 0 Logging off , 1 Logging to file
int connectclicked =0;
QByteArray checksumhex;
QByteArray recvchecksumhex;


Connect::Connect(QObject *parent) :
    QObject(parent),
    m_serialport(Q_NULLPTR),
    m_dashBoard(Q_NULLPTR),
    m_gopro(Q_NULLPTR),
    m_gps(Q_NULLPTR),
    m_adaptronicselect(Q_NULLPTR),
    m_apexi(Q_NULLPTR),
    m_nissanconsult(Q_NULLPTR),
    m_OBD(Q_NULLPTR),
    m_sensors(Q_NULLPTR),
    m_haltechCANV2(Q_NULLPTR),
    m_adaptronicCAN(Q_NULLPTR),
    m_udpreceiver(Q_NULLPTR),
    m_datalogger(Q_NULLPTR)

{

    getPorts();
    m_dashBoard = new DashBoard(this);
   // m_decoder = new Decoder(m_dashBoard, this);
    m_appSettings = new AppSettings(this);
    m_gopro = new GoPro(this);
    m_gps = new GPS(m_dashBoard, this);
    m_adaptronicselect= new AdaptronicSelect(m_dashBoard, this);
    m_apexi= new Apexi(m_dashBoard, this);
    ;m_nissanconsult = new Nissanconsult(m_dashBoard, this);
    m_OBD = new OBD(m_dashBoard, this);
    m_sensors = new Sensors(m_dashBoard, this);
    m_haltechCANV2 = new HaltechCAN(m_dashBoard, this);
    m_adaptronicCAN = new AdaptronicCAN(m_dashBoard, this);
    m_udpreceiver = new udpreceiver(m_dashBoard, this);
    m_datalogger = new datalogger(m_dashBoard, this);
    QQmlApplicationEngine *engine = dynamic_cast<QQmlApplicationEngine*>( parent );
    if (engine == Q_NULLPTR)
        return;
    engine->rootContext()->setContextProperty("Dashboard", m_dashBoard);
    engine->rootContext()->setContextProperty("AppSettings", m_appSettings);
    engine->rootContext()->setContextProperty("GoPro", m_gopro);
    engine->rootContext()->setContextProperty("GPS", m_gps);
    engine->rootContext()->setContextProperty("Nissanconsult",m_nissanconsult);
    engine->rootContext()->setContextProperty("Sens", m_sensors);
    engine->rootContext()->setContextProperty("Logger", m_datalogger);
}



Connect::~Connect()
{


}
/*
void Connect::initConnectPort()
{
    if (m_Connectport)
        delete m_Connectport;
    m_Connectport = new ConnectPort(this);
    connect(this->m_Connectport,SIGNAL(readyRead()),this,SLOT(readyToRead()));
 //   connect(m_Connectport, static_cast<void (QConnectPort::*)(QConnectPort::ConnectPortError)>(&QConnectPort::error),
//            this, &Connect::handleError);

    m_readData.clear();
    //m_timer.start(5000);


}
*/



/*void Connect::setEcus(QStringList ECUList)
{

}*/

void Connect::getPorts()
{
    QStringList PortList;
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        PortList.append(info.portName());
    }
    setPortsNames(PortList);
    // Check available ports evry 1000 ms
    QTimer::singleShot(1000, this, SLOT(getPorts()));
}
//function for flushing all Connect buffers
void Connect::clear() const
{
   // m_Connectport->clear();
}


//function to open Connect port
void Connect::openConnection(const QString &portName, const int &ecuSelect)
{

    ecu = ecuSelect;

    //Apexi
    if (ecuSelect == 0)
    {

        m_apexi->openConnection(portName);

    }


    //Adaptronic
    if (ecuSelect == 1)
    {
         m_adaptronicselect->openConnection(portName);

    }
    //OBD
    if (ecuSelect == 2)
    {
       m_OBD->openConnection(portName);
    }
    //Nissan Consult
    if (ecuSelect == 3)
    {
        ;m_nissanconsult->LiveReqMsg(1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
        ;m_nissanconsult->openConnection(portName);

    }
    //UDP reveiver
    if (ecuSelect == 4)
    {
        if (connectclicked == 0)
        {
        m_udpreceiver->startreceiver();
        connectclicked = 1;
        }

    }
    //Adaptronic ModularCAN protocol
    if (ecuSelect == 5)
    {
        if (connectclicked == 0)
        {
        m_adaptronicCAN->openCAN();
        connectclicked = 1;
        }

    }
    //Haltech V2 CAN protocol
    if (ecuSelect == 6)
    {
        if (connectclicked == 0)
        {
        m_haltechCANV2->openCAN();
        connectclicked = 1;
        }

    }


   /* //Dicktator
    if (ecuSelect == 9)
    {

        initConnectPort();
        m_Connectport->setPortName(portName);
        m_Connectport->setBaudRate(QConnectPort::Baud19200);
        m_Connectport->setParity(QConnectPort::NoParity);
        m_Connectport->setDataBits(QConnectPort::Data8);
        m_Connectport->setStopBits(QConnectPort::OneStop);
        m_Connectport->setFlowControl(QConnectPort::NoFlowControl);

        if(m_Connectport->open(QIODevice::ReadWrite) == false)
        {
            m_dashBoard->setConnectStat(m_Connectport->errorString());
        }
        else
        {
            m_dashBoard->setConnectStat(QString("Connected to Connectport"));
        }
    }*/

}
void Connect::closeConnection()
{

    if(ecu == 0){
    //    m_Connectport->close();
    }
    if(ecu == 1){
        //modbusDevice->disconnectDevice();
    }
    if(ecu == 9){
     //   m_Connectport->close();
    }
}

void Connect::update()
{

/*
    bool bStatus = false;

    QStringList args;
    qint64      pid = 0;

    args << "&";
    bStatus = QProcess::startDetached("/home/pi/update.sh", args, ".", &pid);
*/
}
/*
void Connect::handleError(QConnectPort::ConnectPortError ConnectPortError)
{
    if (ConnectPortError == QConnectPort::ReadError) {
        QString fileName = "Errors.txt";
        QFile mFile(fileName);
        if(!mFile.open(QFile::Append | QFile::Text)){
        }
        QTextStream out(&mFile);
        out << "Connect Error " << (m_Connectport->errorString()) <<endl;
        mFile.close();
        m_dashBoard->setConnectStat(m_Connectport->errorString());

    }
}

*/
/*
void Connect::readyToRead()
{

    if(ecu == 1)
    {


    }
    /*
    if(ecu == 9) //Dicktator ECU
    {
        m_readData = m_Connectport->readAll();
        Connect::dicktatorECU(m_readData);
        m_readData.clear();
    }

}

void Connect::dicktatorECU(const QByteArray &buffer)
{
    //Appending the message until the patterns Start and End Are found , then removing all bytes before and after the message
    m_buffer.append(buffer);
    QByteArray startpattern("START");
    QByteArrayMatcher startmatcher(startpattern);
    QByteArray endpattern("END");
    QByteArrayMatcher endmatcher(endpattern);
    int pos = 0;
    while((pos = startmatcher.indexIn(m_buffer, pos)) != -1)
    {

        if (pos !=0)
        {
            m_buffer.remove(0, pos);
        }
        if (pos == 0 ) break;
    }
    int pos2 = 0;
    while((pos2 = endmatcher.indexIn(m_buffer, pos2)) != -1)
    {


    if (pos2 > 30)
        {
            m_buffer.remove(0,pos2-30);
        }

        if (pos2 == 30 )
        {
            m_dicktatorMsg = m_buffer;
            m_buffer.clear();
            m_decoder->decodeDicktator(m_dicktatorMsg);
            break;
        }
    }

}
*/
