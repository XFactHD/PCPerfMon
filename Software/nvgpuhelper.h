//http://eliang.blogspot.com/2011/05/getting-nvidia-gpu-usage-in-c.html

#ifndef GPUHELPER_H
#define GPUHELPER_H

#include <QObject>

#include <windows.h>

#include <iostream>

//#define DEBUG

#ifdef DEBUG
#define PRINT_DEBUG(msg) std::cout << msg << std::endl
#define WPRINT_DEBUG(msg) std::wcout << msg << std::endl
#else
#define PRINT_DEBUG(msg)
#define WPRINT_DEBUG(msg)
#endif

// magic numbers, do not change them
#define MAKE_NVAPI_VERSION(typeName,ver) (uint32_t)(sizeof(typeName) | ((ver) << 16))
#define NVAPI_MAX_PHYSICAL_GPUS           64
//#define NVAPI_MAX_USAGES_PER_GPU          34
#define NVAPI_MAX_GPU_UTILIZATIONS         8
#define NVAPI_MAX_THERMAL_SENSORS_PER_GPU  3
#define NVAPI_MAX_GPU_PUBLIC_CLOCKS       32

#define DLL_ADDR_NvAPI_Initialize 0x150E828
#define DLL_ADDR_NvAPI_EnumPhysicalGPUs 0xE5AC921F
#define DLL_ADDR_NvAPI_GPU_GetDynamicPstatesInfoEx 0x60DED2ED
#define DLL_ADDR_NvAPI_GPU_ClientPowerTopologyGetStatus 0x0EDCF624E
#define DLL_ADDR_NvAPI_GPU_GetMemoryInfo 0x7F9B368
#define DLL_ADDR_NvAPI_GPU_GetThermalSettings 0xE3640A56
#define DLL_ADDR_NvAPI_GPU_GetAllClockFrequencies 0xDCB616C3
#define DLL_ADDR_NvAPI_GPU_GetShortName 0xD988F0F3
#define DLL_ADDR_NvAPI_Unload 0xD22BDD7E

// data types
typedef uint32_t* NvPhysicalGpuHandle;
typedef int32_t NvAPI_Status;

typedef struct {
    uint32_t version;
    uint32_t flags;
    struct {
        uint32_t bIsPresent = 1;
        uint32_t percentage;
    } utilization[NVAPI_MAX_GPU_UTILIZATIONS];
} NV_GPU_DYNAMIC_PSTATES_INFO_EX;

typedef enum {
    NV_POWER_TOPOLOGY_FLAG_UNKNOWN1,
    NV_POWER_TOPOLOGY_FLAG_UNKNOWN2
} NV_POWER_TOPOLOGY_FLAGS;

typedef struct {
    uint32_t version;
    uint32_t count = 0;

    struct {
        NV_POWER_TOPOLOGY_FLAGS flags;
        uint32_t unknown2 = 0;
        uint32_t power = 0;
        uint32_t unknown3 = 0;
    } entries[4];
} NV_POWER_TOPOLOGY_STATUS;

typedef struct {
    uint32_t version;                           //!< Version info
    uint32_t dedicatedVideoMemory;              //!< Size(in kb) of the physical framebuffer.
    uint32_t availableDedicatedVideoMemory;     //!< Size(in kb) of the available physical framebuffer for allocating video memory surfaces.
    uint32_t systemVideoMemory;                 //!< Size(in kb) of system memory the driver allocates at load time.
    uint32_t sharedSystemMemory;                //!< Size(in kb) of shared system memory that driver is allowed to commit for surfaces across all allocations.
    uint32_t curAvailableDedicatedVideoMemory;  //!< Size(in kb) of the current available physical framebuffer for allocating video memory surfaces.
    uint32_t dedicatedVideoMemoryEvictionsSize; //!< Size(in kb) of the total size of memory released as a result of the evictions.
    uint32_t dedicatedVideoMemoryEvictionCount; //!< Indicates the number of eviction events that caused an allocation to be removed from dedicated video memory to free GPU video memory to make room for other allocations.
} NV_DISPLAY_DRIVER_MEMORY_INFO_V3;

typedef enum {
    NVAPI_THERMAL_CONTROLLER_NONE = 0,
    NVAPI_THERMAL_CONTROLLER_GPU_INTERNAL,
    NVAPI_THERMAL_CONTROLLER_ADM1032,
    NVAPI_THERMAL_CONTROLLER_MAX6649,
    NVAPI_THERMAL_CONTROLLER_MAX1617,
    NVAPI_THERMAL_CONTROLLER_LM99,
    NVAPI_THERMAL_CONTROLLER_LM89,
    NVAPI_THERMAL_CONTROLLER_LM64,
    NVAPI_THERMAL_CONTROLLER_ADT7473,
    NVAPI_THERMAL_CONTROLLER_SBMAX6649,
    NVAPI_THERMAL_CONTROLLER_VBIOSEVT,
    NVAPI_THERMAL_CONTROLLER_OS,
    NVAPI_THERMAL_CONTROLLER_UNKNOWN = -1
} NV_THERMAL_CONTROLLER;

typedef enum {
    NVAPI_THERMAL_TARGET_NONE = 0,
    NVAPI_THERMAL_TARGET_GPU = 1,           //!<GPU core temperature requires NvPhysicalGpuHandle.
    NVAPI_THERMAL_TARGET_MEMORY = 2,        //!<GPU memory temperature requires NvPhysicalGpuHandle.
    NVAPI_THERMAL_TARGET_POWER_SUPPLY = 4,  //!<GPU power supply temperature requires NvPhysicalGpuHandle.
    NVAPI_THERMAL_TARGET_BOARD = 8,         //!<GPU board ambient temperature requires NvPhysicalGpuHandle.
    NVAPI_THERMAL_TARGET_VCD_BOARD = 9, 	//!<Visual Computing Device Board temperature requires NvVisualComputingDeviceHandle.
    NVAPI_THERMAL_TARGET_VCD_INLET = 10, 	//!<Visual Computing Device Inlet temperature requires NvVisualComputingDeviceHandle.
    NVAPI_THERMAL_TARGET_VCD_OUTLET = 11, 	//!<Visual Computing Device Outlet temperature requires NvVisualComputingDeviceHandle.
    NVAPI_THERMAL_TARGET_ALL = 15,
    NVAPI_THERMAL_TARGET_UNKNOWN = -1
} NV_THERMAL_TARGET;

