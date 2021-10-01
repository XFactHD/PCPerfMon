#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(bool startInTray, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), sysTrayIcon(this), perf(this), timer(this), display(this), serialStatus(this)
{
    WidgetStartup* startup = new WidgetStartup(startInTray);

    monoFont = QFont("Monospaced");
    monoFont.setStyleHint(QFont::Monospace);
    setFont(monoFont);
    startup->pushProgress(5);

    ui->setupUi(this);
    startup->pushProgress(6);

    connect(&server, &QLocalServer::newConnection, this, &MainWindow::newIPCConnection);
    server.listen("pcperfmon");
    startup->pushProgress(6);

    //Disable window resizing and maximizing
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    startup->pushProgress(5);

    //Set window icon
    setWindowIcon(QIcon("icon.ico"));
    startup->pushProgress(5);

    //Setup sys tray icon
    sysTrayIcon.setIcon(windowIcon());
    sysTrayIcon.setToolTip("PCPerfMon");
    connect(&sysTrayIcon, &QSystemTrayIcon::activated, this, &MainWindow::sysTrayIconActivated);
    sysTrayIcon.show();
    startup->pushProgress(6);

    //Setup sys tray menu
    sysTrayMenu = new QMenu(this);
    sysTrayMenu->addAction("Show", this, SLOT(unhideWindow()));
    sysTrayMenu->addAction("Restart COM", &display, SLOT(restartCOM()));
    sysTrayMenu->addAction("Exit", this, SLOT(close()));
    sysTrayIcon.setContextMenu(sysTrayMenu);
    startup->pushProgress(6);

    //Configure application details
    QCoreApplication::setOrganizationName("dc");
    QCoreApplication::setApplicationName("PCPerfMon");
    QSettings settings;
    startup->pushProgress(5);

    //Configure color palettes
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::black);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(63, 63, 63));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    darkProgressBarPalette = QPalette(darkPalette);
    darkProgressBarPalette.setColor(QPalette::Highlight, QColor(0, 210, 0));

    lightProgressBarPalette.setColor(QPalette::Highlight, QColor(0, 210, 0));
    lightProgressBarPalette.setColor(QPalette::HighlightedText, Qt::black);
    startup->pushProgress(6);

    //Set configured app style
    darkMode = settings.value("app_dark_mode").toBool();
    configureAppStyle(darkMode);
    startup->pushProgress(6);

    //Create charts
    createCPUChart();
    createRAMChart();
    createNetChart();
    createGPUChart();
    startup->pushProgress(6);

    //Configure mouse event transparency
    ui->label_cpu->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->progressBar_cpu->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_ram->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->progressBar_ram->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_net->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->progressBar_netIn->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->progressBar_netOut->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_netIn->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_netOut->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->label_gpu->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->progressBar_gpuLoad->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->progressBar_gpuVram->setAttribute(Qt::WA_TransparentForMouseEvents);
    startup->pushProgress(5);

    //Add custom meta types
    qRegisterMetaType<cpu_info_t>("cpu_info_t");
    qRegisterMetaType<ram_info_t>("ram_info_t");
    qRegisterMetaType<net_info_t>("net_info_t");
    qRegisterMetaType<gpu_info_t>("gpu_info_t");
    startup->pushProgress(5);

    //Start components and connect signals
    perf.startAndWaitUntilReady();
    connect(&perf, &PerfReader::perfdataReady, this, &MainWindow::perfdataReady);
    startup->pushProgress(6);

    serialStatus.setText("Serial: Disconnected - Port: [Invalid] - Baudrate: -1");
    serialStatus.setContentsMargins(0, 0, 10, 0);
    ui->statusBar->addPermanentWidget(&serialStatus);
    startup->pushProgress(6);

    display.init();
    connect(&perf, &PerfReader::perfdataReady, &display, &DisplayHandler::perfdataReady);
    connect(&display, &DisplayHandler::serialTimedOut, this, &MainWindow::handleSerialTimedOut);
    showSerialStatus();
    showTimeoutNotif = settings.value("show_timeout_notification").toBool();
    startup->pushProgress(6);

    connect(ui->menuMain->actions().at(0), &QAction::triggered, this, &MainWindow::menuOpenSettings);
    connect(ui->menuMain->actions().at(1), &QAction::triggered, &display, &DisplayHandler::restartCOM);
    connect(ui->menuMain->actions().at(2), &QAction::triggered, this, &QCoreApplication::quit);
    startup->pushProgress(5);

    connect(&timer, &QTimer::timeout, &perf, &PerfReader::queryNewData);
    timer.start(1000);
    startup->pushProgress(5);
    startup->deleteLater();
}

