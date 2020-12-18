#ifndef OHMWRAPPER_H
#define OHMWRAPPER_H

#include <QApplication>
#include <QProcess>
#include <iostream>

class OHMWrapper
{
public:
    OHMWrapper();
    ~OHMWrapper() { delete ohm; }

    void init() {}
    void shutdown();

    void update() {}

    uint16_t getCpuClockAvg();
    uint16_t getCpuPkgTemp();
    uint32_t getCpuPkgPower();

private:
    QProcess* ohm;
    bool ready;
};

#endif // OHMWRAPPER_H
