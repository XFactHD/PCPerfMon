#ifndef WIDGETSTARTUP_H
#define WIDGETSTARTUP_H

#include <QDialog>
#include <cmath>

namespace Ui {
    class WidgetStartup;
}

class WidgetStartup : public QDialog
{
    Q_OBJECT

public:
    explicit WidgetStartup(int stepCount, bool startInTray, QWidget *parent = nullptr);
    ~WidgetStartup();

    void initial(QString message);
    void step(QString message);

public slots:
    void subStep(QString message);

private:
    Ui::WidgetStartup *ui;
    float _stepSize;
    int _currStep = 0;
};

#endif // WIDGETSTARTUP_H
