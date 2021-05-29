#include "nvmlreader.h"

NvMLReader::NvMLReader(QObject* parent) : QObject(parent)
{
    //Get nvml.dll from NVSMI folder
    const char* dllName = "nvml.dll"; //The nvsmi path (C:/Program Files/NVIDIA Corporation/NVSMI) must be on the system PATH
    dllHandle = LoadLibraryA(dllName);
    if (dllHandle == NULL) { qCritical() << "Couldn't find nvml.dll"; return; }

    //Retrieve function pointers
    nvmlInit =                      (nvmlInit_t)                        GetProcAddress(dllHandle, "nvmlInit_v2");
    nvmlShutdown =                  (nvmlShutdown_t)                    GetProcAddress(dllHandle, "nvmlShutdown");
    nvmlErrorString =               (nvmlErrorString_t)                 GetProcAddress(dllHandle, "nvmlErrorString");
    nvmlDeviceGetCount =            (nvmlDeviceGetCount_t)              GetProcAddress(dllHandle, "nvmlDeviceGetCount_v2");
    nvmlDeviceGetHandleByIndex =    (nvmlDeviceGetHandleByIndex_t)      GetProcAddress(dllHandle, "nvmlDeviceGetHandleByIndex");
    nvmlDeviceGetName =             (nvmlDeviceGetName_t)               GetProcAddress(dllHandle, "nvmlDeviceGetName");
    nvmlDeviceGetClockInfo =        (nvmlDeviceGetClockInfo_t)          GetProcAddress(dllHandle, "nvmlDeviceGetClockInfo");
    nvmlDeviceGetTemperature =      (nvmlDeviceGetTemperature_t)        GetProcAddress(dllHandle, "nvmlDeviceGetTemperature");
    nvmlDeviceGetPowerUsage =       (nvmlDeviceGetPowerUsage_t)         GetProcAddress(dllHandle, "nvmlDeviceGetPowerUsage");
    nvmlDeviceGetMemoryInfo =       (nvmlDeviceGetMemoryInfo_t)         GetProcAddress(dllHandle, "nvmlDeviceGetMemoryInfo");
    nvmlDeviceGetUtilizationRates = (nvmlDeviceGetUtilizationRates_t)   GetProcAddress(dllHandle, "nvmlDeviceGetUtilizationRates");

    //Validate function pointers
    if (nvmlInit == NULL ||
        nvmlShutdown == NULL ||
        nvmlErrorString == NULL ||
        nvmlDeviceGetCount == NULL ||
        nvmlDeviceGetHandleByIndex == NULL ||
        nvmlDeviceGetName == NULL ||
        nvmlDeviceGetClockInfo == NULL ||
        nvmlDeviceGetTemperature == NULL ||
        nvmlDeviceGetPowerUsage == NULL ||
        nvmlDeviceGetMemoryInfo == NULL ||
        nvmlDeviceGetUtilizationRates == NULL
    ) {
        qCritical("Failed to retrieve one or more functions from nvml.dll!");
        return;
    }

    //Init NVML
    nvmlReturn_t ret = nvmlInit();
    if (checkAndPrintError(ret, "nvmlInit")) { return; }

    //Check device count
    unsigned int count = 0;
    ret = nvmlDeviceGetCount(&count);
    if (checkAndPrintError(ret, "nvmlDeviceGetCount")) { return; }

    if (count == 0) {
        qCritical() << "No NVIDIA GPU found!";
        return;
    }
    else if (count > 1) {
        qInfo() << "Found multiple NVIDIA GPUs, only the first one will be used!";
    }

    //Retrieve device handle
    ret = nvmlDeviceGetHandleByIndex(0, &device);
    if (checkAndPrintError(ret, "nvmlDeviceGetHandleByIndex")) { return; }

    //Retrieve device name
    char buffer[NVML_DEVICE_NAME_BUFFER_SIZE];
    ret = nvmlDeviceGetName(device, buffer, NVML_DEVICE_NAME_BUFFER_SIZE);
    if (!checkAndPrintError(ret, "nvmlDeviceGetName")) {
        qInfo().nospace() << "Found NVIDIA GPU with name: " << buffer;
    }

    //Ready
    ready = true;
    qInfo("NVML reader initialized");
}

void NvMLReader::shutdown()
{
    if (ready) {
        ready = false;
        nvmlReturn_t ret = nvmlShutdown();
        checkAndPrintError(ret, "nvmlShutdown");
    }

    if (dllHandle != NULL) {
        FreeLibrary(dllHandle);
    }
}



uint32_t NvMLReader::getGpuLoad()
{
    nvmlUtilization_t util;
    nvmlReturn_t ret = nvmlDeviceGetUtilizationRates(device, &util);
    if (checkAndPrintError(ret, "nvmlDeviceGetUtilizationRates")) { return 0; }
    return util.gpu;
}

uint16_t NvMLReader::getGpuClock()
{
    unsigned int clock;
    nvmlReturn_t ret = nvmlDeviceGetClockInfo(device, NVML_CLOCK_GRAPHICS, &clock);
    if (checkAndPrintError(ret, "nvmlDeviceGetClockInfo")) { return 0; }
    return clock;
}

void NvMLReader::getVRamInfo(uint64_t *used, uint64_t *total)
{
    nvmlMemory_t mem;
    nvmlReturn_t ret = nvmlDeviceGetMemoryInfo(device, &mem);
    bool error = checkAndPrintError(ret, "nvmlDeviceGetMemoryInfo");
    *used = error ? 0 : mem.used;
    *total = error ? 0 : mem.total;
}

uint32_t NvMLReader::getGpuTemperature()
{
    unsigned int temp = 0;
    nvmlReturn_t ret = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temp);
    if (checkAndPrintError(ret, "nvmlDeviceGetTemperature")) { return 0; }
    return temp;
}

uint32_t NvMLReader::getGpuPower()
{
    unsigned int power = 0;
    nvmlReturn_t ret = nvmlDeviceGetPowerUsage(device, &power);
    if (checkAndPrintError(ret, "nvmlDeviceGetPowerUsage")) { return 0; }
    return power;
}



bool NvMLReader::checkAndPrintError(nvmlReturn_t ret, const char* func)
{
    if (ret != NVML_SUCCESS) {
        qWarning() << "Error in NVML function call (" << func << "): " << nvmlErrorString(ret);
    }
    return ret != NVML_SUCCESS;
}
