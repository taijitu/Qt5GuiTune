#include "audiodialog.h"
#include "ui_audiodialog.h"

AudioDialog::AudioDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AudioDialog)
{
    ui->setupUi(this);
    ui->autoDetectRadioButton->click();
    ui->alsaDeviceLineEdit->setText("default");
}

AudioDialog::~AudioDialog()
{
}


void AudioDialog::on_ossDeviceLineEdit_textChanged(const QString &arg1)
{
    emit ossDeviceSignal(arg1);
}


void AudioDialog::on_alsaDeviceLineEdit_textChanged(const QString &arg1)
{
    emit alsaDeviceSignal(arg1);
}

void AudioDialog::on_alsaRadioButton_clicked()
{
    emit useAlsaSignal();
}

void AudioDialog::on_artsRadioButton_clicked()
{
    emit useArtsSignal();
}

void AudioDialog::on_ossRadioButton_clicked()
{
    emit useOSSSignal();
}

void AudioDialog::on_jackRadioButton_clicked()
{
    emit useJACKSignal();
}
