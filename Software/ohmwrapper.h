#ifndef OHMWRAPPER_H
#define OHMWRAPPER_H

#include <QApplication>
#include <QProcess>
#include <iostream>

class OHMWrapper : public QObject
{
    Q_OBJECT
public:
    explicit OHMWrapper(QObject* parent = nullptr) : QObject(parent) {}

    void init();
    void shutdown();

    void update() {}

    uint16_t getCpuClockAvg();
    uint16_t getCpuPkgTemp();
    uint32_t getCpuPkgPower();

private slots:
    void stderrReadyRead();

private:
    QProcess* ohm;
    bool ready;
};

#endif // OHMWRAPPER_H
