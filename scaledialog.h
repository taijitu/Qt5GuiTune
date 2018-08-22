#ifndef SCALEDIALOG_H
#define SCALEDIALOG_H

#include <QDialog>

namespace Ui {
class ScaleDialog;
}

class ScaleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScaleDialog(QWidget *parent = 0);
    ~ScaleDialog();

signals:
    void scaleChanged(int);

private slots:

    void on_scaleComboBox_currentIndexChanged(int index);

private:
    Ui::ScaleDialog *ui;
};

#endif // SCALEDIALOG_H
