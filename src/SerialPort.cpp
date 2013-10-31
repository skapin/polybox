#include "SerialPort.h"
#include <QDebug>

SerialPort::SerialPort(QObject *parent) :
    QObject(parent)
  //QextSerialPort("/dev/ttyACM0", QextSerialPort::EventDriven, parent)
{
    _port = NULL;
    _printerCOM = NULL;
    startVirtualCOMProcess();
}

bool SerialPort::connectToSerialPort()
{
    if ( _port != NULL && _port->isOpen() )
    {
        _port->close();
    }
    _port = new QextSerialPort(Config::pathToSerialDevice+Config::serialPortName, QextSerialPort::EventDriven);


    _port->setBaudRate( (BaudRateType)(Config::motherboardBaudrate) );
    _port->setFlowControl(FLOW_OFF);
    _port->setParity(PAR_NONE);
    _port->setDataBits(DATA_8);
    _port->setStopBits(STOP_1);

    if ( _port->open(QIODevice::ReadWrite) == true)
    {
        //connect(this, SIGNAL(readyRead()), this, SLOT(onReadyRead()) );
        //      connect(this, SIGNAL(dsrChanged(bool)), this, SLOT(onDsrChanged(bool)) );
     /*   if (!(lineStatus() & LS_DSR)){
            qDebug() << "warning: device is not turned on"<<lineStatus();
            return false;
        }*/
        qDebug() << "listening for data on" << _port->portName();
        qDebug() << "Ok connected";
        connect ( _port, SIGNAL(readyRead()), this, SLOT(parseSerialDatas()) );
        _port->write("totot\n");
        return true;
    }
    else
    {
        qDebug() << "device failed to open:" << _port->errorString();
        return false;
    }

}

void SerialPort::parseSerialDatas()
{
    QByteArray bytes;
    int a = _port->bytesAvailable();
    bytes.resize(a);
    _port->read(bytes.data(), bytes.size());
    qDebug() << "#"<<bytes.size() <<"bytes=" << bytes.data();
}


void SerialPort::sendMCode(int code)
{
    sendMCode( QString::number(code) );
}
void SerialPort::sendMCode(QString code)
{
    code = "M"+code;
    if ( _port->isWritable() && _port->isOpen() )
    {
        _port->write( code.toStdString().c_str() );
    }

}
