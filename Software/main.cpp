#include "mainwindow.h"

#include <QApplication>
#include <QLocalSocket>

QFile logFile;

void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    (void)context;
    QByteArray localMsg = msg.toLocal8Bit();
    QByteArray time = QTime::currentTime().toString("hh:mm:ss.zzz").toLocal8Bit();

    char text[1024];
    switch (type) {
        case QtDebugMsg:
            sprintf_s(text, "[%s][Debug]: %s\n", time.constData(), localMsg.constData());
            break;
        case QtInfoMsg:
            sprintf_s(text, "[%s][Info]: %s\n", time.constData(), localMsg.constData());
            break;
        case QtWarningMsg:
            sprintf_s(text, "[%s][Warning]: %s\n", time.constData(), localMsg.constData());
            break;
        case QtCriticalMsg:
            sprintf_s(text, "[%s][Critical]: %s\n", time.constData(), localMsg.constData());
            break;
        case QtFatalMsg:
            sprintf_s(text, "[%s][Fatal]: %s\n", time.constData(), localMsg.constData());
            break;
    }

#ifdef INDEV
    std::cout << text << std::flush;
#endif
    logFile.write(text, strlen(text));
    logFile.flush();
}

int checkSingleInstance()
{
    QLocalSocket socket;
    socket.connectToServer("pcperfmon");
    if (socket.waitForConnected()) {
        socket.write("show");
        socket.waitForBytesWritten();
        socket.disconnectFromServer();
        return 1;
    }
    else {
        //ServerNotFoundError means no other instance is running
        if (socket.error() != QLocalSocket::ServerNotFoundError) {
            std::cout << socket.errorString().toStdString() << std::endl;
            return 2;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int notSingle = checkSingleInstance();
    if (notSingle) { return notSingle; }

    //Redirect console output to file
    logFile.setFileName(QDir::homePath() + "/pcperfmon_log.txt");
    logFile.open(QFile::WriteOnly);
    qInstallMessageHandler(msgHandler);

    //Create and show window
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));

    bool startInTray = app.arguments().contains("--startInTray");
    MainWindow w(startInTray);
    if(startInTray) {
        w.showMinimized();
    }
    else {
        w.show();
    }

    app.connect(&app, &QApplication::aboutToQuit, &w, &MainWindow::aboutToQuit);

    //Start application
    return app.exec();
}
