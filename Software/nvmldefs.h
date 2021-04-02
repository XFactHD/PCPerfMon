#ifndef NVMLDEFS_H
#define NVMLDEFS_H

#include "nvml.h"

/*
 Typedefs of needed functions declared in nvml.h for use with GetProcAdress()
*/

typedef nvmlReturn_t (*nvmlInit_t)(void);
typedef nvmlReturn_t (*nvmlShutdown_t)(void);
typedef const char*  (*nvmlErrorString_t)(nvmlReturn_t result);
typedef nvmlReturn_t (*nvmlDeviceGetCount_t)(unsigned int *deviceCount);
typedef nvmlReturn_t (*nvmlDeviceGetHandleByIndex_t)(unsigned int index, nvmlDevice_t *device);
typedef nvmlReturn_t (*nvmlDeviceGetName_t)(nvmlDevice_t device, char *name, unsigned int length);
typedef nvmlReturn_t (*nvmlDeviceGetClockInfo_t)(nvmlDevice_t device, nvmlClockType_t type, unsigned int *clock);
typedef nvmlReturn_t (*nvmlDeviceGetTemperature_t)(nvmlDevice_t device, nvmlTemperatureSensors_t sensorType, unsigned int *temp);
typedef nvmlReturn_t (*nvmlDeviceGetPowerUsage_t)(nvmlDevice_t device, unsigned int *power);
typedef nvmlReturn_t (*nvmlDeviceGetMemoryInfo_t)(nvmlDevice_t device, nvmlMemory_t *memory);
typedef nvmlReturn_t (*nvmlDeviceGetUtilizationRates_t)(nvmlDevice_t device, nvmlUtilization_t *utilization);

#endif // NVMLDEFS_H
