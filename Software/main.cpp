#include "mainwindow.h"

#include <QApplication>

QFile logFile;

void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    char text[1024];
    switch (type) {
        case QtDebugMsg:
            sprintf_s(text, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtInfoMsg:
            sprintf_s(text, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtWarningMsg:
            sprintf_s(text, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtCriticalMsg:
            sprintf_s(text, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtFatalMsg:
            sprintf_s(text, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
    }

#ifdef INDEV
    std::cout << text << std::flush;
#endif
    logFile.write(text, strlen(text));
    logFile.flush();
}

int main(int argc, char *argv[])
{
    //Try to create mutex
    HANDLE hMutexHandle = CreateMutex(NULL, TRUE, L"pcperfmon.singleinstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        std::cout << "Only one instance is allowed!" << std::endl;
        CloseHandle(hMutexHandle);
        return 1;
    }

    //Redirect console output to file
    logFile.setFileName(QDir::homePath() + "/pcperfmon_log.txt");
    logFile.open(QFile::WriteOnly);
    qInstallMessageHandler(msgHandler);

    //Create and show window
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));

    MainWindow w;
    if(app.arguments().contains("--startInTray")) {
        w.showMinimized();
    }
    else {
        w.show();
    }

    app.connect(&app, &QApplication::aboutToQuit, &w, &MainWindow::on_app_aboutToQuit);

    //Start application
    int retVal = app.exec();

    //Destroy mutex
    ReleaseMutex(hMutexHandle);
    CloseHandle(hMutexHandle);

    return retVal;
}