typedef struct {
    uint32_t version;
    uint32_t count;
    struct {
        NV_THERMAL_CONTROLLER controller;
        int32_t defaultMinTemp;
        int32_t defaultMaxTemp;
        int32_t currentTemp;
        NV_THERMAL_TARGET target;
    } sensor[NVAPI_MAX_THERMAL_SENSORS_PER_GPU];
} NV_GPU_THERMAL_SETTINGS_V2;

typedef enum {
    NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ,
    NV_GPU_CLOCK_FREQUENCIES_BASE_CLOCK,
    NV_GPU_CLOCK_FREQUENCIES_BOOST_CLOCK,
    NV_GPU_CLOCK_FREQUENCIES_CLOCK_TYPE_NUM
} NV_GPU_CLOCK_FREQUENCIES_CLOCK_TYPE;

typedef struct {
    uint32_t version;
    NV_GPU_CLOCK_FREQUENCIES_CLOCK_TYPE clockType:2;
    uint32_t reserved:22;
    uint32_t reserved2:8;
    struct {
        uint32_t bIsPresent:1;
        uint32_t reserved:31;
        uint32_t frequency;
    } domain[NVAPI_MAX_GPU_PUBLIC_CLOCKS];
} NV_GPU_CLOCK_FREQUENCIES_V2;

// function pointer types
typedef int32_t*     (*NvAPI_QueryInterface_t)(unsigned int offset);
typedef NvAPI_Status (*NvAPI_Initialize_t)();
typedef NvAPI_Status (*NvAPI_EnumPhysicalGPUs_t)(NvPhysicalGpuHandle* nvGPUHandle, int* count);
typedef NvAPI_Status (*NvAPI_GPU_GetDynamicPstatesInfoEx_t)(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_DYNAMIC_PSTATES_INFO_EX* pDynamicPstatesInfoEx);
typedef NvAPI_Status (*NvAPI_GPU_ClientPowerTopologyGetStatus_t)(NvPhysicalGpuHandle hPhysicalGpu, NV_POWER_TOPOLOGY_STATUS* pTopologyStatus);
typedef NvAPI_Status (*NvAPI_GPU_GetMemoryInfo_t)(NvPhysicalGpuHandle hPhysicalGpu, NV_DISPLAY_DRIVER_MEMORY_INFO_V3* pMemoryInfo);
typedef NvAPI_Status (*NvAPI_GPU_GetThermalSettings_t)(NvPhysicalGpuHandle hPhysicalGpu, uint32_t sensorIndex, NV_GPU_THERMAL_SETTINGS_V2* pThermalSettings);
typedef NvAPI_Status (*NvAPI_GPU_GetAllClockFrequencies_t)(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_CLOCK_FREQUENCIES_V2* pClkFreqs);
typedef NvAPI_Status (*NvAPI_GPU_GetShortName_t)(NvPhysicalGpuHandle hPhysicalGpu, char name[64]);
typedef NvAPI_Status (*NvAPI_Unload_t)();

class NvGPUHelper : public QObject
{
    Q_OBJECT
public:
    explicit NvGPUHelper(QObject *parent = nullptr);

    uint32_t getGpuLoad();
    uint16_t getGpuClock();
    void getVRamInfo(uint32_t* used, uint32_t* total);
    uint32_t getGpuTemperature();
    uint32_t getGpuPower();

    void shutdown();

signals:

private:
    void getGpuTdp();

    NvAPI_QueryInterface_t                      NvAPI_QueryInterface                    = NULL;
    NvAPI_Initialize_t                          NvAPI_Initialize                        = NULL;
    NvAPI_EnumPhysicalGPUs_t                    NvAPI_EnumPhysicalGPUs                  = NULL;
    NvAPI_GPU_GetDynamicPstatesInfoEx_t         NvAPI_GPU_GetDynamicPstatesInfoEx       = NULL;
    NvAPI_GPU_ClientPowerTopologyGetStatus_t    NvAPI_GPU_ClientPowerTopologyGetStatus  = NULL;
    NvAPI_GPU_GetMemoryInfo_t                   NvAPI_GPU_GetMemoryInfo                 = NULL;
    NvAPI_GPU_GetThermalSettings_t              NvAPI_GPU_GetThermalSettings            = NULL;
    NvAPI_GPU_GetAllClockFrequencies_t          NvAPI_GPU_GetAllClockFrequencies        = NULL;
    NvAPI_GPU_GetShortName_t                    NvAPI_GPU_GetShortName                  = NULL;
    NvAPI_Unload_t                              NvAPI_Unload                            = NULL;

    bool ready = false;
    double gpuTdp = 0;
    NvPhysicalGpuHandle primaryGpu = 0;
    NV_GPU_DYNAMIC_PSTATES_INFO_EX pstates;
    NV_POWER_TOPOLOGY_STATUS power;
    NV_DISPLAY_DRIVER_MEMORY_INFO_V3 memory;
    NV_GPU_THERMAL_SETTINGS_V2 thermal;
    NV_GPU_CLOCK_FREQUENCIES_V2 clocks;
};

#endif // GPUHELPER_H
