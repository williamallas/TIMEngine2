#include "LightComponent.h"
#include "ui_LightComponent.h"

LightComponent::LightComponent(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LightComponent)
{
    ui->setupUi(this);
}

LightComponent::~LightComponent()
{
    delete ui;
}
