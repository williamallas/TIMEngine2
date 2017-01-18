#include "ObjectInstancingDialog.h"
#include "ui_ObjectInstancingDialog.h"

ObjectInstancingDialog::ObjectInstancingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ObjectInstancingDialog)
{
    ui->setupUi(this);
    setWindowTitle("ObjectInstancing");
}

ObjectInstancingDialog::~ObjectInstancingDialog()
{
    delete ui;
}

ObjectInstancingDialog::ObjectInstancingConfig ObjectInstancingDialog::getConfig() const
{
    ObjectInstancingConfig config;
    config.name = ui->name->text();
    config.number = ui->number->value();
    config.prefix = ui->prefix->isChecked();
    config.size[0] = ui->sizeX->value();
    config.size[1] = ui->sizeY->value();
    config.size[2] = ui->sizeZ->value();
    config.randRot[0] = ui->randRotX->isChecked();
    config.randRot[1] = ui->randRotY->isChecked();
    config.randRot[2] = ui->randRotZ->isChecked();
    config.randScale[0] = ui->scaleMin->value();
    config.randScale[1] = ui->scaleMax->value();
    config.seed = ui->seed->value();

    return config;
}

void ObjectInstancingDialog::on_generate_clicked()
{
    _state = 1;
    close();
}

void ObjectInstancingDialog::on_cancel_clicked()
{
    _state = 0;
    close();
}

void ObjectInstancingDialog::on_remLast_clicked()
{
    _state = 2;
    close();
}
