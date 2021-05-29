#ifndef NVMLREADER_H
#define NVMLREADER_H

#include <QObject>
#include <QDebug>

#include <windows.h>

#include "nvmldefs.h"

nvmlReturn_t nvmlInitFunc();

class NvMLReader : public QObject
{
    Q_OBJECT
public:
    explicit NvMLReader(QObject *parent = nullptr);

    uint32_t getGpuLoad();
    uint16_t getGpuClock();
    void getVRamInfo(uint64_t* used, uint64_t* total);
    uint32_t getGpuTemperature();
    uint32_t getGpuPower();

    void shutdown();

private:
    bool checkAndPrintError(nvmlReturn_t ret, const char* func);

    HMODULE dllHandle = NULL;
    bool ready = false;
    nvmlDevice_t device = NULL;

    nvmlInit_t                      nvmlInit = NULL;
    nvmlShutdown_t                  nvmlShutdown = NULL;
    nvmlErrorString_t               nvmlErrorString = NULL;
    nvmlDeviceGetCount_t            nvmlDeviceGetCount = NULL;
    nvmlDeviceGetHandleByIndex_t    nvmlDeviceGetHandleByIndex = NULL;
    nvmlDeviceGetName_t             nvmlDeviceGetName = NULL;
    nvmlDeviceGetClockInfo_t        nvmlDeviceGetClockInfo = NULL;
    nvmlDeviceGetTemperature_t      nvmlDeviceGetTemperature = NULL;
    nvmlDeviceGetPowerUsage_t       nvmlDeviceGetPowerUsage = NULL;
    nvmlDeviceGetMemoryInfo_t       nvmlDeviceGetMemoryInfo = NULL;
    nvmlDeviceGetUtilizationRates_t nvmlDeviceGetUtilizationRates = NULL;
};

#endif // NVMLREADER_H