void MainWindow::configureAppStyle(bool dark)
{
    if (dark) {
        setPalette(darkPalette);

        ui->menuMain->setPalette(darkPalette);

        QColor darkBg(63, 63, 63);
        setPlotColors(ui->plot_cpu, darkBg, Qt::white);
        setPlotColors(ui->plot_ram, darkBg, Qt::white);
        setPlotColors(ui->plot_net, darkBg, Qt::white);
        setPlotColors(ui->plot_gpu, darkBg, Qt::white);

        ui->progressBar_cpu->setPalette(darkProgressBarPalette);
        ui->progressBar_ram->setPalette(darkProgressBarPalette);
        ui->progressBar_netIn->setPalette(darkProgressBarPalette);
        ui->progressBar_netOut->setPalette(darkProgressBarPalette);
        ui->progressBar_gpuLoad->setPalette(darkProgressBarPalette);
        ui->progressBar_gpuVram->setPalette(darkProgressBarPalette);
    }
    else {
        setPalette(lightPalette);

        ui->menuMain->setPalette(lightPalette);

        setPlotColors(ui->plot_cpu, Qt::white, Qt::black);
        setPlotColors(ui->plot_ram, Qt::white, Qt::black);
        setPlotColors(ui->plot_net, Qt::white, Qt::black);
        setPlotColors(ui->plot_gpu, Qt::white, Qt::black);

        ui->progressBar_cpu->setPalette(lightProgressBarPalette);
        ui->progressBar_ram->setPalette(lightProgressBarPalette);
        ui->progressBar_netIn->setPalette(lightProgressBarPalette);
        ui->progressBar_netOut->setPalette(lightProgressBarPalette);
        ui->progressBar_gpuLoad->setPalette(lightProgressBarPalette);
        ui->progressBar_gpuVram->setPalette(lightProgressBarPalette);
    }
}

void MainWindow::setPlotColors(QCustomPlot* plot, QColor background, QColor foreground)
{
    plot->setBackground(QBrush(background));
    plot->xAxis->setBasePen(QPen(foreground));
    plot->xAxis->setLabelColor(foreground);
    plot->xAxis->setTickPen(QPen(foreground));
    plot->xAxis->setSubTickPen(QPen(foreground));
    plot->xAxis->setTickLabelColor(foreground);
    plot->yAxis->setBasePen(QPen(foreground));
    plot->yAxis->setLabelColor(foreground);
    plot->yAxis->setTickPen(QPen(foreground));
    plot->yAxis->setSubTickPen(QPen(foreground));
    plot->yAxis->setTickLabelColor(foreground);
}

void MainWindow::createCPUChart()
{
    ui->plot_cpu->xAxis->setRange(0, 60, Qt::AlignLeft);
    ui->plot_cpu->yAxis->setRange(0, 100);

    ui->plot_cpu->axisRect()->setAutoMargins(QCP::msLeft | QCP::msTop | QCP::msRight);
    ui->plot_cpu->axisRect()->setMargins(QMargins(0, 0, 0, 24));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->plot_cpu->xAxis->setTicker(timeTicker);

    QSharedPointer<FixedWidthTicker> yTicker(new FixedWidthTicker(4));
    ui->plot_cpu->yAxis->setTicker(yTicker);
    ui->plot_cpu->yAxis->setLabelFont(monoFont);

    ui->plot_cpu->addGraph();
    ui->plot_cpu->graph(0)->setName("CPU Load (%)");

    ui->plot_cpu->addGraph();
    ui->plot_cpu->graph(1)->setPen(QPen(Qt::red));
    ui->plot_cpu->graph(1)->setName("CPU Temp (°C)");

    ui->plot_cpu->legend->setBrush(QBrush(QColor(255,255,255,200)));
    ui->plot_cpu->legend->setVisible(true);

    ui->plot_cpu->xAxis->setTickLabelFont(monoFont);
    ui->plot_cpu->yAxis->setTickLabelFont(monoFont);
}

