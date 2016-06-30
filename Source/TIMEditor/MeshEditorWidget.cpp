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
    setMinimumWidth(320);
    updateColorButton();

    connect(ui->dm_roughnessSlider, SIGNAL(sliderMoved(int)), this, SLOT(dm_roughnessSlider_sliderMoved(int)));
    connect(ui->dm_metallicSlider, SIGNAL(sliderMoved(int)), this, SLOT(dm_metallicSlider_sliderMoved(int)));
    connect(ui->dm_specularSlider, SIGNAL(sliderMoved(int)), this, SLOT(dm_specularSlider_sliderMoved(int)));
    connect(ui->dm_emissiveSlider, SIGNAL(sliderMoved(int)), this, SLOT(dm_emissiveSlider_sliderMoved(int)));

    connect(ui->dm_roughnessVal, SIGNAL(valueChanged(double)), this, SLOT(dm_roughnessVal_valueChanged(double)));
    connect(ui->dm_metallicVal, SIGNAL(valueChanged(double)), this, SLOT(dm_metallicVal_valueChanged(double)));
    connect(ui->dm_emissiveVal, SIGNAL(valueChanged(double)), this, SLOT(dm_emissiveVal_valueChanged(double)));

    connect(ui->dmSelectColor, SIGNAL(pressed()), this, SLOT(dmSelectColor_clicked()));
}

QString MeshEditorWidget::currentMeshName() const
{
    return ui->meshName->text();
}

void MeshEditorWidget::setMainRenderer(tim::MainRenderer* r)
{
    if(!r) return;
    _renderer = r;

    _renderer->lock();
    _meshEditorNode = &r->getScene(0).scene.add<MeshInstance>(mat4::IDENTITY());
    r->getScene(0).globalLight.dirLights.push_back({vec3(1,1,-1), vec4::construct(1), true});
    _renderer->unlock();

    activeEditMode();
}

void MeshEditorWidget::activeEditMode()
{
    _editedMaterials = &_meshEditorMaterials;
    _editedMesh = _meshEditorNode;
    _highlightedMesh = nullptr;
}

void MeshEditorWidget::setEditedMesh(MeshInstance* node, MeshInstance* highlighted, QList<MeshElement>* materials, QString model)
{
    if(node == nullptr || materials == nullptr)
    {
        _editedMesh = nullptr;
        _highlightedMesh = nullptr;
        _editedMaterials = nullptr;
        _curElementIndex = -1;

        ui->meshPartView->clear();
        ui->meshName->setText("");

        return;
    }

    _editedMesh = node;
    _highlightedMesh = highlighted;
    _editedMaterials = materials;
    _curElementIndex = 0;

    ui->meshPartView->clear();
    ui->meshName->setText(model);

    for(int i=0 ; i<materials->size() ; ++i)
    {
        addUiElement((*materials)[i]);
    }

    itemSelectionChanged(_curElementIndex);
}

void MeshEditorWidget::addElement(QString geometry)
{
    MeshElement elem;
    elem.geometry = geometry;

    addElement(elem);
}

void MeshEditorWidget::addElement(MeshElement elem)
{
    if(!_editedMaterials || !_editedMesh)
        return;

    (*_editedMaterials) += elem;
    _curElementIndex = (*_editedMaterials).size()-1;

    addUiElement(elem);

    _renderer->addEvent([=](){
        Mesh mesh  = _editedMesh->mesh();

        Mesh::Element melem = constructMeshElement(elem);
        mesh.addElement(melem);

        _editedMesh->setMesh(mesh);

        if(_highlightedMesh)
        {
            _highlightedMesh->setMesh(highlightMesh(mesh));
        }
    });


    itemSelectionChanged(_curElementIndex);
}

