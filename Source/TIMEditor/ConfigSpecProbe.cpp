#include "ConfigSpecProbe.h"
#include "ui_ConfigSpecProbe.h"
#include <QFileDialog>

ConfigSpecProbe::ConfigSpecProbe(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigSpecProbe)
{
    ui->setupUi(this);

    ui->resolution->addItem("512x512");
    ui->resolution->addItem("1024x1024");
    ui->resolution->addItem("2048x2048");

    ui->centerOn->addItem("Camera");
    ui->centerOn->addItem("Selection");
}

ConfigSpecProbe::~ConfigSpecProbe()
{
    delete ui;
}

void ConfigSpecProbe::on_render_clicked()
{
    _pathSkybox = ui->skybox->isChecked() ? QFileDialog::getExistingDirectory(this, "Save skybox", ".") : "";
    _pathRawData = ui->rawData->isChecked() ? QFileDialog::getSaveFileName(this, "Save cubemap", ".", "RawImage Files (*.itim)") : "";

    _renderClicked = true;
    _nbIterations = ui->iterations->value();
    _resolution = 512 << ui->resolution->currentIndex();
    _addToScene = ui->addToScene->isChecked();
    _centerOnSelection = ui->centerOn->currentIndex() > 0;
    _radius = static_cast<float>(ui->radius->value());
    _farDist = static_cast<float>(ui->far->value());
    _exportAsRawData = ui->rawData->isChecked();
    _exportAsSkybox = ui->skybox->isChecked();

    close();
}
