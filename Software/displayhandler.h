#ifndef DISPLAYHANDLER_H
#define DISPLAYHANDLER_H

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QQueue>
#include <QtSerialPort/QSerialPort>
#include <cmath>

#include "perfreader.h"

#define ACK_TIMEOUT 2000

#define CMD_STARTUP 0x10
#define CMD_SHUTDOWN 0x11
#define CMD_DATA 0x20
#define CMD_CFG 0x30
#define CMD_ACK 0x99

#define CFG_DARK_MODE 0x01
#define CFG_BRIGHTNESS 0x02
#define CFG_TIMEOUT 0x03
#define CFG_HYBRID_CPU 0x04

enum class data_type_t : uint8_t {
    CPU_LOAD,
    CPU_CLOCK,
    CPU_CLOCK_SEC,
    CPU_TEMP,
    CPU_CORE_VOLTAGE,
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
    GPU_POWER,
    DATA_TYPE_END
};

#pragma pack(push,1)
typedef struct {
    data_type_t type;
    uint64_t data;
} data_point_t;
#pragma pack(pop)

typedef struct {
    uint8_t cmd;
    uint8_t length;
    uint8_t data[255];
} cmd_t;

class DisplayHandler : public QObject
{
    Q_OBJECT
public:
    explicit DisplayHandler(QObject *parent = nullptr) : QObject(parent), timer(this) {}

    void init();
    bool isConnected() { return serial.isOpen(); }
    QString getComPort() { return serial.portName(); }
    qint32 getBaudrate() { return serial.baudRate(); }
    void shutdown();

signals:
    void serialTimedOut();

public slots:
    void restartCOM();
    void perfdataReady(cpu_info_t cpuInfo, ram_info_t ramInfo, net_info_t netInfo, gpu_info_t gpuInfo);
    void setDisplayDarkMode(bool dark);
    void setDisplayBrightness(uint8_t percent);
    void setDisplayTimeout(uint16_t timeout);

private:
    void startCOM();
    void stopCOM();

    void dispatchCommand(uint8_t cmd, uint8_t* data, uint8_t size);
    void sendPacket(uint8_t cmd, uint8_t* data, uint8_t size);

    bool active = false;
    bool awaitingAck = false;
    QSerialPort serial;
    QQueue<cmd_t> cmdQueue;
    QTimer timer;
    bool hybridCpu = false;

private slots:
    void timerTimedOut();
    void serialReadyRead();
};

#endif // DISPLAYHANDLER_H
