#include "ohmwrapper.h"

OHMWrapper::OHMWrapper()
{
    ohm = new QProcess();
    //ohm->setProgram("ohm_reader.exe");
    //ohm->setProgram("python");
    //ohm->setArguments({"ohm_reader.py"});
    ohm->setProgram("java");
    ohm->setArguments({"Main"});
    ohm->setProcessChannelMode(QProcess::ForwardedErrorChannel);
    ohm->start(QIODevice::ReadWrite | QIODevice::Unbuffered);
    ready = ohm->waitForStarted();
    if (!ready) {
        std::cout << "Failed to start ohm_reader.exe!" << std::endl;
        return;
    }

    ohm->waitForReadyRead();
    QString result = QString::fromUtf8(ohm->read(64));
    ready = result.startsWith("ready");
    if (!ready) {
        std::cout << "ohm_reader.exe encountered an error while starting!" << std::endl;
    }
    else {
        std::cout << "ohm_reader.exe started succesfully!" << std::endl;
    }
}

uint16_t OHMWrapper::getCpuClockAvg()
{
    if (!ready) { return 0; }

    ohm->write("cpu_clock_avg\r\n");
    ohm->waitForBytesWritten();

    ohm->waitForReadyRead();
    QByteArray data = ohm->read(64);
    return QString::fromUtf8(data).toUInt();
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

void OHMWrapper::shutdown()
{
    if (ready) {
        ohm->write("shutdown\n");
        ohm->closeWriteChannel();
        ohm->waitForFinished(1000);
    }
}
