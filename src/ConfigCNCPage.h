#ifndef CONFIGCNCPAGE_H
#define CONFIGCNCPAGE_H

#include <QWidget>

namespace Ui {
class ConfigCNCPage;
}

class ConfigCNCPage : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigCNCPage(QWidget *parent = 0);
    ~ConfigCNCPage();

private:
    Ui::ConfigCNCPage *ui;
};

#endif // CONFIGCNCPAGE_H
