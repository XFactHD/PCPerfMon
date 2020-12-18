#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    //Try to create mutex
    HANDLE hMutexHandle = CreateMutex(NULL, TRUE, L"pcperfmon.singleinstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        std::cout << "Only one instance is allowed!" << std::endl;
        CloseHandle(hMutexHandle);
        return 1;
    }

    //Create and show window
    QApplication app(argc, argv);
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
