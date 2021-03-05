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
