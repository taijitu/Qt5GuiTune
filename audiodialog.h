#ifndef AUDIODIALOG_H
#define AUDIODIALOG_H

#include <QDialog>

namespace Ui {
class AudioDialog;
}

class AudioDialog : public QDialog
{
    Q_OBJECT

public:
    AudioDialog(QWidget *parent = 0);
    ~AudioDialog();

signals:
    void useAlsaSignal();
    void useArtsSignal();
    void useOSSSignal();
    void useJACKSignal();
    void alsaDeviceSignal(const QString &arg1);
    void ossDeviceSignal(const QString &arg1);

private slots:
 
    void on_ossDeviceLineEdit_textChanged(const QString &arg1);

    void on_alsaDeviceLineEdit_textChanged(const QString &arg1);

    void on_alsaRadioButton_clicked();

    void on_artsRadioButton_clicked();

    void on_ossRadioButton_clicked();

    void on_jackRadioButton_clicked();

private:
    Ui::AudioDialog *ui;

};

#endif // AUDIODIALOG_H