void MeshEditorWidget::addUiElement(const MeshElement& elem)
{
    QListWidgetItem* item = new QListWidgetItem(QFileInfo(elem.geometry).baseName() + " (" +
                                                QFileInfo(elem.geometry).fileName() + ")");
    item->setSizeHint(QSize(0, 40));
    item->setFont(QFont("Franklin Gothic Medium", 16));
    item->setBackgroundColor(elem.color);

    ui->meshPartView->addItem(item);
    item->setSelected(true);
}

Mesh::Element MeshEditorWidget::constructMeshElement(const MeshElement& elem)
{
    Mesh::Element melem;
    melem.setGeometry(resource::AssetManager<Geometry>::instance().load<false>(elem.geometry.toStdString()).value());

    vec4 color = vec4(elem.color.red(), elem.color.green(), elem.color.blue(), 255) / vec4::construct(255);
    melem.setColor(color);

    melem.setEmissive(elem.material.w());
    melem.setRoughness(elem.material.x());
    melem.setMetallic(elem.material.y());
    melem.setSpecular(elem.material.z());

    for(int i=0 ; i<MeshElement::NB_TEXTURES ; ++i)
    {
        if(elem.textures[i].isEmpty()) continue;

        renderer::Texture::GenTexParam param;
        param.linear = true;
        param.repeat = true;
        param.trilinear = true;
        param.anisotropy = 4;
        param.nbLevels = -1;
        Texture tex = resource::AssetManager<Texture>::instance().load<false>(elem.textures[i].toStdString(), param).value();
        melem.setTexture(tex, i);
    }

    melem.drawState().setShader(ShaderPool::instance().get("gPass"));
    return melem;
}

interface::Mesh MeshEditorWidget::highlightMesh(const interface::Mesh& mesh)
{
    Mesh m = mesh;
    for(uint i=0 ; i<m.nbElements() ; ++i)
    {
        m.element(i).setTexture(Texture(), 0);
        m.element(i).setTexture(Texture(), 1);
        m.element(i).setTexture(Texture(), 2);
        m.element(i).drawState().setCullBackFace(false);
        m.element(i).drawState().setShader(ShaderPool::instance().get("highlighted"));
    }
    return m;
}

