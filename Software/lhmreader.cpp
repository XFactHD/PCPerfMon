#include "lhmreader.h"

void LHMReader::init()
{
#ifdef USE_VULNERABLE_DRIVER
    handle = LoadLibraryA("./lhm/LhmReaderLib.dll");
    if (!handle) {
        qWarning("[LHMWrapper] Failed to find LhmReaderLib");
        return;
    }

    lhm_init = (lhm_init_t)GetProcAddress(handle, "init");
    lhm_shutdown = (lhm_shutdown_t)GetProcAddress(handle, "shutdown");
    lhm_update = (lhm_update_t)GetProcAddress(handle, "update");
    lhm_getAvgCpuClock = (lhm_getAvgCpuClock_t)GetProcAddress(handle, "getAvgCpuClock");
    lhm_getAvgSecCpuClock = (lhm_getAvgSecCpuClock_t)GetProcAddress(handle, "getAvgSecCpuClock");
    lhm_getCpuPkgTemp = (lhm_getCpuPkgTemp_t)GetProcAddress(handle, "getCpuPkgTemperature");
    lhm_getCpuPkgPower = (lhm_getCpuPkgPower_t)GetProcAddress(handle, "getCpuPkgPower");
    lhm_getCpuCoreVoltage = (lhm_getCpuCoreVoltage_t)GetProcAddress(handle, "getCpuCoreVoltage");

    if (!lhm_init || !lhm_shutdown || !lhm_update || !lhm_getAvgCpuClock || !lhm_getAvgSecCpuClock || !lhm_getCpuPkgTemp || !lhm_getCpuPkgPower || !lhm_getCpuCoreVoltage) {
        qWarning("[LHMReader] Missing functions from LhmReaderLib, aborting");
        return;
    }

    if (!lhm_init()) {
        qWarning("[LHMReader] LhmReaderLib failed to start");
        return;
    }

    ready = true;
    qInfo("[LHMReader] LhmReader ready");
#else
    qInfo("[LHMReader] LhmReader not present");
#endif
}

void LHMReader::update()
{
    if (ready) {
        lhm_update();
    }
}

std::pair<uint16_t, uint16_t> LHMReader::getCpuClockAvg()
{
    if (!ready) { return std::pair<uint16_t, uint16_t>(0, 0); }

    return std::pair<uint16_t, uint16_t>(
        lhm_getAvgCpuClock(),
        lhm_getAvgSecCpuClock()
    );
}

uint16_t LHMReader::getCpuPkgTemp()
{
    if (!ready) { return 0; }

    return lhm_getCpuPkgTemp();
}

uint32_t LHMReader::getCpuPkgPower()
{
    if (!ready) { return 0; }

    return lhm_getCpuPkgPower();
}

uint32_t LHMReader::getCpuCoreVoltage()
{
    if (!ready) { return 0; }

    return lhm_getCpuCoreVoltage();
}

void LHMReader::shutdown()
{
    if (ready) {
        lhm_shutdown();
    }
}
