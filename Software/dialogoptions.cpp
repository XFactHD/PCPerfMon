#include "dialogoptions.h"
#include "ui_dialogoptions.h"
#include <iostream>

DialogOptions::DialogOptions(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOptions)
{
    ui->setupUi(this);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    QSettings settings;
    if(settings.contains("drive_display")) {
        ui->checkBox_active->setChecked(settings.value("drive_display").toBool());
    }
    if(settings.contains("com_port")) {
        ui->lineEdit_com_port->setText(settings.value("com_port").toString());
    }
    if(settings.contains("baudrate")) {
        ui->lineEdit_baudrate->setText(QString::number(settings.value("baudrate").toUInt()));
    }
    if(settings.contains("app_dark_mode")) {
        ui->checkBox_app_dark->setChecked(settings.value("app_dark_mode").toBool());
    }
    if(settings.contains("display_dark_mode")) {
        ui->checkBox_disp_dark->setChecked(settings.value("display_dark_mode").toBool());
    }
    if(settings.contains("display_brightness")) {
        ui->slider_brightness->setValue(settings.value("display_brightness").toInt());
    }
    if(settings.contains("display_timeout")) {
        ui->spinBox_timeout->setValue(settings.value("display_timeout").toInt());
    }
    if(settings.contains("show_timeout_notification")) {
        ui->checkBox_timeout_notif->setChecked(settings.value("show_timeout_notification").toBool());
    }
}

DialogOptions::~DialogOptions()
{
    delete ui;
}

void DialogOptions::on_DialogOptions_finished(int result)
{
    if(result == 1) {
        QSettings settings;
        settings.setValue("drive_display", ui->checkBox_active->isChecked());
        settings.setValue("com_port", ui->lineEdit_com_port->text());
        settings.setValue("baudrate", ui->lineEdit_baudrate->text().toUInt());

        bool appDark = ui->checkBox_app_dark->isChecked();
        settings.setValue("app_dark_mode", appDark);
        emit setAppDarkMode(appDark);

        bool dispDark = ui->checkBox_disp_dark->isChecked();
        settings.setValue("display_dark_mode", dispDark);
        emit setDisplayDarkMode(dispDark);

        int brightness = ui->slider_brightness->value();
        settings.setValue("display_brightness", brightness);
        emit setDisplayBrightness(brightness);

        int timeout = ui->spinBox_timeout->value();
        settings.setValue("display_timeout", timeout);
        emit setDisplayTimeout(timeout);

        bool timeoutNotifs = ui->checkBox_timeout_notif->isChecked();
        settings.setValue("show_timeout_notification", timeoutNotifs);
        emit setShowTimeoutNotifications(timeoutNotifs);
    }
}

void DialogOptions::on_lineEdit_com_port_textEdited(const QString &arg1)
{
    (void)arg1;

    bool valid = ui->lineEdit_com_port->hasAcceptableInput() && ui->lineEdit_baudrate->hasAcceptableInput();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

void DialogOptions::on_lineEdit_baudrate_textEdited(const QString &arg1)
{
    (void)arg1;

    bool valid = ui->lineEdit_com_port->hasAcceptableInput() && ui->lineEdit_baudrate->hasAcceptableInput();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}