void MainWindow::createRAMChart()
{
    ui->plot_ram->xAxis->setRange(0, 60, Qt::AlignLeft);
    ui->plot_ram->yAxis->setRange(0, 100);

    ui->plot_ram->axisRect()->setAutoMargins(QCP::msLeft | QCP::msTop | QCP::msRight);
    ui->plot_ram->axisRect()->setMargins(QMargins(0, 0, 0, 24));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->plot_ram->xAxis->setTicker(timeTicker);

    QSharedPointer<FixedWidthTicker> yTicker(new FixedWidthTicker(4));
    ui->plot_ram->yAxis->setTicker(yTicker);
    ui->plot_ram->yAxis->setLabelFont(monoFont);

    ui->plot_ram->addGraph();
    ui->plot_ram->graph(0)->setName("RAM Load (%)");

    ui->plot_ram->legend->setBrush(QBrush(QColor(255,255,255,200)));
    ui->plot_ram->legend->setVisible(true);

    ui->plot_ram->xAxis->setTickLabelFont(monoFont);
    ui->plot_ram->yAxis->setTickLabelFont(monoFont);

    ui->plot_ram->setVisible(false);
}

void MainWindow::createNetChart()
{
    ui->plot_net->xAxis->setRange(0, 60, Qt::AlignLeft);
    ui->plot_net->yAxis->setRange(0, 100);

    ui->plot_net->axisRect()->setAutoMargins(QCP::msLeft | QCP::msTop | QCP::msRight);
    ui->plot_net->axisRect()->setMargins(QMargins(0, 0, 0, 24));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->plot_net->xAxis->setTicker(timeTicker);

    QSharedPointer<NetTicker> netTicker(new NetTicker());
    ui->plot_net->yAxis->setTicker(netTicker);
    ui->plot_net->yAxis->setLabelFont(monoFont);

    ui->plot_net->addGraph();
    ui->plot_net->graph(0)->setPen(QPen(Qt::red));
    ui->plot_net->graph(0)->setName("Download (Kbit/s)");

    ui->plot_net->addGraph();
    ui->plot_net->graph(1)->setPen(QPen(Qt::green));
    ui->plot_net->graph(1)->setName("Upload (Kbit/s)");

    ui->plot_net->legend->setBrush(QBrush(QColor(255,255,255,200)));
    ui->plot_net->legend->setVisible(true);

    ui->plot_net->xAxis->setTickLabelFont(monoFont);
    ui->plot_net->yAxis->setTickLabelFont(monoFont);

    ui->plot_net->setVisible(false);
}

void MainWindow::createGPUChart()
{
    ui->plot_gpu->xAxis->setRange(0, 60, Qt::AlignLeft);
    ui->plot_gpu->yAxis->setRange(0, 100);

    ui->plot_gpu->axisRect()->setAutoMargins(QCP::msLeft | QCP::msTop | QCP::msRight);
    ui->plot_gpu->axisRect()->setMargins(QMargins(0, 0, 0, 24));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->plot_gpu->xAxis->setTicker(timeTicker);

    QSharedPointer<FixedWidthTicker> yTicker(new FixedWidthTicker(4));
    ui->plot_gpu->yAxis->setTicker(yTicker);
    ui->plot_gpu->yAxis->setLabelFont(monoFont);

    ui->plot_gpu->addGraph();
    ui->plot_gpu->graph(0)->setName("GPU Load (%)");

    ui->plot_gpu->addGraph();
    ui->plot_gpu->graph(1)->setPen(QPen(Qt::green));
    ui->plot_gpu->graph(1)->setName("VRAM Load (%)");

    ui->plot_gpu->addGraph();
    ui->plot_gpu->graph(2)->setPen(QPen(Qt::red));
    ui->plot_gpu->graph(2)->setName("GPU Temp (°C)");

    ui->plot_gpu->legend->setBrush(QBrush(QColor(255,255,255,200)));
    ui->plot_gpu->legend->setVisible(true);

    ui->plot_gpu->xAxis->setTickLabelFont(monoFont);
    ui->plot_gpu->yAxis->setTickLabelFont(monoFont);

    ui->plot_gpu->setVisible(false);
}

