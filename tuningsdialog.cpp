#include "tuningsdialog.h"
#include "ui_tuningsdialog.h"


TuningsDialog::TuningsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TuningsDialog)
{
    ui->setupUi(this);
    ui->naturalRadioButton->click();
    ui->normalRadioButton->click();
}

TuningsDialog::~TuningsDialog()
{
    delete ui;
}

void TuningsDialog::on_normalRadioButton_clicked()
{
   emit normalSignal();
}

void TuningsDialog::on_physicalRadioButton_clicked()
{
   emit physicalSignal();
}

void TuningsDialog::on_wienRadioButton_clicked()
{
    emit wienSignal();
}

void TuningsDialog::on_naturalRadioButton_clicked()
{
    emit naturalModeSignal();
}

void TuningsDialog::on_equidistantRadioButton_clicked()
{
    emit equidistantModeSignal();
}
