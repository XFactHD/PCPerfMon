#include "displayhandler.h"

void DisplayHandler::init()
{
    QSettings settings;
    if (!settings.contains("com_port")) {
        settings.setValue("com_port", "COM1");
    }
    if (!settings.contains("baudrate")) {
        settings.setValue("baudrate", 1000000);
    }

    active = settings.value("drive_display", true).toBool();

    if(!active) { return; }

    connect(&timer, &QTimer::timeout, this, &DisplayHandler::timerTimedOut);

    startCOM();
}

void DisplayHandler::perfdataReady(cpu_info_t cpuInfo, ram_info_t ramInfo, net_info_t netInfo, gpu_info_t gpuInfo)
{
    if (!active || !isConnected()) { return; }

    // Secondary CPU clock higher than 0 indicates a hybrid CPU
    if (hybridCpu != (cpuInfo.cpuClockSecondary > 0)) {
        hybridCpu = cpuInfo.cpuClockSecondary > 0;

        uint8_t data[] = { CFG_HYBRID_CPU, hybridCpu };
        sendPacket(CMD_CFG, data, 2);
    }

    data_point_t data[(uint8_t)data_type_t::DATA_TYPE_END];
    size_t idx = 0;

    data[idx++] = { data_type_t::CPU_LOAD, (uint64_t)std::round(cpuInfo.cpuLoad * 100.0f)};
    data[idx++] = { data_type_t::CPU_CLOCK, cpuInfo.cpuClockMain };
    if (hybridCpu) { data[idx++] = { data_type_t::CPU_CLOCK_SEC, cpuInfo.cpuClockSecondary }; }
    data[idx++] = { data_type_t::CPU_TEMP, (uint64_t)cpuInfo.cpuTemp };
    data[idx++] = { data_type_t::CPU_POWER, cpuInfo.cpuPower };
    data[idx++] = { data_type_t::RAM_LOAD, (uint64_t)ramInfo.ramLoad };
    data[idx++] = { data_type_t::RAM_TOTAL, ramInfo.ramTotal };
    data[idx++] = { data_type_t::RAM_USED, ramInfo.ramUsed };
    data[idx++] = { data_type_t::NET_IN, (uint64_t)(netInfo.netIn * 100.0) };
    data[idx++] = { data_type_t::NET_OUT, (uint64_t)(netInfo.netOut * 100.0) };
    data[idx++] = { data_type_t::GPU_LOAD, (uint64_t)gpuInfo.gpuLoad };
    data[idx++] = { data_type_t::GPU_CLOCK, gpuInfo.gpuClock };
    data[idx++] = { data_type_t::GPU_TEMP, gpuInfo.gpuTemp };
    data[idx++] = { data_type_t::GPU_VRAM_LOAD, (uint64_t)gpuInfo.vramLoad };
    data[idx++] = { data_type_t::GPU_VRAM_TOTAL, gpuInfo.vramTotal };
    data[idx++] = { data_type_t::GPU_VRAM_USED, gpuInfo.vramUsed };
    data[idx++] = { data_type_t::GPU_POWER, gpuInfo.gpuPower };

    sendPacket(CMD_DATA, (uint8_t*)data, sizeof(data));
}

void DisplayHandler::shutdown()
{
    if (active) {
        stopCOM();
    }
}

void DisplayHandler::restartCOM()
{
    if(!active) { return; }

    stopCOM();
    startCOM();
}

void DisplayHandler::setDisplayDarkMode(bool dark)
{
    if (!active || !isConnected()) { return; }

    uint8_t data[] = { CFG_DARK_MODE, dark };
    sendPacket(CMD_CFG, data, 2);
}

void DisplayHandler::setDisplayBrightness(uint8_t percent) {
    if (!active || !isConnected()) { return; }

    uint8_t data[] = { CFG_BRIGHTNESS, percent };
    sendPacket(CMD_CFG, data, 2);
}

void DisplayHandler::setDisplayTimeout(uint16_t timeout) {
    if (!active || !isConnected()) { return; }

    uint8_t data[] = { CFG_TIMEOUT, (uint8_t)((timeout >> 8) & 0xFF), (uint8_t)(timeout & 0xFF) };
    sendPacket(CMD_CFG, data, 3);
}



void DisplayHandler::startCOM()
{
    QSettings settings;
    QString comPort = settings.value("com_port", "COM1").toString();
    quint32 baudrate = settings.value("baudrate", 115200).toUInt();

    serial.setParent(this);
    serial.setPortName(comPort);
    serial.setBaudRate(baudrate);

    if (!connect(&serial, &QSerialPort::readyRead, this, &DisplayHandler::serialReadyRead)) {
        qWarning("[DisplayHandler] Failed to connect slot to QSerialPort::readyRead!");
    }

    if (!serial.open(QIODevice::ReadWrite)) {
        qWarning().nospace() << "[DisplayHandler] An error occured while opening COM port!" << " [Port: " << comPort << ", Baudrate: " << baudrate << ", Error: " << serial.error() << "]";
    }
    else {
        serial.setDataTerminalReady(true); //Necessary to make the Adafruit Feather M0+ correctly detect a working connection
        sendPacket(CMD_STARTUP, nullptr, 0);
        setDisplayDarkMode(settings.value("display_dark_mode", false).toBool());
        setDisplayTimeout(settings.value("display_timeout", ACK_TIMEOUT).toUInt());
    }
}

void DisplayHandler::stopCOM()
{
    if (isConnected()) {
        QEventLoop loop;
        while (!cmdQueue.isEmpty() || awaitingAck) {
            loop.processEvents();
        }

        sendPacket(CMD_SHUTDOWN, nullptr, 0);
        serial.flush();
        serial.waitForBytesWritten();
        serial.close();
    }

    disconnect(&serial, &QSerialPort::readyRead, this, &DisplayHandler::serialReadyRead);
}

void DisplayHandler::dispatchCommand(uint8_t cmd, uint8_t *data, uint8_t size)
{
    if (awaitingAck) {
        cmd_t cmdT = { cmd, size, {0} };
        if (size) { memcpy(cmdT.data, data, size); }
        cmdQueue.enqueue(cmdT);
    }
    else {
        sendPacket(cmd, data, size);
    }
}

void DisplayHandler::sendPacket(uint8_t cmd, uint8_t* data, uint8_t size)
{
    const uint8_t cmdPtr[2] = { cmd, size };
    serial.write((char*)cmdPtr, 2);

    if (data != nullptr && size > 0) {
        serial.write((char*)data, size);
    }

    awaitingAck = true;
    timer.start(ACK_TIMEOUT);
}

void DisplayHandler::serialReadyRead()
{
    QByteArray data = serial.readAll();
    if (((uint8_t)data.at(0)) == CMD_ACK) {
        if (!awaitingAck) {
            qWarning("[DisplayHandler] Received erronous ACK!");
        }
        awaitingAck = false;

        if (!cmdQueue.isEmpty()) {
            cmd_t cmd = cmdQueue.dequeue();
            sendPacket(cmd.cmd, cmd.data, cmd.length);
        }
    }
}

void DisplayHandler::timerTimedOut()
{
    if (awaitingAck) {
        awaitingAck = false;
        qWarning("[DisplayHandler] ACK timed out, closing connection!");
        serial.close();
        timer.stop();

        emit serialTimedOut();
    }
}
