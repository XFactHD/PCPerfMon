#include "perfreader.h"

PerfReader::PerfReader(QObject *parent) : QThread(parent) { }

PerfReader::~PerfReader()
{
    if(localizationTable != NULL) {
        for(size_t i = 0; i < localizationTableSize; i++) {
            delete[] localizationTable[i];
        }
        delete[] localizationTable;
    }

    delete gpu;
    delete ohm;
}

void PerfReader::run()
{
    initialize();

    while (running) {
        mutex.lock();
        condition.wait(&mutex);
        mutex.unlock();

        if(request) {
            mutex.lock();
            request = false;
            mutex.unlock();

            lastQueryStatus = PdhCollectQueryData(query);

            cpu_info_t cpuInfo = getCPUInfo();
            ram_info_t ramInfo = getRAMInfo();
            net_info_t netInfo = getNetInfo();
            gpu_info_t gpuInfo = getGPUInfo();

            emit perfdataReady(cpuInfo, ramInfo, netInfo, gpuInfo);
        }
    }

    shutdown();
}

void PerfReader::initialize()
{
    getCounterLocalizationTable();

    openQuery();
    initializeNetCounters();
    gpu = new NvMLReader();
    ohm = new OHMWrapper();
    ohm->init();
}

void PerfReader::requestShutdown()
{
    mutex.lock();
    running = false;
    condition.wakeOne();
    mutex.unlock();
}

void PerfReader::shutdown()
{
#ifdef DEBUG
    qDebug("Shutting down PerfReader");
#endif
    if(netBwCounter != NULL) { PdhRemoveCounter(netBwCounter); }
    if(netInCounter != NULL) { PdhRemoveCounter(netInCounter); }
    if(netOutCounter != NULL) { PdhRemoveCounter(netOutCounter); }
    if(query != NULL) { PdhCloseQuery(query); }

    gpu->shutdown();
    ohm->shutdown();
}



void PerfReader::queryNewData()
{
    mutex.lock();
    request = true;
    condition.wakeOne();
    mutex.unlock();
}

cpu_info_t PerfReader::getCPUInfo()
{
    cpu_info_t cpuInfo = { getCPULoad(), 0, 0 , 0};

    ohm->update();
    cpuInfo.cpuClock = ohm->getCpuClockAvg();
    cpuInfo.cpuTemp = ohm->getCpuPkgTemp();
    cpuInfo.cpuPower = ohm->getCpuPkgPower();

    return cpuInfo;
}

ram_info_t PerfReader::getRAMInfo()
{
    MEMORYSTATUSEX memStat;
    memStat.dwLength = sizeof (memStat);
    GlobalMemoryStatusEx(&memStat);

    uint64_t ramTotal = memStat.ullTotalPhys;
    uint64_t ramUsed = ramTotal - memStat.ullAvailPhys;
    int ramLoad = memStat.dwMemoryLoad;

    return { ramTotal, ramUsed, ramLoad };
}

net_info_t PerfReader::getNetInfo()
{
    net_info_t netInfo = { 0, 0, 0 };

    PDH_STATUS status = PdhGetFormattedCounterValue(netBwCounter, PDH_FMT_LONG, NULL, netBwValue);
    if (status == ERROR_SUCCESS)
    {
        netInfo.netIn = netBwValue->longValue / 1000.0;
        netInfo.netOut = netBwValue->longValue / 1000.0;
    }
    status = PdhGetFormattedCounterValue(netInCounter, PDH_FMT_LONG, NULL, netInValue);
    if (status == ERROR_SUCCESS)
    {
        netInfo.netIn = netInValue->longValue * 8 / 1000.0;
    }

    status = PdhGetFormattedCounterValue(netOutCounter, PDH_FMT_LONG, NULL, netOutValue);
    if (status == ERROR_SUCCESS)
    {
        netInfo.netOut = netOutValue->longValue * 8 / 1000.0;
    }

    return netInfo;
}

