#include "ohmwrapper.h"

void OHMWrapper::init()
{
    ohm = new QProcess();
    ohm->setProgram("ohm_reader.exe");
    connect(ohm, &QProcess::readyReadStandardError, this, &OHMWrapper::stderrReadyRead);

    ohm->start(QIODevice::ReadWrite | QIODevice::Unbuffered);
    ready = ohm->waitForStarted();
    if (!ready) {
        qWarning("[OHMWrapper] Failed to start ohm_reader.exe!");
        return;
    }

    ohm->waitForReadyRead();
    QString result = QString::fromUtf8(ohm->read(64));
    ready = result.startsWith("ready");
    if (!ready) {
        qWarning("[OHMWrapper] ohm_reader.exe encountered an error while starting!");
    }
    else {
        qInfo("[OHMWrapper] ohm_reader.exe started succesfully!");
    }
}

void OHMWrapper::update()
{
    if (!ready) { return; }

    ohm->write("update\r\n");
    ohm->waitForBytesWritten();

    ohm->waitForReadyRead();
    ohm->read(64); //Clear buffer
}

std::pair<uint16_t, uint16_t> OHMWrapper::getCpuClockAvg()
{
    if (!ready) { return std::pair<uint16_t, uint16_t>(0, 0); }

    ohm->write("cpu_clock_avg\r\n");
    ohm->waitForBytesWritten();

    ohm->waitForReadyRead();
    QStringList data = QString::fromUtf8(ohm->read(64)).split(",");
    return std::pair<uint16_t, uint16_t>(data[0].toUInt(), data.size() > 1 ? data[1].toUInt() : 0);
}

uint16_t OHMWrapper::getCpuPkgTemp()
{
    if (!ready) { return 0; }

    ohm->write("cpu_temp\r\n");
    ohm->waitForBytesWritten();

    ohm->waitForReadyRead();
    QByteArray data = ohm->read(64);
    return QString::fromUtf8(data).toUInt();
}

uint32_t OHMWrapper::getCpuPkgPower()
{
    if (!ready) { return 0; }

    ohm->write("cpu_power\r\n");
    ohm->waitForBytesWritten();

    ohm->waitForReadyRead();
    QByteArray data = ohm->read(64);
    return QString::fromUtf8(data).toUInt();
}

uint32_t OHMWrapper::getCpuCoreVoltage()
{
    if (!ready) { return 0; }

    ohm->write("cpu_core_voltage\r\n");
    ohm->waitForBytesWritten();

    ohm->waitForReadyRead();
    QByteArray data = ohm->read(64);
    return QString::fromUtf8(data).toUInt();
}

void OHMWrapper::shutdown()
{
    if (ready) {
        ohm->write("shutdown\n");
        ohm->closeWriteChannel();
        ohm->waitForFinished(1000);
        delete ohm;
    }
}



void OHMWrapper::stderrReadyRead()
{
    qWarning("[OHMReader] %s", ohm->readAllStandardError().toStdString().c_str());
}
