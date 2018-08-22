#ifndef TUNINGSDIALOG_H
#define TUNINGSDIALOG_H

#include <QDialog>

namespace Ui {
class TuningsDialog;
}

class TuningsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TuningsDialog(QWidget *parent = 0);
    ~TuningsDialog();

signals:
    void normalSignal();
    void physicalSignal();
    void wienSignal();
    void naturalModeSignal();
    void equidistantModeSignal();

private slots:

    void on_normalRadioButton_clicked();

    void on_physicalRadioButton_clicked();

    void on_wienRadioButton_clicked();

    void on_naturalRadioButton_clicked();

    void on_equidistantRadioButton_clicked();

private:
    Ui::TuningsDialog *ui;
};

#endif // TUNINGSDIALOG_H
