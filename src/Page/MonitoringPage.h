#ifndef MONITORINGPAGE_H
#define MONITORINGPAGE_H

#include <QWidget>

#include "Module/ComModule.h"
#include "Monitoring/ComputerMonitoring.h"
#include "Monitoring/SelfMonitoring.h"
#include "../externals/QCustomPlot/qcustomplot.h"

namespace Ui {
class MonitoringPage;
}

class MonitoringPage : public QWidget
{
    Q_OBJECT

public:
    explicit MonitoringPage(QWidget *parent = 0);
    ~MonitoringPage();

    QVector<double> generateXAxis(int items);

public slots:
    void updateComputer();
    void updateSelf();

private slots:
    void on_diskBox_currentIndexChanged(int index);

private:
    void initializeComputerGraphs();
    void initializeSelfGraphs();
    void initializeSelfInfos();
    void swapDiskGraph(Disk disk);
    Ui::MonitoringPage *ui;

    int _currentComputerTick;
    int _currentSelfTick;
    bool _update;
};

#endif // MONITORINGPAGE_H
