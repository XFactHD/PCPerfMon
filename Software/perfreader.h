#ifndef PERFREADER_H
#define PERFREADER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include <iostream>

#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>

#include "nvmlreader.h"
#include "ohmwrapper.h"

//#define DEBUG

#ifdef DEBUG
#define PRINT_DEBUG(msg) std::cout << msg << std::endl
#define WPRINT_DEBUG(msg) std::wcout << msg << std::endl
#else
#define PRINT_DEBUG(msg)
#define WPRINT_DEBUG(msg)
#endif

typedef struct cpu_info {
    float cpuLoad;     //in %
    uint16_t cpuClock; //in MHz
    uint16_t cpuTemp;  //in °C
    uint32_t cpuPower; //in W
} cpu_info_t;

typedef struct ram_info {
    uint64_t ramTotal; //in Bytes
    uint64_t ramUsed;  //in Bytes
    int ramLoad;       //in %
} ram_info_t;

typedef struct net_info {
    double bandwidth; //in KBit/s
    double netIn;     //in KBit/s
    double netOut;    //in KBit/s
} net_info_t;

typedef struct gpu_info {
    int gpuLoad;        //in %
    uint16_t gpuClock;  //in MHz
    uint64_t vramTotal; //in Bytes
    uint64_t vramUsed;  //in Bytes
    int vramLoad;       //in %
    uint32_t gpuTemp;   //in °C
    uint32_t gpuPower;  //in W
} gpu_info_t;

Q_DECLARE_METATYPE(cpu_info);
Q_DECLARE_METATYPE(ram_info);
Q_DECLARE_METATYPE(net_info);
Q_DECLARE_METATYPE(gpu_info);

class PerfReader : public QThread
{
    Q_OBJECT
public:
    explicit PerfReader(QObject *parent = nullptr);
    ~PerfReader();

    void run();
    void requestShutdown();

    void initialize();
    void shutdown();

    void queryNewData();

    cpu_info_t getCPUInfo();
    ram_info_t getRAMInfo();
    net_info_t getNetInfo();
    gpu_info_t getGPUInfo();

signals:
    void perfdataReady(cpu_info_t, ram_info_t, net_info_t, gpu_info_t);

private:
    void initializeNetCounters();

    float getCPULoad(); //https://stackoverflow.com/a/23148493

    void getCounterLocalizationTable();
    size_t localizeCounter(PWSTR counterEn, PWSTR* counterLoc);
    PDH_STATUS enumObjectItems(PWSTR object, PWSTR** counterList, size_t* counterCount, PWSTR** instanceList, size_t* instanceCount);
    void deleteList(PWSTR** list, size_t listSize);
    void constructCounterPath(PWSTR dest, size_t destSize, PWSTR object, PWSTR instance, PWSTR counter);
    PDH_STATUS openQuery();
    PDH_STATUS addCounter(PWSTR path, PDH_HCOUNTER* counter);

    uint64_t fileTimeToUInt64(const FILETIME& ft) { return (((uint64_t)ft.dwHighDateTime) << 32) | ((uint64_t)ft.dwLowDateTime); }

    size_t localizationTableSize = 0;
    PWSTR* localizationTable = NULL;

    const PWSTR CPU_INFO_OBJECT_NAME = (PWSTR)L"Processor Information";
    const PWSTR CPU_FREQUENCY_COUNTER_NAME = (PWSTR)L"Processor Frequency";
    const PWSTR POWER_OBJECT_NAME = (PWSTR)L"Power Meter";
    const PWSTR POWER_COUNTER_NAME = (PWSTR)L"Power";
    const PWSTR THERMAL_OBJECT_NAME = (PWSTR)L"Thermal Zone Information";
    const PWSTR TEMP_COUNTER_NAME = (PWSTR)L"Temperature";
    const PWSTR NET_OBJECT_NAME = (PWSTR)L"Network Interface";
    const PWSTR NET_BW_COUNTER_NAME = (PWSTR)L"Current Bandwidth";
    const PWSTR NET_IN_COUNTER_NAME = (PWSTR)L"Bytes Received/sec";
    const PWSTR NET_OUT_COUNTER_NAME = (PWSTR)L"Bytes Sent/sec";

    PDH_HQUERY query = NULL;
    PDH_HCOUNTER cpuFreqCounter = NULL;
    PDH_HCOUNTER powerCounter = NULL;
    PDH_HCOUNTER tempCounter = NULL;
    PDH_HCOUNTER netBwCounter = NULL;
    PDH_HCOUNTER netInCounter = NULL;
    PDH_HCOUNTER netOutCounter = NULL;
    PPDH_FMT_COUNTERVALUE cpuFreqValue = new PDH_FMT_COUNTERVALUE;
    PPDH_FMT_COUNTERVALUE powerValue = new PDH_FMT_COUNTERVALUE;
    PPDH_FMT_COUNTERVALUE tempValue = new PDH_FMT_COUNTERVALUE;
    PPDH_FMT_COUNTERVALUE netBwValue = new PDH_FMT_COUNTERVALUE;
    PPDH_FMT_COUNTERVALUE netInValue = new PDH_FMT_COUNTERVALUE;
    PPDH_FMT_COUNTERVALUE netOutValue = new PDH_FMT_COUNTERVALUE;

    PDH_STATUS lastQueryStatus = ERROR_SUCCESS;

    bool running = true;
    bool request = false;
    QMutex mutex;
    QWaitCondition condition;

    NvMLReader* gpu;
    OHMWrapper* ohm;
};

#endif // PERFREADER_H
