#ifndef DIALOGCONFIGSOFT_H
#define DIALOGCONFIGSOFT_H

#include <QDialog>

#include "MainWindow.h"

namespace Ui {
class DialogConfigSoft;
}
/**
 * @brief The DialogConfigSoft class QDialog allowing the user to see and modify the software values/constants such as PATH, or default values.
 */
class DialogConfigSoft : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief DialogConfigSoft Constrcuts a QDialog with a parent.
     * @param parent
     */
    explicit DialogConfigSoft(QWidget *parent = 0);
    ~DialogConfigSoft();

    void updateValues();

private slots:

    void on_close_clicked();
    /**
     * @brief on_validate_clicked Called when the user click on update/apply button. It's just...apply and save the changes.
     */
    void on_validate_clicked();

private:
    /**
     * @brief ui Graphical layer.
     */
    Ui::DialogConfigSoft *ui;
};

#endif // DIALOGCONFIGSOFT_H
