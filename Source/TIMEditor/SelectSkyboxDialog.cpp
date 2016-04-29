#include "SelectSkyboxDialog.h"
#include "ui_SelectSkyboxDialog.h"

SelectSkyboxDialog::SelectSkyboxDialog(QWidget *parent, ResourceViewWidget* resourceWidget) :
    QDialog(parent),
    _resourceWidget(resourceWidget),
    ui(new Ui::SelectSkyboxDialog)
{
    ui->setupUi(this);
}

SelectSkyboxDialog::~SelectSkyboxDialog()
{
    delete ui;
}

QList<QString> SelectSkyboxDialog::getSkydirPaths()
{
    return _returnPaths;
}

void SelectSkyboxDialog::on_xButton_clicked()
{
    selectFileFor(ui->xButton, SKY_DIR_INDEX::X);
}

void SelectSkyboxDialog::on_yButton_clicked()
{
    selectFileFor(ui->yButton, SKY_DIR_INDEX::Y);
}

void SelectSkyboxDialog::on_zButton_clicked()
{
    selectFileFor(ui->zButton, SKY_DIR_INDEX::Z);
}

void SelectSkyboxDialog::on_nxButton_clicked()
{
    selectFileFor(ui->nxButton, SKY_DIR_INDEX::NX);
}

void SelectSkyboxDialog::on_nyButton_clicked()
{
    selectFileFor(ui->nyButton, SKY_DIR_INDEX::NY);
}

void SelectSkyboxDialog::on_nzButton_clicked()
{
    selectFileFor(ui->nzButton, SKY_DIR_INDEX::NZ);
}

void SelectSkyboxDialog::selectFileFor(QPushButton* button, int pathIndex) {
    QList<QString> paths = _resourceWidget->selectResources(ResourceViewWidget::Element::Texture, this, true);
    if (paths.size() > 0)
    {
        QString skydirPath = paths[0];
        _skydirpaths[pathIndex] = skydirPath;
        button->setIcon(_resourceWidget->getResourceIconForPath(skydirPath));
    }
}

void SelectSkyboxDialog::on_okButton_clicked()
{
    for (size_t i = 0; i < 6; ++i)
    {
        if (_skydirpaths[i].isEmpty())
        {
            _returnPaths.clear();
            close();
            return;
        }
        _returnPaths += _skydirpaths[i];
    }
    close();
}
