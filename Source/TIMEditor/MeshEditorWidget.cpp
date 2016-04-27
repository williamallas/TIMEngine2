#include "MeshEditorWidget.h"
#include "ResourceViewWidget.h"
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

    ui->addElementButton->setIcon(QIcon("icon/plus-icon.png"));
    ui->removeElementButton->setIcon(QIcon("icon/minus-icon.png"));

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
    _curElementIndex = _meshData.size()-1;

    ui->meshPartView->addItem(item);
    item->setSelected(true);

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

void MeshEditorWidget::updateColorButton()
{
    QColor c(ui->dm_colorR->value(), ui->dm_colorG->value(), ui->dm_colorB->value());
    QString qss = QString("background-color: %1;").arg(c.name());
    ui->dmSelectColor->setStyleSheet(qss);
    ui->dmSelectColor->update();

    if(_curElementIndex >= 0)
    {
        ui->meshPartView->item(_curElementIndex)->setBackground(c);
    }
}

/* SLOTS */
void MeshEditorWidget::dm_roughnessSlider_sliderMoved(int n)
{
    ui->dm_roughnessVal->setValue(double(n) / ui->dm_roughnessSlider->maximum());
    updateMaterial();
}

void MeshEditorWidget::dm_metallicSlider_sliderMoved(int n)
{
    ui->dm_metallicVal->setValue(double(n) / ui->dm_metallicSlider->maximum());
    updateMaterial();
}

void MeshEditorWidget::dm_specularSlider_sliderMoved(int n)
{
    ui->dm_specularVal->setValue(double(n) / ui->dm_specularSlider->maximum());
    updateMaterial();
}

void MeshEditorWidget::dm_roughnessVal_valueChanged(double n)
{
    ui->dm_roughnessSlider->setValue(int(n*ui->dm_roughnessSlider->maximum()));
    updateMaterial();
}

void MeshEditorWidget::dm_metallicVal_valueChanged(double n)
{
    ui->dm_metallicSlider->setValue(int(n*ui->dm_metallicSlider->maximum()));
    updateMaterial();
}

void MeshEditorWidget::dm_specularVal_valueChanged(double n)
{
    ui->dm_specularSlider->setValue(int(n*ui->dm_specularSlider->maximum()));
    updateMaterial();
}

void MeshEditorWidget::dmSelectColor_clicked()
{
    QColor c = QColorDialog::getColor(QColor(ui->dm_colorR->value(), ui->dm_colorG->value(), ui->dm_colorB->value()),
                                      this, "Material color");
    ui->dm_colorR->setValue(c.red());
    ui->dm_colorG->setValue(c.green());
    ui->dm_colorB->setValue(c.blue());
    updateMaterial();
}

void MeshEditorWidget::updateMaterial()
{
    if(_curElementIndex < 0)
        return;

    vec3 material;
    material[0] = ui->dm_roughnessVal->value();
    material[1] = ui->dm_metallicVal->value();
    material[2] = ui->dm_specularVal->value();

    _meshData[_curElementIndex].material = material;
    _meshData[_curElementIndex].color = QColor(ui->dm_colorR->value(), ui->dm_colorG->value(), ui->dm_colorB->value());

    vec4 color = vec4(_meshData[_curElementIndex].color.red(),
                      _meshData[_curElementIndex].color.green(),
                      _meshData[_curElementIndex].color.blue(), 255) / vec4::construct(255);

    int index = _curElementIndex;
    MeshInstance* editedMesh = _editedMesh;

    _renderer->addEvent([=](){
        Mesh mesh = editedMesh->mesh();
        if(int(mesh.nbElements()) <= index)
            return;

        mesh.element(index).setRougness(material.x());
        mesh.element(index).setMetallic(material.y());
        mesh.element(index).setSpecular(material.z());
        mesh.element(index).setColor(color);
        editedMesh->setMesh(mesh);
    });

    updateColorButton();
}

void MeshEditorWidget::itemSelectionChanged(QModelIndex index)
{
    itemSelectionChanged(index.row());
}

