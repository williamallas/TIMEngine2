#include "MeshEditorWidget.h"
#include "ResourceViewWidget.h"
#include "ui_MeshEditor.h"
#include <QColorDialog>
#include <QListWidgetItem>
#include <QFileInfo>

#undef DrawState

using namespace tim;
using namespace tim::core;
using namespace tim::interface;

MeshEditorWidget::MeshEditorWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MeshEditor)
{
    ui->setupUi(this);
    setMinimumWidth(320);
    updateColorButton();

    ui->shaderList->addItem("gPass");
    ui->shaderList->addItem("gPassAlphaTest");
    ui->shaderList->addItem("water");

    connect(ui->dm_roughnessSlider, SIGNAL(sliderMoved(int)), this, SLOT(dm_roughnessSlider_sliderMoved(int)));
    connect(ui->dm_metallicSlider, SIGNAL(sliderMoved(int)), this, SLOT(dm_metallicSlider_sliderMoved(int)));
    connect(ui->dm_specularSlider, SIGNAL(sliderMoved(int)), this, SLOT(dm_specularSlider_sliderMoved(int)));
    connect(ui->dm_emissiveSlider, SIGNAL(sliderMoved(int)), this, SLOT(dm_emissiveSlider_sliderMoved(int)));
    connect(ui->dm_textureScaleSlider, SIGNAL(sliderMoved(int)), this, SLOT(dm_textureScaleSlider_sliderMoved(int)));

    connect(ui->dm_roughnessVal, SIGNAL(valueChanged(double)), this, SLOT(dm_roughnessVal_valueChanged(double)));
    connect(ui->dm_metallicVal, SIGNAL(valueChanged(double)), this, SLOT(dm_metallicVal_valueChanged(double)));
    connect(ui->dm_specularVal, SIGNAL(valueChanged(double)), this, SLOT(dm_specularVal_valueChanged(double)));
    connect(ui->dm_emissiveVal, SIGNAL(valueChanged(double)), this, SLOT(dm_emissiveVal_valueChanged(double)));
    connect(ui->dm_textureScaleVal, SIGNAL(valueChanged(double)), this, SLOT(dm_textureScaleVal_valueChanged(double)));

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

    ui->meshPartView->clear();
    ui->meshName->setText("");
    _curElementIndex = _editedMaterials->size()>0 ? 0:-1;

    for(int i=0 ; i<_editedMaterials->size() ; ++i)
    {
        addUiElement((*_editedMaterials)[i]);
    }

    if(!_editedMaterials->empty())
    {
        _curElementIndex = 0;
        itemSelectionChanged(0);
    }
    else setUi(MeshElement());

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

        ui->diffuseTex->setIcon(QIcon());
        ui->normalTex->setIcon(QIcon());
        ui->materialTex->setIcon(QIcon());

        ui->advancedMaterial->setChecked(false);

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

    auto optGeom = resource::AssetManager<Geometry>::instance().load<false>(elem.geometry.toStdString(), true);
    if(optGeom.hasValue())
        melem.setGeometry(optGeom.value());

    vec4 color = vec4(elem.color.red(), elem.color.green(), elem.color.blue(), 255) / vec4::construct(255);
    melem.setColor(color);

    melem.setEmissive(elem.material.w());
    melem.setRoughness(elem.material.x());
    melem.setMetallic(elem.material.y());
    melem.setSpecular(elem.material.z());
    melem.setTextureScale(elem.textureScale);

    for(int i=0 ; i<MeshElement::NB_TEXTURES ; ++i)
    {
        if(elem.textures[i].isEmpty()) continue;

        renderer::Texture::GenTexParam param;
        param.linear = true;
        param.repeat = true;
        param.trilinear = true;
        param.anisotropy = 4;
        param.nbLevels = -1;

        auto opt = resource::AssetManager<Texture>::instance().load<false>(elem.textures[i].toStdString(), param);
        if(opt.hasValue())
            melem.setTexture(opt.value(), i);
    }

    melem.drawState().setShader(ShaderPool::instance().get("gPass"));
    if(elem.useAdvanced)
    {
        melem.drawState() = elem.advanced;
        if(!elem.advancedShader.isEmpty())
            melem.drawState().setShader(ShaderPool::instance().get(elem.advancedShader.toStdString()));
        else
            melem.drawState().setShader(ShaderPool::instance().get("gPass"));
    }

    melem.setCastShadow(elem.castShadow);

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
    emit changeCurBaseModelName(name);

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

    bool clearTexture = false;
    QList<QString> list = _resourceWidget->selectResources(ResourceViewWidget::Element::Texture, this, true, clearTexture);
    if(list.empty() && !clearTexture)
        return;
    else if(list.empty() && clearTexture)
    {
        (*_editedMaterials)[_curElementIndex].textures[texId] = "";
        (*_editedMaterials)[_curElementIndex].texturesIcon[texId] = QIcon();
    }
    else
    {
        (*_editedMaterials)[_curElementIndex].textures[texId] = list[0];
        (*_editedMaterials)[_curElementIndex].texturesIcon[texId] = _resourceWidget->getResourceIconForPath(list[0]);
    }

   switch(texId)
   {
   case 0: ui->diffuseTex->setIcon((*_editedMaterials)[_curElementIndex].texturesIcon[texId]);
       break;

   case 1: ui->normalTex->setIcon((*_editedMaterials)[_curElementIndex].texturesIcon[texId]);
       break;

   case 2: ui->materialTex->setIcon((*_editedMaterials)[_curElementIndex].texturesIcon[texId]);
       break;
   }

   std::string texFile;
   if(!list.empty())
    texFile = list[0].toStdString();

    _renderer->addEvent([=](){

        Mesh mesh = _editedMesh->mesh();

        if(texFile.empty())
        {
            mesh.element(_curElementIndex).setTexture(Texture(), texId);
            _editedMesh->setMesh(mesh);
            return;
        }

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

QList<MeshElement> MeshEditorWidget::convertFromEngine(const vector<interface::XmlMeshAssetLoader::MeshElementModel>& model)
{
    QList<MeshElement> res;

    for(size_t i=0 ; i<model.size() ; ++i)
    {
        MeshElement mat;
        mat.color = QColor(255*model[i].color.x(), 255*model[i].color.y(), 255*model[i].color.z());
        mat.material = model[i].material;
        mat.geometry = QString::fromStdString(model[i].geometry.c_str());
        mat.textureScale = model[i].textureScale;

        mat.advanced = model[i].advanced;
        mat.advancedShader = QString::fromStdString(model[i].advancedShader);
        mat.useAdvanced = model[i].useAdvanced;
        mat.castShadow = model[i].castShadow;

        for(int j=0 ; j<MeshElement::NB_TEXTURES ; ++j)
        {
            mat.textures[j] = model[i].textures[j].c_str();
            QIcon ic = _resourceWidget->getResourceIconForPath(model[i].textures[j].c_str());
            if(!ic.isNull())
                mat.texturesIcon[j] = ic;
            else
                mat.texturesIcon[j] = QIcon(model[i].textures[j].c_str());

        }
        res.push_back(mat);
    }

    return res;
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

void MeshEditorWidget::dm_textureScaleSlider_sliderMoved(int n)
{
    ui->dm_textureScaleVal->setValue(pow(1.01, double(n) - (ui->dm_emissiveSlider->maximum() / 2)));
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

int convertTexScaleForSlider(double n)
{
    return 500 + int(log(double(n)) / log(1.01));
}

void MeshEditorWidget::dm_textureScaleVal_valueChanged(double n)
{
    int v = convertTexScaleForSlider(n);
    ui->dm_textureScaleSlider->setValue(v);
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
    (*_editedMaterials)[_curElementIndex].castShadow = ui->castShadow->isChecked();

    (*_editedMaterials)[_curElementIndex].useAdvanced = ui->advancedMaterial->isChecked();
    if((*_editedMaterials)[_curElementIndex].useAdvanced)
    {
        (*_editedMaterials)[_curElementIndex].advancedShader = ui->shaderList->currentText();
        (*_editedMaterials)[_curElementIndex].advanced.setCullFace(ui->cullFace->isChecked());
        (*_editedMaterials)[_curElementIndex].advanced.setCullBackFace(ui->cullBackFace->isChecked());
    }

    vec4 color = vec4((*_editedMaterials)[_curElementIndex].color.red(),
                      (*_editedMaterials)[_curElementIndex].color.green(),
                      (*_editedMaterials)[_curElementIndex].color.blue(), 255) / vec4::construct(255);

    (*_editedMaterials)[_curElementIndex].textureScale = ui->dm_textureScaleVal->value();
    float texScale = ui->dm_textureScaleVal->value();

    int index = _curElementIndex;
    MeshInstance* editedMesh = _editedMesh;
    MeshInstance* hMesh = _highlightedMesh;

    bool useAdvanced = (*_editedMaterials)[_curElementIndex].useAdvanced;
    renderer::DrawState adv = (*_editedMaterials)[_curElementIndex].advanced;
    std::string advShader = (*_editedMaterials)[_curElementIndex].advancedShader.toStdString();
    bool castShadow = (*_editedMaterials)[_curElementIndex].castShadow;

    _renderer->addEvent([=](){
        Mesh mesh = editedMesh->mesh();
        if(int(mesh.nbElements()) <= index)
            return;

        mesh.element(index).setRoughness(material.x());
        mesh.element(index).setMetallic(material.y());
        mesh.element(index).setSpecular(material.z());
        mesh.element(index).setEmissive(material.w());
        mesh.element(index).setColor(color);
        mesh.element(index).setTextureScale(texScale);
        mesh.element(index).setCastShadow(castShadow);

        if(useAdvanced)
        {
            mesh.element(index).drawState() = adv;
            if(!advShader.empty())
                mesh.element(index).drawState().setShader(ShaderPool::instance().get(advShader));
            else
                mesh.element(index).drawState().setShader(ShaderPool::instance().get("gPass"));
        }
        else
        {
            mesh.element(index).drawState() = renderer::DrawState();
            mesh.element(index).drawState().setShader(ShaderPool::instance().get("gPass"));
        }

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

     setUi((*_editedMaterials)[_curElementIndex]);
}

void MeshEditorWidget::setUi(const MeshElement& mat)
{
    ui->dm_colorR->blockSignals(true);
    ui->dm_colorG->blockSignals(true);
    ui->dm_colorB->blockSignals(true);
    ui->dm_roughnessVal->blockSignals(true);
    ui->dm_metallicVal->blockSignals(true);
    ui->dm_specularVal->blockSignals(true);
    ui->dm_emissiveVal->blockSignals(true);
    ui->dm_textureScaleVal->blockSignals(true);
    ui->dm_roughnessSlider->blockSignals(true);
    ui->dm_metallicSlider->blockSignals(true);
    ui->dm_specularSlider->blockSignals(true);
    ui->dm_emissiveSlider->blockSignals(true);
    ui->dm_textureScaleSlider->blockSignals(true);
    ui->advancedMaterial->blockSignals(true);
    ui->castShadow->blockSignals(true);
    ui->cullBackFace->blockSignals(true);
    ui->cullFace->blockSignals(true);
    ui->shaderList->blockSignals(true);

    ui->dm_colorR->setValue(mat.color.red());
    ui->dm_colorG->setValue(mat.color.green());
    ui->dm_colorB->setValue(mat.color.blue());
    updateColorButton();

    ui->dm_roughnessVal->setValue(mat.material.x());
    ui->dm_metallicVal->setValue(mat.material.y());
    ui->dm_specularVal->setValue(mat.material.z());
    ui->dm_emissiveVal->setValue(mat.material.w());
    ui->dm_textureScaleVal->setValue(mat.textureScale);

    ui->dm_roughnessSlider->setValue(int(mat.material.x()*ui->dm_roughnessSlider->maximum()));
    ui->dm_metallicSlider->setValue(int(mat.material.y()*ui->dm_metallicSlider->maximum()));
    ui->dm_specularSlider->setValue(int(mat.material.z()*ui->dm_specularSlider->maximum()));
    ui->dm_emissiveSlider->setValue(int(mat.material.w()*ui->dm_emissiveSlider->maximum()));
    ui->dm_textureScaleSlider->setValue(convertTexScaleForSlider(mat.textureScale));
    ui->diffuseTex->setIcon(mat.texturesIcon[0]);
    ui->normalTex->setIcon(mat.texturesIcon[1]);
    ui->materialTex->setIcon(mat.texturesIcon[2]);

    ui->advancedMaterial->setChecked(mat.useAdvanced);
    ui->castShadow->setChecked(mat.castShadow);
    ui->cullBackFace->setChecked(mat.advanced.cullBackFace());
    ui->cullFace->setChecked(mat.advanced.cullFace());
    if(mat.advancedShader.isEmpty())
        ui->shaderList->setCurrentIndex(0);
    else
        ui->shaderList->setCurrentText(mat.advancedShader);

    ui->dm_colorR->blockSignals(false);
    ui->dm_colorG->blockSignals(false);
    ui->dm_colorB->blockSignals(false);
    ui->dm_roughnessVal->blockSignals(false);
    ui->dm_metallicVal->blockSignals(false);
    ui->dm_specularVal->blockSignals(false);
    ui->dm_emissiveVal->blockSignals(false);
    ui->dm_textureScaleVal->blockSignals(false);
    ui->dm_roughnessSlider->blockSignals(false);
    ui->dm_metallicSlider->blockSignals(false);
    ui->dm_specularSlider->blockSignals(false);
    ui->dm_emissiveSlider->blockSignals(false);
    ui->dm_textureScaleSlider->blockSignals(false);
    ui->advancedMaterial->blockSignals(false);
    ui->castShadow->blockSignals(false);
    ui->cullBackFace->blockSignals(false);
    ui->cullFace->blockSignals(false);
    ui->shaderList->blockSignals(false);
}

void MeshEditorWidget::selectGeometryFromResources()
{
    if(!_editedMaterials)
        return;

    bool tmp;
    QList<QString> elems = _resourceWidget->selectResources(ResourceViewWidget::Element::Geometry, this, false, tmp);
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
    {
        setUi(MeshElement());
        return;
    }

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
