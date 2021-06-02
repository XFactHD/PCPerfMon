#ifndef DIALOGOPTIONS_H
#define DIALOGOPTIONS_H

#include <QDialog>

#include <QSettings>
#include <QPushButton>

namespace Ui {
class DialogOptions;
}

class DialogOptions : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOptions(QWidget *parent = nullptr);
    ~DialogOptions();

    Ui::DialogOptions* getUi() { return ui; }

signals:
    void setAppDarkMode(bool dark);
    void setDisplayDarkMode(bool dark);

private slots:
    void on_DialogOptions_finished(int result);

    void on_lineEdit_com_port_textEdited(const QString &arg1);

    void on_lineEdit_baudrate_textEdited(const QString &arg1);

private:
    Ui::DialogOptions *ui;
};

#endif // DIALOGOPTIONS_H