void MainWindow::perfdataReady(cpu_info_t cpuInfo, ram_info_t ramInfo, net_info_t netInfo, gpu_info_t gpuInfo)
{
    //The first data point must not be displayed
    static bool first = true;
    if(first) {
        first = false;

        ui->progressBar_netIn->setRange(0, netInfo.netIn);
        ui->progressBar_netOut->setRange(0, netInfo.netOut);

        return;
    }

    constexpr double multiplier = 1; //1 for 1 second interval, 0.5 for half second interval
    static uint64_t idx = 0;

    double timeStamp = ((double)idx) * multiplier;

    //Display CPU data
    int cpuLoad = std::round(cpuInfo.cpuLoad * 100.0f);
    ui->progressBar_cpu->setValue(cpuLoad);
    ui->plot_cpu->graph(0)->addData(timeStamp, cpuLoad);
    ui->plot_cpu->graph(1)->addData(timeStamp, cpuInfo.cpuTemp);

    //Display RAM data
    ui->progressBar_ram->setValue(ramInfo.ramLoad);
    QString ramText = QString::number(ramInfo.ramUsed / 1073741824.0, 'f', 1) + "/" + QString::number(ramInfo.ramTotal / 1073741824.0, 'f', 1) + " GB (%p%)";
    ui->progressBar_ram->setFormat(ramText);
    ui->plot_ram->graph(0)->addData(timeStamp, ramInfo.ramLoad);

    //Display network data
    QString netInText = formatScientific(netInfo.netIn, 1, { "KBit/s", "MBit/s", "GBit/s" });
    QString netOutText = formatScientific(netInfo.netOut, 1, { "KBit/s", "MBit/s", "GBit/s" });
    ui->label_netIn->setText(netInText);
    ui->label_netOut->setText(netOutText);
    ui->progressBar_netIn->setValue(netInfo.netIn);
    ui->progressBar_netOut->setValue(netInfo.netOut);
    ui->plot_net->graph(0)->addData(timeStamp, netInfo.netIn);
    ui->plot_net->graph(1)->addData(timeStamp, netInfo.netOut);
    double max = findHighest(ui->plot_net->graph(0)->data(), ui->plot_net->graph(1)->data(), 60 / multiplier);
    if(max < 100.0) { max = 100.0; }
    ui->plot_net->yAxis->setRange(0, max);

    //Display GPU data
    ui->progressBar_gpuLoad->setValue(gpuInfo.gpuLoad);
    ui->progressBar_gpuVram->setValue(gpuInfo.vramLoad);
    QString vramText = QString::number(gpuInfo.vramUsed / 1073741824.0, 'f', 1) + "/" + QString::number(gpuInfo.vramTotal / 1073741824.0, 'f', 1) + " GB (%p%)";
    ui->progressBar_gpuVram->setFormat(vramText);
    ui->plot_gpu->graph(0)->addData(timeStamp, gpuInfo.gpuLoad);
    ui->plot_gpu->graph(1)->addData(timeStamp, gpuInfo.vramLoad);
    ui->plot_gpu->graph(2)->addData(timeStamp, gpuInfo.gpuTemp);

    //Advance x axis if necessary
    if(idx >= 60 / multiplier) {
        ui->plot_cpu->xAxis->setRange(timeStamp, 60, Qt::AlignRight);
        ui->plot_ram->xAxis->setRange(timeStamp, 60, Qt::AlignRight);
        ui->plot_net->xAxis->setRange(timeStamp, 60, Qt::AlignRight);
        ui->plot_gpu->xAxis->setRange(timeStamp, 60, Qt::AlignRight);
    }

    //Redraw graphs (only when the window is open and only the visible graph)
    if(!isHidden()) {
        if(ui->plot_cpu->isVisible()) { ui->plot_cpu->replot(); }
        if(ui->plot_ram->isVisible()) { ui->plot_ram->replot(); }
        if(ui->plot_net->isVisible()) { ui->plot_net->replot(); }
        if(ui->plot_gpu->isVisible()) { ui->plot_gpu->replot(); }
    }

    showSerialStatus();

    idx++;
}

void MainWindow::showSerialStatus()
{
    const char* connected = display.isConnected() ? "Connected" : "Disconnected";
    QString text = QString("Serial: %1 | Port: %2 | Baudrate: %3").arg(connected, 12).arg(display.getComPort(), 5).arg(display.getBaudrate(), 10);
    serialStatus.setText(text);
}

MainWindow::~MainWindow()
{
    delete sysTrayMenu;
    delete ui;
}



void MainWindow::on_pushButton_cpu_clicked()
{
    ui->pushButton_cpu->setChecked(true);
    ui->pushButton_ram->setChecked(false);
    ui->pushButton_net->setChecked(false);
    ui->pushButton_gpu->setChecked(false);

    ui->plot_cpu->setVisible(true);
    ui->plot_ram->setVisible(false);
    ui->plot_net->setVisible(false);
    ui->plot_gpu->setVisible(false);

    ui->plot_cpu->replot();
}

