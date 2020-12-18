#ifndef DISPLAYHANDLER_H
#define DISPLAYHANDLER_H

#include <QObject>
#include <QSettings>
#include <QErrorMessage>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <cmath>

#include "perfreader.h"
#include "dialogoptions.h"

#define CMD_STARTUP 0x10
#define CMD_SHUTDOWN 0x11
#define CMD_DATA 0x20
#define CMD_CFG 0x30
#define CMD_ACK 0x99

enum class data_type_t {
    CPU_LOAD,
    CPU_CLOCK,
    CPU_TEMP,
    CPU_POWER,
    RAM_LOAD,
    RAM_TOTAL,
    RAM_USED,
    NET_BANDWIDTH,
    NET_IN,
    NET_OUT,
    GPU_LOAD,
    GPU_CLOCK,
    GPU_TEMP,
    GPU_VRAM_LOAD,
    GPU_VRAM_TOTAL,
    GPU_VRAM_USED,
    GPU_POWER
};

#pragma pack(push,1)
typedef struct {
    data_type_t type;
    uint64_t data;
} data_point_t;
#pragma pack(pop)

class DisplayHandler : public QObject
{
    Q_OBJECT
public:
    explicit DisplayHandler(QObject *parent = nullptr);
    ~DisplayHandler();

    void sendPerformanceData(cpu_info_t& cpuInfo, ram_info_t& ramInfo, net_info_t& netInfo, gpu_info_t& gpuInfo);
    void openSettingsDialog();

    bool isConnected() { return serial != nullptr && serial->isOpen(); }
    QString getComPort() { return serial != nullptr ? serial->portName() : "[Invalid]"; }
    qint32 getBaudrate() { return serial != nullptr ? serial->baudRate() : -1; }
    void shutdown();

signals:

private:
    void sendPacket(uint8_t cmd, uint8_t* data, uint8_t size);

    QSerialPort* serial = nullptr;
    QErrorMessage* errorMsg;
    QTimer* timer;

private slots:
    void checkPortAck();
};

#endif // DISPLAYHANDLER_H
