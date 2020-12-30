#include "nvgpuhelper.h"

NvGPUHelper::NvGPUHelper(QObject *parent) : QObject(parent)
{
    const char* dllName = sizeof(void*) == 4 ? "nvapi.dll" : "nvapi64.dll";
    HMODULE hmod = LoadLibraryA(dllName);
    if(hmod == NULL) { std::cerr << "Couldn't find nvapi.dll" << std::endl; return; }

    // nvapi_QueryInterface is a function used to retrieve other internal functions in nvapi.dll
    NvAPI_QueryInterface = (NvAPI_QueryInterface_t)GetProcAddress(hmod, "nvapi_QueryInterface");
    if(NvAPI_QueryInterface == NULL) { qCritical("Failed to get NvAPI_QueryInterface from nvapi.dll."); return; }

    // Some useful internal functions that aren't exported by nvapi.dll
    NvAPI_Initialize =                          (NvAPI_Initialize_t)                        (*NvAPI_QueryInterface)(DLL_ADDR_NvAPI_Initialize);
    NvAPI_EnumPhysicalGPUs =                    (NvAPI_EnumPhysicalGPUs_t)                  (*NvAPI_QueryInterface)(DLL_ADDR_NvAPI_EnumPhysicalGPUs);
    NvAPI_GPU_GetDynamicPstatesInfoEx =         (NvAPI_GPU_GetDynamicPstatesInfoEx_t)       (*NvAPI_QueryInterface)(DLL_ADDR_NvAPI_GPU_GetDynamicPstatesInfoEx);
    NvAPI_GPU_ClientPowerTopologyGetStatus =    (NvAPI_GPU_ClientPowerTopologyGetStatus_t)  (*NvAPI_QueryInterface)(DLL_ADDR_NvAPI_GPU_ClientPowerTopologyGetStatus);
    NvAPI_GPU_GetMemoryInfo =                   (NvAPI_GPU_GetMemoryInfo_t)                 (*NvAPI_QueryInterface)(DLL_ADDR_NvAPI_GPU_GetMemoryInfo);
    NvAPI_GPU_GetThermalSettings =              (NvAPI_GPU_GetThermalSettings_t)            (*NvAPI_QueryInterface)(DLL_ADDR_NvAPI_GPU_GetThermalSettings);
    NvAPI_GPU_GetAllClockFrequencies =          (NvAPI_GPU_GetAllClockFrequencies_t)        (*NvAPI_QueryInterface)(DLL_ADDR_NvAPI_GPU_GetAllClockFrequencies);
    NvAPI_GPU_GetShortName =                    (NvAPI_GPU_GetShortName_t)                  (*NvAPI_QueryInterface)(DLL_ADDR_NvAPI_GPU_GetShortName);
    NvAPI_Unload =                              (NvAPI_Unload_t)                            (*NvAPI_QueryInterface)(DLL_ADDR_NvAPI_Unload);

    if (NvAPI_Initialize == NULL || NvAPI_EnumPhysicalGPUs == NULL ||
        NvAPI_EnumPhysicalGPUs == NULL || NvAPI_GPU_GetDynamicPstatesInfoEx == NULL ||
        NvAPI_GPU_ClientPowerTopologyGetStatus == NULL || NvAPI_GPU_GetMemoryInfo == NULL ||
        NvAPI_GPU_GetThermalSettings == NULL || NvAPI_GPU_GetAllClockFrequencies == NULL ||
        NvAPI_GPU_GetShortName == NULL || NvAPI_Unload == NULL)
    {
        qCritical("Failed to get private functions in nvapi.dll");
        return;
    }

    // Initialize NvAPI library, call it once before calling any other NvAPI functions
    (*NvAPI_Initialize)();

    // Get handles to physical GPUs installed in the system
    int gpuCount = 0;
    NvPhysicalGpuHandle gpuHandles[NVAPI_MAX_PHYSICAL_GPUS] = { NULL };
    (*NvAPI_EnumPhysicalGPUs)(gpuHandles, &gpuCount);
    if(gpuCount == 0) { std::cerr << "No GPUs were found." << std::endl; }
    primaryGpu = gpuHandles[0];

    getGpuTdp();

    // Initialize version field of the data structs
    pstates.version = MAKE_NVAPI_VERSION(NV_GPU_DYNAMIC_PSTATES_INFO_EX, 1);
    power.version = MAKE_NVAPI_VERSION(NV_POWER_TOPOLOGY_STATUS, 1);
    memory.version = MAKE_NVAPI_VERSION(NV_DISPLAY_DRIVER_MEMORY_INFO_V3, 3);
    thermal.version = MAKE_NVAPI_VERSION(NV_GPU_THERMAL_SETTINGS_V2, 2);
    clocks.version = MAKE_NVAPI_VERSION(NV_GPU_CLOCK_FREQUENCIES_V2, 2);

    //Initialize miscellaneous constant fields of the data structs
    clocks.clockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;

    // Ready to rock
    ready = true;
}

uint32_t NvGPUHelper::getGpuLoad()
{
    if(!ready) { return 0; }

    (*NvAPI_GPU_GetDynamicPstatesInfoEx)(primaryGpu, &pstates);
    int usage = pstates.utilization[0].percentage;

    return usage;
}

uint16_t NvGPUHelper::getGpuClock()
{
    if(!ready) { return 0; }

    (*NvAPI_GPU_GetAllClockFrequencies)(primaryGpu, &clocks);
    uint32_t frequency = clocks.domain[0].frequency;
    frequency /= 1000; //Convert to MHz

    return frequency;
}

void NvGPUHelper::getVRamInfo(uint32_t* used, uint32_t* total)
{
    if(!ready) { return; }

    (*NvAPI_GPU_GetMemoryInfo)(primaryGpu, &memory);
    *used = memory.dedicatedVideoMemory - memory.curAvailableDedicatedVideoMemory;
    *total = memory.dedicatedVideoMemory;
}

uint32_t NvGPUHelper::getGpuTemperature()
{
    if(!ready) { return 0; }

    (*NvAPI_GPU_GetThermalSettings)(primaryGpu, NVAPI_THERMAL_TARGET_ALL, &thermal);
    for(uint32_t i = 0; i < thermal.count; i++) {
        if(thermal.sensor[i].target == NVAPI_THERMAL_TARGET_GPU) {
            return thermal.sensor[i].currentTemp;
        }
    }

    return 0;
}

uint32_t NvGPUHelper::getGpuPower()
{
    if(!ready) { return 0; }

    (*NvAPI_GPU_ClientPowerTopologyGetStatus)(primaryGpu, &power);
    double tdpPercent = power.entries[0].power / 100000.0;

    return (uint32_t)(gpuTdp * tdpPercent);
}

void NvGPUHelper::shutdown()
{
    if(ready) {
        ready = false;
        (*NvAPI_Unload)();
    }
}



void NvGPUHelper::getGpuTdp()
{
    char buffer[64] = { 0 };
    (*NvAPI_GPU_GetShortName)(primaryGpu, buffer);

    QString name(buffer);
    name = name.split('-')[0]; //Split off revision

    //TODO: add more GPUs
    if(name.compare("GP104") == 0) {
        gpuTdp = 180;
    }
    else if(name.compare("GP102") == 0) {
        gpuTdp = 250;
    }
}












