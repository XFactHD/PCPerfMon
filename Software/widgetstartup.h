#ifndef WIDGETSTARTUP_H
#define WIDGETSTARTUP_H

#include <QDialog>

namespace Ui {
    class WidgetStartup;
}

class WidgetStartup : public QDialog
{
    Q_OBJECT

public:
    explicit WidgetStartup(bool startInTray, QWidget *parent = nullptr);
    ~WidgetStartup();

    void pushProgress(int amount);

private:
    Ui::WidgetStartup *ui;
};

#endif // WIDGETSTARTUP_H
