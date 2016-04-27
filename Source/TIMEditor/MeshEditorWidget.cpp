#include "MeshEditorWidget.h"
#include "ui_MeshEditor.h"
#include <QColorDialog>
#include <QListWidgetItem>
#include <QFileInfo>

using namespace tim;
using namespace tim::core;
using namespace tim::interface;

MeshEditorWidget::MeshEditorWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MeshEditor)
{
    ui->setupUi(this);
    updateColorButton();

    connect(ui->dm_roughnessSlider, SIGNAL(sliderMoved(int)), this, SLOT(dm_roughnessSlider_sliderMoved(int)));
    connect(ui->dm_metallicSlider, SIGNAL(sliderMoved(int)), this, SLOT(dm_metallicSlider_sliderMoved(int)));
    connect(ui->dm_specularSlider, SIGNAL(sliderMoved(int)), this, SLOT(dm_specularSlider_sliderMoved(int)));

    connect(ui->dm_roughnessVal, SIGNAL(valueChanged(double)), this, SLOT(dm_roughnessVal_valueChanged(double)));
    connect(ui->dm_metallicVal, SIGNAL(valueChanged(double)), this, SLOT(dm_metallicVal_valueChanged(double)));
    connect(ui->dm_specularVal, SIGNAL(valueChanged(double)), this, SLOT(dm_specularVal_valueChanged(double)));

    connect(ui->dmSelectColor, SIGNAL(pressed()), this, SLOT(dmSelectColor_clicked()));
}

void MeshEditorWidget::setMainRenderer(tim::MainRenderer* r)
{
    if(!r) return;
    _renderer = r;

    _renderer->lock();
    _editedMesh = &r->getScene(0).scene.add<MeshInstance>(mat4::IDENTITY());
    r->getScene(0).globalLight.dirLights.push_back({vec3(1,1,-1), vec4::construct(1), true});
    _renderer->unlock();
}

void MeshEditorWidget::addElement(QString geometry)
{
    Element elem;
    elem.geometry = geometry;
    elem.name = QFileInfo(geometry).baseName();
    _meshData += elem;

    QListWidgetItem* item = new QListWidgetItem(elem.name + " (" +
                                                QFileInfo(geometry).fileName() + ")");
    item->setSizeHint(QSize(0, 40));
    item->setFont(QFont("Franklin Gothic Medium", 16));
    item->setBackgroundColor(elem.color);
    item->setSelected(true);

    ui->meshPartView->addItem(item);

    std::string geomFile = geometry.toStdString();
    vec4 color = vec4(elem.color.red(), elem.color.green(), elem.color.blue(), 255) / vec4::construct(255);

    _renderer->addEvent([=](){
        Mesh mesh = _editedMesh->mesh();
        Mesh::Element elem;
        elem.setGeometry(resource::AssetManager<Geometry>::instance().load<false>(geomFile).value());
        elem.setColor(color);
        elem.drawState().setShader(ShaderPool::instance().get("gPass"));
        mesh.addElement(elem);
        _editedMesh->setMesh(mesh);
    });
}

void MeshEditorWidget::updateMesh()
{
    vec3 material;

}

void MeshEditorWidget::updateColorButton()
{
    QColor c(ui->dm_colorR->value(), ui->dm_colorG->value(), ui->dm_colorB->value());
    QString qss = QString("background-color: %1;").arg(c.name());
    ui->dmSelectColor->setStyleSheet(qss);
    ui->dmSelectColor->update();
}

/* SLOTS */
void MeshEditorWidget::dm_roughnessSlider_sliderMoved(int n)
{
    ui->dm_roughnessVal->setValue(double(n) / ui->dm_roughnessSlider->maximum());
}

void MeshEditorWidget::dm_metallicSlider_sliderMoved(int n)
{
    ui->dm_metallicVal->setValue(double(n) / ui->dm_metallicSlider->maximum());
}

void MeshEditorWidget::dm_specularSlider_sliderMoved(int n)
{
    ui->dm_specularVal->setValue(double(n) / ui->dm_specularSlider->maximum());
}

void MeshEditorWidget::dm_roughnessVal_valueChanged(double n)
{
    ui->dm_roughnessSlider->setValue(int(n*ui->dm_roughnessSlider->maximum()));
}

void MeshEditorWidget::dm_metallicVal_valueChanged(double n)
{
    ui->dm_metallicSlider->setValue(int(n*ui->dm_metallicSlider->maximum()));
}

void MeshEditorWidget::dm_specularVal_valueChanged(double n)
{
    ui->dm_specularSlider->setValue(int(n*ui->dm_specularSlider->maximum()));
}

void MeshEditorWidget::dmSelectColor_clicked()
{
    QColor c = QColorDialog::getColor(QColor(ui->dm_colorR->value(), ui->dm_colorG->value(), ui->dm_colorB->value()),
                                      this, "Material color");
    ui->dm_colorR->setValue(c.red());
    ui->dm_colorG->setValue(c.green());
    ui->dm_colorB->setValue(c.blue());

    updateColorButton();
}

