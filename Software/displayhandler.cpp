#include "displayhandler.h"

DisplayHandler::DisplayHandler(QObject *parent) : QObject(parent)
{
    errorMsg = new QErrorMessage((QWidget*)parent);

    QSettings settings;
    if (!settings.contains("com_port") || !settings.contains("baudrate")) {
        openSettingsDialog();
    }

    active = settings.value("drive_display", true).toBool();

    if(!active) { return; }

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DisplayHandler::checkPortAck);

    startCOM();
}

void DisplayHandler::sendPerformanceData(cpu_info_t &cpuInfo, ram_info_t &ramInfo, net_info_t &netInfo, gpu_info_t &gpuInfo)
{
    if (!active || !isConnected()) { return; }

    data_point_t data[17];
    data[0] =  { data_type_t::CPU_LOAD, (uint64_t)std::round(cpuInfo.cpuLoad * 100.0f)};
    data[1] =  { data_type_t::CPU_CLOCK, cpuInfo.cpuClock };
    data[2] =  { data_type_t::CPU_TEMP, (uint64_t)cpuInfo.cpuTemp };
    data[3] =  { data_type_t::CPU_POWER, cpuInfo.cpuPower };
    data[4] =  { data_type_t::RAM_LOAD, (uint64_t)ramInfo.ramLoad };
    data[5] =  { data_type_t::RAM_TOTAL, ramInfo.ramTotal };
    data[6] =  { data_type_t::RAM_USED, ramInfo.ramUsed };
    data[7] =  { data_type_t::NET_BANDWIDTH, (uint64_t)(netInfo.bandwidth * 100.0) };
    data[8] =  { data_type_t::NET_IN, (uint64_t)(netInfo.netIn * 100.0) };
    data[9] =  { data_type_t::NET_OUT, (uint64_t)(netInfo.netOut * 100.0) };
    data[10] = { data_type_t::GPU_LOAD, (uint64_t)gpuInfo.gpuLoad };
    data[11] = { data_type_t::GPU_CLOCK, gpuInfo.gpuClock };
    data[12] = { data_type_t::GPU_TEMP, gpuInfo.gpuTemp };
    data[13] = { data_type_t::GPU_VRAM_LOAD, (uint64_t)gpuInfo.vramLoad };
    data[14] = { data_type_t::GPU_VRAM_TOTAL, gpuInfo.vramTotal };
    data[15] = { data_type_t::GPU_VRAM_USED, gpuInfo.vramUsed };
    data[16] = { data_type_t::GPU_POWER, gpuInfo.gpuPower };

    sendPacket(CMD_DATA, (uint8_t*)data, sizeof(data));
}

DisplayHandler::~DisplayHandler()
{
    delete errorMsg;
    if(active) {
        delete serial;
        delete timer;
    }
}

void DisplayHandler::openSettingsDialog()
{
    DialogOptions options((QWidget*)parent());
    options.exec();
}

void DisplayHandler::restartCOM()
{
    if(!active) { return; }

    stopCOM();
    delete serial;
    startCOM();
}

void DisplayHandler::shutdown()
{
    stopCOM();
}



void DisplayHandler::startCOM()
{
    QSettings settings;
    QString comPort = settings.value("com_port", "COM1").toString();
    quint32 baudrate = settings.value("baudrate", 115200).toUInt();

    serial = new QSerialPort(comPort, this);
    serial->setBaudRate(baudrate);
    if (!serial->open(QIODevice::ReadWrite)) {
        QString msg = "An error occured while opening COM port!\n";
        msg += "Port: " + comPort + "\nBaudrate: " + QString::number(baudrate) + "\n";
        msg += "Error code: " + QString::number(serial->error());
        //errorMsg->showMessage(msg);
        qWarning("%s", msg.toStdString().c_str());
    }
    else {
        sendPacket(CMD_STARTUP, nullptr, 0);
    }
}

void DisplayHandler::stopCOM()
{
    if (active && isConnected()) {
        sendPacket(CMD_SHUTDOWN, nullptr, 0);
        serial->flush();
        serial->waitForBytesWritten();
    }
}

void DisplayHandler::sendPacket(uint8_t cmd, uint8_t* data, uint8_t size)
{
    //Make sure the MCU sent an ACK for the last packet before sending another one
    if (timer->isActive()) {
        checkPortAck();
        if (!isConnected()) {
            return;
        }
    }

    const uint8_t cmdPtr[2] = { cmd, size };
    serial->write((char*)cmdPtr, 2);

    if (data != nullptr && size > 0) {
        serial->write((char*)data, size);
    }

    timer->start(200);
}

void DisplayHandler::checkPortAck()
{
    timer->stop();

    bool acked = false;
    if (serial->bytesAvailable() > 0) {
        QByteArray data = serial->readAll();
        if (((uint8_t)data.at(0)) == CMD_ACK) {
            acked = true;
        }
    }

    if (!acked) {
        serial->close();
    }
}
