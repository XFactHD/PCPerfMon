#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QLocalServer>
#include <QLocalSocket>

#include <iostream>

#include "qcustomplot.h"
#include "perfreader.h"
#include "displayhandler.h"
#include "dialogoptions.h"
#include "widgetstartup.h"

//#define DEBUG

#ifdef DEBUG
#define PRINT_DEBUG(msg) std::cout << msg << std::endl
#define WPRINT_DEBUG(msg) std::wcout << msg << std::endl
#else
#define PRINT_DEBUG(msg)
#define WPRINT_DEBUG(msg)
#endif

#define PLOT_X_WIDTH 60 //60 seconds
#define ONE_GIGABYTE 1073741824.0

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(bool startInTray, QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void setAppDarkMode(bool dark);

    void aboutToQuit();

    void unhideWindow();

private slots:
    void perfdataReady(cpu_info_t cpuInfo, ram_info_t ramInfo, net_info_t netInfo, gpu_info_t gpuInfo);

    void on_pushButton_cpu_clicked();

    void on_pushButton_ram_clicked();

    void on_pushButton_net_clicked();

    void on_pushButton_gpu_clicked();

    void menuOpenSettings();

    void sysTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

    void newIPCConnection();

    void setShowTimeoutNotifications(bool value) { showTimeoutNotif = value; }

    void handleSerialTimedOut();

protected:
    virtual void changeEvent(QEvent* event);

private:
    void configureAppStyle(bool dark);
    void setPlotColors(QCustomPlot* plot, QColor background, QColor foreground);
    void createCPUChart();
    void createRAMChart();
    void createNetChart();
    void createGPUChart();

    void showSerialStatus();
    QString formatScientific(double value, int precision, QVector<QString> unit);
    double findHighest(QSharedPointer<QCPGraphDataContainer> data1, QSharedPointer<QCPGraphDataContainer> data2, int backCount);

    Ui::MainWindow *ui;
    QLocalServer server;
    QSystemTrayIcon sysTrayIcon;
    QMenu* sysTrayMenu;

    QFont monoFont;
    QPalette lightPalette;
    QPalette lightProgressBarPalette;
    QPalette darkPalette;
    QPalette darkProgressBarPalette;
    bool darkMode;
    bool showTimeoutNotif;
    uint64_t dataIdx = 0;

    PerfReader perf;
    QTimer timer;
    DisplayHandler display;
    QLabel serialStatus;
};

class NetTicker : public QCPAxisTicker
{
    virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) Q_DECL_OVERRIDE {
        (void)locale;
        (void)formatChar;
        (void)precision;

        QString mult = "K";
        if(tick >= 1000.0) { tick /= 1000.0; mult = "M"; }
        if(tick >= 1000.0) { tick /= 1000.0; mult = "G"; }
        if(tick >= 1000.0) { tick /= 1000.0; mult = "T"; }
        return QString("%1").arg(tick, 3, 'f', 0, ' ') + mult;
    }
};

class FixedWidthTicker : public QCPAxisTicker
{
public:
    FixedWidthTicker(int w) : w(w) {}

    virtual QString getTickLabel(double tick, const QLocale &locale, QChar formatChar, int precision) Q_DECL_OVERRIDE {
        QString text = locale.toString(tick, formatChar.toLatin1(), precision);
        text = QString("%1").arg(text, w);
        return text;
    }

private:
    int w;
};

#endif // MAINWINDOW_H