gpu_info_t PerfReader::getGPUInfo()
{
    gpu_info_t gpuInfo = { 0, 0, 0, 0, 0, 0, 0 };

    gpuInfo.gpuLoad = gpu->getGpuLoad();
    gpuInfo.gpuClock = gpu->getGpuClock();

    uint64_t vramUsed = 0;
    uint64_t vramTotal = 0;
    gpu->getVRamInfo(&vramUsed, &vramTotal);
    gpuInfo.vramUsed = vramUsed;
    gpuInfo.vramTotal = vramTotal;
    gpuInfo.vramLoad = (vramUsed == 0) ? 0.0 : (((double)vramUsed) / ((double)vramTotal) * 100);

    gpuInfo.gpuTemp = gpu->getGpuTemperature();
    gpuInfo.gpuPower = gpu->getGpuPower();

    return gpuInfo;
}



void PerfReader::initializeNetCounters()
{
    PWSTR objectName = NULL;
    PWSTR bwCounterName = NULL;
    PWSTR inCounterName = NULL;
    PWSTR outCounterName = NULL;
    localizeCounter(NET_OBJECT_NAME, &objectName);
    localizeCounter(NET_BW_COUNTER_NAME, &bwCounterName);
    localizeCounter(NET_IN_COUNTER_NAME, &inCounterName);
    localizeCounter(NET_OUT_COUNTER_NAME, &outCounterName);

    PWSTR* instanceList = NULL;
    size_t instanceCount = 0;

    enumObjectItems(objectName, NULL, NULL, &instanceList, &instanceCount);
    if(instanceList == NULL || instanceCount == 0) { return; }

    PWSTR path = new wchar_t[2048];

    constructCounterPath(path, 2048, objectName, instanceList[0], bwCounterName);
    addCounter(path, &netBwCounter);

    constructCounterPath(path, 2048, objectName, instanceList[0], inCounterName);
    addCounter(path, &netInCounter);

    constructCounterPath(path, 2048, objectName, instanceList[0], outCounterName);
    addCounter(path, &netOutCounter);

    deleteList(&instanceList, instanceCount);
}



float PerfReader::getCPULoad()
{
    static uint64_t prevTotalTicks = 0;
    static uint64_t prevIdleTicks = 0;

    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;

    if(GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        uint64_t totalTicks = fileTimeToUInt64(kernelTime) + fileTimeToUInt64(userTime);
        uint64_t idleTicks = fileTimeToUInt64(idleTime);

        uint64_t totalDiff = totalTicks - prevTotalTicks;
        uint64_t idleDiff = idleTicks - prevIdleTicks;

        float result = 1.0f - ((totalDiff > 0) ? (((float)idleDiff) / ((float)totalDiff)) : 0.0f);

        prevTotalTicks = totalTicks;
        prevIdleTicks = idleTicks;

        return result;
    }

    return -1.0f;
}



void PerfReader::getCounterLocalizationTable()
{
    DWORD size = 1024;
    PWSTR data = new WCHAR[size];
    DWORD passedSize = size;

    PRINT_DEBUG("Querying HKEY_PERFORMANCE_TEXT for localization.");
    LSTATUS status = RegQueryValueExW(HKEY_PERFORMANCE_TEXT, L"Counter", NULL, NULL, (LPBYTE)data, &passedSize);
    while(status == ERROR_MORE_DATA) {
        delete[] data;
        size *= 2;
        passedSize = size;
        data = new WCHAR[size];
        status = RegQueryValueExW(HKEY_PERFORMANCE_TEXT, L"Counter", NULL, NULL, (LPBYTE)data, &passedSize);
    }

    if(status != ERROR_SUCCESS) {
        PRINT_DEBUG("Querying HKEY_PERFORMANCE_TEXT failed with 0x" << std::hex << status << ".");
        return;
    }
    PRINT_DEBUG("Localization data retrieved.");

    size_t entryCount = passedSize == 0 ? 0 : std::count(data, data + passedSize - 1, '\0');
    if(entryCount == 0) {
        PRINT_DEBUG("Querying HKEY_PERFORMANCE_TEXT returned no entries.");
        return;
    }

    localizationTable = new PWSTR[entryCount];
    if(localizationTable == NULL) {
        PRINT_DEBUG("Failed to allocate memory for localization table.");
        return;
    }
    localizationTableSize = entryCount;
    PRINT_DEBUG("Localization table created.");

    //PRINT_DEBUG("Localization entries:");
    size_t i = 0;
    for(PWSTR entry = data; *entry != '\0'; entry += wcslen(entry) + 1) {
        //WPRINT_DEBUG(i << L": " << entry);
        if(i < entryCount) {
            size_t len = wcslen(entry);
            PWSTR loc = new WCHAR[len + 1];
            wcscpy_s(loc, len + 1, entry);
            localizationTable[i] = loc;
        }
        i++;
    }
    PRINT_DEBUG("Localization entries parsed.");
}