void MeshEditorWidget::itemSelectionChanged(int row)
{
    if(row >= _meshData.size())
        return;

    _curElementIndex = row;

    ui->dm_colorR->blockSignals(true);
    ui->dm_colorG->blockSignals(true);
    ui->dm_colorB->blockSignals(true);
    ui->dm_roughnessVal->blockSignals(true);
    ui->dm_metallicVal->blockSignals(true);
    ui->dm_specularVal->blockSignals(true);
    ui->dm_roughnessSlider->blockSignals(true);
    ui->dm_metallicSlider->blockSignals(true);
    ui->dm_specularSlider->blockSignals(true);

    ui->dm_colorR->setValue(_meshData[_curElementIndex].color.red());
    ui->dm_colorG->setValue(_meshData[_curElementIndex].color.green());
    ui->dm_colorB->setValue(_meshData[_curElementIndex].color.blue());
    updateColorButton();

    ui->dm_roughnessVal->setValue(_meshData[_curElementIndex].material.x());
    ui->dm_metallicVal->setValue(_meshData[_curElementIndex].material.y());
    ui->dm_specularVal->setValue(_meshData[_curElementIndex].material.z());

    ui->dm_roughnessSlider->setValue(int(_meshData[_curElementIndex].material.x()*ui->dm_roughnessSlider->maximum()));
    ui->dm_metallicSlider->setValue(int(_meshData[_curElementIndex].material.y()*ui->dm_metallicSlider->maximum()));
    ui->dm_specularSlider->setValue(int(_meshData[_curElementIndex].material.z()*ui->dm_specularSlider->maximum()));

    vec3 material = _meshData[_curElementIndex].material;

    vec4 color = vec4(_meshData[_curElementIndex].color.red(),
                      _meshData[_curElementIndex].color.green(),
                      _meshData[_curElementIndex].color.blue(), 255) / vec4::construct(255);

    int elem = _curElementIndex;
    MeshInstance* editedMesh = _editedMesh;

    _renderer->addEvent([=](){
        Mesh mesh = editedMesh->mesh();
        if(int(mesh.nbElements()) <= elem)
            return;

        mesh.element(elem).setRougness(material.x());
        mesh.element(elem).setMetallic(material.y());
        mesh.element(elem).setSpecular(material.z());
        mesh.element(elem).setColor(color);
        editedMesh->setMesh(mesh);
    });

    ui->dm_colorR->blockSignals(false);
    ui->dm_colorG->blockSignals(false);
    ui->dm_colorB->blockSignals(false);
    ui->dm_roughnessVal->blockSignals(false);
    ui->dm_metallicVal->blockSignals(false);
    ui->dm_specularVal->blockSignals(false);
    ui->dm_roughnessSlider->blockSignals(false);
    ui->dm_metallicSlider->blockSignals(false);
    ui->dm_specularSlider->blockSignals(false);
}

void MeshEditorWidget::selectGeometryFromResources()
{
    QList<QString> elems = _resourceWidget->selectResources(ResourceViewWidget::Element::Geometry, this);
    for(QString& str : elems)
    {
        addElement(str);
    }
}

void MeshEditorWidget::removeElement()
{
    if(_curElementIndex < 0)
        return;

    _meshData.removeAt(_curElementIndex);
    delete ui->meshPartView->takeItem(_curElementIndex);

    int elemToRemove = _curElementIndex;
    MeshInstance* editedMesh = _editedMesh;

    _renderer->addEvent([=](){
        Mesh mesh;

        int i=0;
        while(i++ < int(editedMesh->mesh().nbElements()))
        {
            if(i-1 != elemToRemove)
                mesh.addElement(editedMesh->mesh().element(i-1));
        }
        editedMesh->setMesh(mesh);
    });

    if(_meshData.size() <= _curElementIndex)
        _curElementIndex--;

    if(_curElementIndex < 0)
        return;

    ui->meshPartView->item(_curElementIndex)->setSelected(true);
    itemSelectionChanged(_curElementIndex);
}
