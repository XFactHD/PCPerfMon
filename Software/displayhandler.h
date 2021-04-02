#ifndef DISPLAYHANDLER_H
#define DISPLAYHANDLER_H

#include <QObject>
#include <QSettings>
#include <QTimer>
#include <QQueue>
#include <QtSerialPort/QSerialPort>
#include <cmath>

#include "perfreader.h"

#define ACK_TIMEOUT 400

#define CMD_STARTUP 0x10
#define CMD_SHUTDOWN 0x11
#define CMD_DATA 0x20
#define CMD_CFG 0x30
#define CMD_ACK 0x99

#define CFG_DARK_MODE 0x01

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

typedef struct {
    uint8_t cmd;
    uint8_t length;
    uint8_t data[255];
} cmd_t;

class DisplayHandler : public QObject
{
    Q_OBJECT
public:
    explicit DisplayHandler(QObject *parent = nullptr);
    ~DisplayHandler();

    bool isConnected() { return serial != nullptr && serial->isOpen(); }
    QString getComPort() { return serial != nullptr ? serial->portName() : "[Invalid]"; }
    qint32 getBaudrate() { return serial != nullptr ? serial->baudRate() : -1; }
    void shutdown();

public slots:
    void restartCOM();
    void on_perfdata_ready(cpu_info_t cpuInfo, ram_info_t ramInfo, net_info_t netInfo, gpu_info_t gpuInfo);
    void on_settings_setDisplayDarkMode(bool dark);

private:
    void startCOM();
    void stopCOM();

    void dispatchCommand(uint8_t cmd, uint8_t* data, uint8_t size);
    void sendPacket(uint8_t cmd, uint8_t* data, uint8_t size);

    bool active = false;
    bool awaitingAck = false;
    QSerialPort* serial = nullptr;
    QQueue<cmd_t> cmdQueue;
    QTimer* timer;

private slots:
    void on_timer_timeout();
    void on_serial_readyRead();
};

#endif // DISPLAYHANDLER_H
