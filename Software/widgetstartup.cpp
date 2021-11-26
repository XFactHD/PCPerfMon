#include "widgetstartup.h"
#include "ui_widgetstartup.h"

WidgetStartup::WidgetStartup(int stepCount, bool startInTray, QWidget *parent) :
    QDialog(parent), ui(new Ui::WidgetStartup), _stepSize(100.f / (float)stepCount)
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

void WidgetStartup::initial(QString message)
{
    ui->label_message->setText(message);
}

void WidgetStartup::step(QString message)
{
    _currStep++;
    int value = ceil(_stepSize * (float)_currStep);
    ui->progressBar->setValue(value);
    ui->label_message->setText(message);

    QEventLoop().processEvents();
}
