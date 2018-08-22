#include "scaledialog.h"
#include "ui_scaledialog.h"
#include "logview.h"

ScaleDialog::ScaleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScaleDialog)
{
    ui->setupUi(this);
    ui->scaleComboBox->addItem( tr("US"));
    ui->scaleComboBox->addItem( tr("US Alt"));
    ui->scaleComboBox->addItem( tr("German"));
    ui->scaleComboBox->addItem( tr("German Alt"));
    ui->scaleComboBox->addItem( tr("French"));
    ui->scaleComboBox->addItem( tr("French Alt"));

}

ScaleDialog::~ScaleDialog()
{
    delete ui;
}



void ScaleDialog::on_scaleComboBox_currentIndexChanged(int index)
{
    emit scaleChanged( index );
}