size_t PerfReader::localizeCounter(PWSTR counterEn, PWSTR* counterLoc)
{
    if(localizationTable == NULL) {
        PRINT_DEBUG("Localization table empty. Localization failed.");
        return 0;
    }

    WPRINT_DEBUG(L"Localizing \"" << counterEn << L"\".");
    for(size_t i = 0; i < localizationTableSize; i++) {
        PWSTR entry = localizationTable[i];
        if(wcscmp(entry, counterEn) == 0) {
            DWORD idx = _wtoi(localizationTable[i - 1]);

            DWORD bufSize = PDH_MAX_COUNTER_NAME;
            LPWSTR name = new WCHAR[bufSize];
            PDH_STATUS status = PdhLookupPerfNameByIndexW(NULL, idx, name, &bufSize);
            if(status == ERROR_SUCCESS) {
                if(bufSize == 0 || wcslen(name) == 0) { break; } //If the result is empty, the key is not localized

                size_t len = wcslen(name);
                *counterLoc = new WCHAR[len + 1];
                wcscpy_s(*counterLoc, len + 1, name);
                WPRINT_DEBUG(L"Localization for \"" << counterEn << L"\" returned \"" << name << L"\".");
                return len;
            }
        }
    }

    WPRINT_DEBUG(L"\"" << counterEn << L"\" has no localization, checking if this key even exists.");
    for(size_t i = 0; i < localizationTableSize; i++) {
        PWSTR entry = localizationTable[i];
        if(wcscmp(entry, counterEn) == 0) {
            size_t len = wcslen(entry);
            *counterLoc = new WCHAR[len + 1];
            wcscpy_s(*counterLoc, len + 1, entry);
            WPRINT_DEBUG(L"Found \"" << counterEn << L"\" in localization table, returning original value.");
            return len;
        }
    }

    WPRINT_DEBUG(L"Failed to localize \"" << counterEn << L"\".");
    return 0;
}

