#include "widgetstartup.h"
#include "ui_widgetstartup.h"

WidgetStartup::WidgetStartup(bool startInTray, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WidgetStartup)
{
    ui->setupUi(this);

    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);

    if (!startInTray) {
        show();
        QEventLoop().processEvents();
    }
}

WidgetStartup::~WidgetStartup()
{
    delete ui;
}

void WidgetStartup::pushProgress(int amount)
{
    ui->progressBar->setValue(ui->progressBar->value() + amount);
    QEventLoop().processEvents();
}