void MainWindow::on_pushButton_ram_clicked()
{
    ui->pushButton_cpu->setChecked(false);
    ui->pushButton_ram->setChecked(true);
    ui->pushButton_net->setChecked(false);
    ui->pushButton_gpu->setChecked(false);

    ui->plot_cpu->setVisible(false);
    ui->plot_ram->setVisible(true);
    ui->plot_net->setVisible(false);
    ui->plot_gpu->setVisible(false);

    ui->plot_ram->replot();
}

void MainWindow::on_pushButton_net_clicked()
{
    ui->pushButton_cpu->setChecked(false);
    ui->pushButton_ram->setChecked(false);
    ui->pushButton_net->setChecked(true);
    ui->pushButton_gpu->setChecked(false);

    ui->plot_cpu->setVisible(false);
    ui->plot_ram->setVisible(false);
    ui->plot_net->setVisible(true);
    ui->plot_gpu->setVisible(false);

    ui->plot_net->replot();
}

void MainWindow::on_pushButton_gpu_clicked()
{
    ui->pushButton_cpu->setChecked(false);
    ui->pushButton_ram->setChecked(false);
    ui->pushButton_net->setChecked(false);
    ui->pushButton_gpu->setChecked(true);

    ui->plot_cpu->setVisible(false);
    ui->plot_ram->setVisible(false);
    ui->plot_net->setVisible(false);
    ui->plot_gpu->setVisible(true);

    ui->plot_gpu->replot();
}

void MainWindow::menuOpenSettings()
{
    DialogOptions options((QWidget*)parent());
    options.setPalette(darkMode ? darkPalette : lightPalette);

    connect(&options, &DialogOptions::setAppDarkMode, this, &MainWindow::setAppDarkMode);
    connect(&options, &DialogOptions::setDisplayDarkMode, &display, &DisplayHandler::setDisplayDarkMode);
    connect(&options, &DialogOptions::setDisplayBrightness, &display, &DisplayHandler::setDisplayBrightness);
    connect(&options, &DialogOptions::setShowTimeoutNotifications, this, &MainWindow::setShowTimeoutNotifications);

    options.exec();
}

void MainWindow::setAppDarkMode(bool dark)
{
    darkMode = dark;
    configureAppStyle(dark);
}

void MainWindow::aboutToQuit()
{
    display.shutdown();
    perf.requestShutdown();
    while(!perf.isFinished());
}

void MainWindow::newIPCConnection() {
    QLocalSocket* sock = server.nextPendingConnection();

    sock->waitForReadyRead();

    QString msg = sock->readAll();
    if (msg.compare("show") == 0) {
        unhideWindow();
    }

    sock->close();
    delete sock;
}

void MainWindow::handleSerialTimedOut() {
    if (showTimeoutNotif) {
        sysTrayIcon.showMessage(QApplication::applicationDisplayName(), "Serial connection to display timed out!", windowIcon());
    }
}

QString MainWindow::formatScientific(double value, int precision, QVector<QString> unit)
{
    int unitIdx = 0;
    for(int i = 1; i < unit.size(); i++) {
        if(value > 1000) {
            value /= 1000.0;
            unitIdx++;
        }
    }

    return QString::number(value, 'f', precision) + " " + unit[unitIdx];
}

double MainWindow::findHighest(QSharedPointer<QCPGraphDataContainer> data1, QSharedPointer<QCPGraphDataContainer> data2, int backCount)
{
    double max = 0;

    int offset = std::min(data1.get()->size(), backCount) + 1;
    for(int i = data1.get()->size() - offset; i < data1.get()->size(); i++) {
        double d = data1.get()->at(i)->value;
        if(d > max) { max = d; }
    }

    offset = std::min(data2.get()->size(), backCount) + 1;
    for(int i = data2.get()->size() - offset; i < data2.get()->size(); i++) {
        double d = data2.get()->at(i)->value;
        if(d > max) { max = d; }
    }

    return max;
}

void MainWindow::sysTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger) {
        emit sysTrayIcon.activated(QSystemTrayIcon::Context);
    }
    else if(reason == QSystemTrayIcon::DoubleClick) {
        unhideWindow();
    }
}

void MainWindow::unhideWindow()
{
    if(isHidden()) {
        showNormal();
        raise();
        setFocus();
        activateWindow();

        if(ui->plot_cpu->isVisible()) { ui->plot_cpu->replot(); }
        if(ui->plot_ram->isVisible()) { ui->plot_ram->replot(); }
        if(ui->plot_net->isVisible()) { ui->plot_net->replot(); }
        if(ui->plot_gpu->isVisible()) { ui->plot_gpu->replot(); }
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized()) {
            QTimer::singleShot(0, this, SLOT(hide()));
        }
    }
}
