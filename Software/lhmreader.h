#ifndef LHMREADER_H
#define LHMREADER_H

#include <QObject>

#include "windows.h"

typedef bool (*lhm_init_t)(void);
typedef void (*lhm_shutdown_t)(void);
typedef void (*lhm_update_t)(void);
typedef uint32_t (*lhm_getAvgCpuClock_t)(void);
typedef uint32_t (*lhm_getAvgSecCpuClock_t)(void);
typedef uint32_t (*lhm_getCpuPkgTemp_t)(void);
typedef uint32_t (*lhm_getCpuPkgPower_t)(void);
typedef uint32_t (*lhm_getCpuCoreVoltage_t)(void);

class LHMReader : public QObject
{
    Q_OBJECT
public:
    explicit LHMReader(QObject* parent = nullptr) : QObject(parent) {}

    void init();
    void shutdown();

    void update();

    std::pair<uint16_t, uint16_t> getCpuClockAvg();
    uint16_t getCpuPkgTemp();
    uint32_t getCpuPkgPower();
    uint32_t getCpuCoreVoltage();

private:
    HINSTANCE handle;
    lhm_init_t lhm_init;
    lhm_shutdown_t lhm_shutdown;
    lhm_update_t lhm_update;
    lhm_getAvgCpuClock_t lhm_getAvgCpuClock;
    lhm_getAvgSecCpuClock_t lhm_getAvgSecCpuClock;
    lhm_getCpuPkgTemp_t lhm_getCpuPkgTemp;
    lhm_getCpuPkgPower_t lhm_getCpuPkgPower;
    lhm_getCpuCoreVoltage_t lhm_getCpuCoreVoltage;
    bool ready;
};

#endif // LHMREADER_H