void MeshEditorWidget::setMesh(QString name, const QList<MeshElement>& elements)
{
    if(!_editedMaterials || !_editedMesh)
        return;

    _curElementIndex = -1;
    (*_editedMaterials).clear();
    ui->meshPartView->clear();

    _renderer->lock();
    _editedMesh->setMesh(Mesh());
    if(_highlightedMesh) _highlightedMesh->setMesh(Mesh());
    _renderer->unlock();

    ui->meshName->setText(name);

    for(int i=0 ; i<elements.size() ; ++i)
    {
        addElement(elements[i]);
    }
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

void MeshEditorWidget::updateTexture(int texId)
{
    if(_curElementIndex < 0)
        return;

    QList<QString> list = _resourceWidget->selectResources(ResourceViewWidget::Element::Texture, this, true);
    if(list.empty())
        return;

    (*_editedMaterials)[_curElementIndex].textures[texId] = list[0];
    (*_editedMaterials)[_curElementIndex].texturesIcon[texId] = _resourceWidget->getResourceIconForPath(list[0]);

   switch(texId)
   {
   case 0: ui->diffuseTex->setIcon((*_editedMaterials)[_curElementIndex].texturesIcon[texId]);
       break;

   case 1: ui->normalTex->setIcon((*_editedMaterials)[_curElementIndex].texturesIcon[texId]);
       break;

   case 2: ui->materialTex->setIcon((*_editedMaterials)[_curElementIndex].texturesIcon[texId]);
       break;
   }

    std::string texFile = list[0].toStdString();
    _renderer->addEvent([=](){
        Mesh mesh = _editedMesh->mesh();
        renderer::Texture::GenTexParam param;
        param.linear = true;
        param.repeat = true;
        param.trilinear = true;
        param.nbLevels = -1;
        param.anisotropy = 4;
        Texture tex = resource::AssetManager<Texture>::instance().load<false>(texFile, param).value();
        mesh.element(_curElementIndex).setTexture(tex, texId);
        _editedMesh->setMesh(mesh);
    });
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

void MeshEditorWidget::dm_emissiveSlider_sliderMoved(int n)
{
    ui->dm_emissiveVal->setValue(double(n) / ui->dm_emissiveSlider->maximum());
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

void MeshEditorWidget::dm_emissiveVal_valueChanged(double n)
{
    ui->dm_emissiveSlider->setValue(int(n*ui->dm_emissiveSlider->maximum()));
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

    vec4 material;
    material[0] = ui->dm_roughnessVal->value();
    material[1] = ui->dm_metallicVal->value();
    material[2] = ui->dm_specularVal->value();
    material[3] = ui->dm_emissiveVal->value();

    (*_editedMaterials)[_curElementIndex].material = material;
    (*_editedMaterials)[_curElementIndex].color = QColor(ui->dm_colorR->value(), ui->dm_colorG->value(), ui->dm_colorB->value());

    vec4 color = vec4((*_editedMaterials)[_curElementIndex].color.red(),
                      (*_editedMaterials)[_curElementIndex].color.green(),
                      (*_editedMaterials)[_curElementIndex].color.blue(), 255) / vec4::construct(255);

    int index = _curElementIndex;
    MeshInstance* editedMesh = _editedMesh;
    MeshInstance* hMesh = _highlightedMesh;

    _renderer->addEvent([=](){
        Mesh mesh = editedMesh->mesh();
        if(int(mesh.nbElements()) <= index)
            return;

        mesh.element(index).setRoughness(material.x());
        mesh.element(index).setMetallic(material.y());
        mesh.element(index).setSpecular(material.z());
        mesh.element(index).setEmissive(material.w());
        mesh.element(index).setColor(color);
        editedMesh->setMesh(mesh);

        if(hMesh) hMesh->setMesh(highlightMesh(mesh));
    });

    updateColorButton();
}

void MeshEditorWidget::itemSelectionChanged(QModelIndex index)
{
    itemSelectionChanged(index.row());
}

void MeshEditorWidget::itemSelectionChanged(int row)
{
    if(row >= (*_editedMaterials).size())
        return;

    _curElementIndex = row;

    ui->dm_colorR->blockSignals(true);
    ui->dm_colorG->blockSignals(true);
    ui->dm_colorB->blockSignals(true);
    ui->dm_roughnessVal->blockSignals(true);
    ui->dm_metallicVal->blockSignals(true);
    ui->dm_specularVal->blockSignals(true);
    ui->dm_emissiveVal->blockSignals(true);
    ui->dm_roughnessSlider->blockSignals(true);
    ui->dm_metallicSlider->blockSignals(true);
    ui->dm_specularSlider->blockSignals(true);
    ui->dm_emissiveSlider->blockSignals(true);

    ui->dm_colorR->setValue((*_editedMaterials)[_curElementIndex].color.red());
    ui->dm_colorG->setValue((*_editedMaterials)[_curElementIndex].color.green());
    ui->dm_colorB->setValue((*_editedMaterials)[_curElementIndex].color.blue());
    updateColorButton();

    ui->dm_roughnessVal->setValue((*_editedMaterials)[_curElementIndex].material.x());
    ui->dm_metallicVal->setValue((*_editedMaterials)[_curElementIndex].material.y());
    ui->dm_specularVal->setValue((*_editedMaterials)[_curElementIndex].material.z());
    ui->dm_emissiveVal->setValue((*_editedMaterials)[_curElementIndex].material.w());

    ui->dm_roughnessSlider->setValue(int((*_editedMaterials)[_curElementIndex].material.x()*ui->dm_roughnessSlider->maximum()));
    ui->dm_metallicSlider->setValue(int((*_editedMaterials)[_curElementIndex].material.y()*ui->dm_metallicSlider->maximum()));
    ui->dm_specularSlider->setValue(int((*_editedMaterials)[_curElementIndex].material.z()*ui->dm_specularSlider->maximum()));
    ui->dm_emissiveSlider->setValue(int((*_editedMaterials)[_curElementIndex].material.w()*ui->dm_emissiveSlider->maximum()));

    ui->diffuseTex->setIcon((*_editedMaterials)[_curElementIndex].texturesIcon[0]);
    ui->normalTex->setIcon((*_editedMaterials)[_curElementIndex].texturesIcon[1]);
    ui->materialTex->setIcon((*_editedMaterials)[_curElementIndex].texturesIcon[2]);

    ui->dm_colorR->blockSignals(false);
    ui->dm_colorG->blockSignals(false);
    ui->dm_colorB->blockSignals(false);
    ui->dm_roughnessVal->blockSignals(false);
    ui->dm_metallicVal->blockSignals(false);
    ui->dm_specularVal->blockSignals(false);
    ui->dm_emissiveVal->blockSignals(false);
    ui->dm_roughnessSlider->blockSignals(false);
    ui->dm_metallicSlider->blockSignals(false);
    ui->dm_specularSlider->blockSignals(false);
    ui->dm_emissiveSlider->blockSignals(false);
}

void MeshEditorWidget::selectGeometryFromResources()
{
    QList<QString> elems = _resourceWidget->selectResources(ResourceViewWidget::Element::Geometry, this, false);
    for(QString& str : elems)
    {
        addElement(str);
    }
}

void MeshEditorWidget::removeElement()
{
    if(_curElementIndex < 0)
        return;

    (*_editedMaterials).removeAt(_curElementIndex);
    delete ui->meshPartView->takeItem(_curElementIndex);

    int elemToRemove = _curElementIndex;
    MeshInstance* editedMesh = _editedMesh;
    MeshInstance* hMesh = _highlightedMesh;

    _renderer->addEvent([=](){
        Mesh mesh;

        int i=0;
        while(i++ < int(editedMesh->mesh().nbElements()))
        {
            if(i-1 != elemToRemove)
                mesh.addElement(editedMesh->mesh().element(i-1));
        }
        editedMesh->setMesh(mesh);
        if(hMesh) hMesh->setMesh(highlightMesh(mesh));
    });

    if((*_editedMaterials).size() <= _curElementIndex)
        _curElementIndex--;

    if(_curElementIndex < 0)
        return;

    ui->meshPartView->item(_curElementIndex)->setSelected(true);
    itemSelectionChanged(_curElementIndex);
}

void MeshEditorWidget::rotateEditedMesh(int dx, int dy)
{
    if(!_editedMaterials || !_editedMesh)
        return;

    _renderer->lock();

    _rz += dx * _renderer->elapsedTime();
    _ry += dy * _renderer->elapsedTime();

    mat4 m = _editedMesh->matrix();
    m = mat4::RotationX(dy * _renderer->elapsedTime()) * mat4::RotationZ(dx * _renderer->elapsedTime())*m;
    _editedMesh->setMatrix(m);

    _renderer->unlock();
}

void MeshEditorWidget::resetView()
{
    _renderer->lock();
    _editedMesh->setMatrix(mat4::IDENTITY());
    _rz=0;
    _ry=0;
    _renderer->getSceneView(0).camera.pos = {0,-3,0};
    _renderer->getSceneView(0).camera.dir = {0,0,0};
    _renderer->unlock();
}

void MeshEditorWidget::on_diffuseTex_clicked()
{
    updateTexture(0);
}

void MeshEditorWidget::on_normalTex_clicked()
{
    updateTexture(1);
}

void MeshEditorWidget::on_materialTex_clicked()
{
    updateTexture(2);
}

void MeshEditorWidget::on_saveMeshButton_pressed()
{
    emit saveMeshClicked();
}