PDH_STATUS PerfReader::enumObjectItems(PWSTR object, PWSTR** counterList, size_t* counterCount, PWSTR** instanceList, size_t* instanceCount)
{
    LPWSTR pwsCounterListBuffer = NULL;
    DWORD dwCounterListSize = 0;
    LPWSTR pwsInstanceListBuffer = NULL;
    DWORD dwInstanceListSize = 0;

    WPRINT_DEBUG(L"Enumerating items for object \"" << object << "\".");

    //List all counter and instance names to get the list of network interface name (Called with NULL and size 0 first to get required buffer sizes)
    PDH_STATUS status = PdhEnumObjectItemsW(
            NULL,                   // real-time source
            NULL,                   // local machine
            object,                 // object to enumerate
            pwsCounterListBuffer,   // pass NULL and 0
            &dwCounterListSize,     // to get required buffer size
            pwsInstanceListBuffer,
            &dwInstanceListSize,
            PERF_DETAIL_WIZARD,     // counter detail level
            0);
    WPRINT_DEBUG(L"Buffer sizes requested.");

    if(status != (PDH_STATUS)PDH_MORE_DATA) {
        WPRINT_DEBUG(L"PdhEnumObjectItems for object \"" << object << "\" failed with 0x" << std::hex << status << ".");
        return status;
    }

    //Allocate required buffers
    pwsCounterListBuffer = (LPWSTR)malloc(dwCounterListSize * sizeof(WCHAR));
    pwsInstanceListBuffer = (LPWSTR)malloc(dwInstanceListSize * sizeof(WCHAR));
    if(pwsCounterListBuffer == NULL || pwsInstanceListBuffer == NULL) {

        WPRINT_DEBUG(L"Unable to allocate buffers.");
        return status;
    }

    //List all counter and instance names to get the list of network interface name (Called again to actually get the data)
    status = PdhEnumObjectItems(
        NULL,                   // real-time source
        NULL,                   // local machine
        object,                 // object to enumerate
        pwsCounterListBuffer,
        &dwCounterListSize,
        pwsInstanceListBuffer,
        &dwInstanceListSize,
        PERF_DETAIL_WIZARD,     // counter detail level
        0);
    WPRINT_DEBUG(L"Counter and instance names requested.");

    if (status != ERROR_SUCCESS) {
        WPRINT_DEBUG(L"Second PdhEnumObjectItems failed with 0x" << std::hex << status << ".");
        return status;
    }

    if(counterList != NULL && counterCount != NULL) {
        //Allocate counter list
        *counterCount = dwCounterListSize == 0 ? 0 : std::count(pwsCounterListBuffer, pwsCounterListBuffer + dwCounterListSize - 1, '\0');
        *counterList = counterCount == 0 ? NULL : new PWSTR[*counterCount];

        //Traverse counter buffer and copy counters to counter list
        if(*counterList != NULL) {
            size_t i = 0;
            for (PWSTR pTemp = pwsCounterListBuffer; *pTemp != 0; pTemp += wcslen(pTemp) + 1)
            {
                if (i < *counterCount) {
                    (*counterList)[i] = new wchar_t[wcslen(pTemp) + 1];
                    wcscpy_s((*counterList)[i], wcslen(pTemp) + 1, pTemp);
                }
                i++;
            }
        }
        else { WPRINT_DEBUG(L"Unable to allocate counter list."); }
    }

    if(instanceList != NULL && instanceCount != NULL) {
        //Allocate instance list
        *instanceCount = dwInstanceListSize == 0 ? 0 : std::count(pwsInstanceListBuffer, pwsInstanceListBuffer + dwInstanceListSize - 1, '\0');
        *instanceList = instanceCount == 0 ? NULL : new PWSTR[*instanceCount];

        //Traverse instance buffer and copy instances to instance list
        if(*instanceList != NULL) {
            size_t i = 0;
            for (PWSTR pTemp = pwsInstanceListBuffer; *pTemp != 0; pTemp += wcslen(pTemp) + 1)
            {
                if (i < *instanceCount) {
                    (*instanceList)[i] = new wchar_t[wcslen(pTemp) + 1];
                    wcscpy_s((*instanceList)[i], wcslen(pTemp) + 1, pTemp);
                }
                i++;
            }
        }
        else { WPRINT_DEBUG(L"Unable to allocate instance list."); }
    }

    free(pwsCounterListBuffer);
    free(pwsInstanceListBuffer);

    WPRINT_DEBUG(L"Items for object \"" << object << "\" enumerated.");
    return status;
}

void PerfReader::deleteList(PWSTR** list, size_t listSize)
{
    if(*list != NULL) {
        for(size_t i = 0; i < listSize; i++) {
            delete (*list)[i];
        }
        delete *list;
    }
}

void PerfReader::constructCounterPath(PWSTR dest, size_t destSize, PWSTR object, PWSTR instance, PWSTR counter)
{
    WPRINT_DEBUG(L"Constructing counter path for counter \"" << counter << L"\" on instance \"" << (instance == NULL ? (L"null") : instance) << L"\" of object \"" << object << L"\".");

    memset(dest, 0, destSize);

    wcscat_s(dest, destSize, L"\\");
    wcscat_s(dest, destSize, object);
    if(instance != NULL) {
        wcscat_s(dest, destSize, L"(");
        wcscat_s(dest, destSize, instance);
        wcscat_s(dest, destSize, L")");
    }
    wcscat_s(dest, destSize, L"\\");
    wcscat_s(dest, destSize, counter);
}

PDH_STATUS PerfReader::openQuery()
{
    PDH_STATUS status = PdhOpenQueryW(NULL, 0, &query);
    if (status != ERROR_SUCCESS) { WPRINT_DEBUG(L"PdhOpenQueryW failed with 0x" << std::hex << status << ".\n"); }
    else { PRINT_DEBUG("Query opened."); }
    return status;
}

PDH_STATUS PerfReader::addCounter(PWSTR path, PDH_HCOUNTER* counter)
{
    PDH_STATUS status = PdhAddCounterW(query, path, 0, counter);
    if (status == ERROR_SUCCESS) { WPRINT_DEBUG(L"Added counter for path \"" << path << "\"."); }
    else {
        WPRINT_DEBUG(L"PdhAddCounterW for path \"" << path << "\" failed with 0x" << std::hex << status << ".");
        *counter = NULL;
    }
    return status;
}
