#include "SceneEditorWidget.h"
#include "ui_SceneEditor.h"

#include "MeshEditorWidget.h"
#include "RendererWidget.h"
#include "core/Matrix.h"

#include <QQuaternion>
#include <QMatrix3x3>

using namespace tim;
using namespace tim::core;
using namespace tim::interface;

SceneEditorWidget::SceneEditorWidget(QWidget* parent) : QWidget(parent), ui(new Ui::SceneEditor), _flushState{}
{
    ui->setupUi(this);
    setMinimumWidth(320);

    connect(ui->listSceneObject, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(sceneItemActivated(QListWidgetItem*)));
    connect(&_flushState, SIGNAL(timeout()), this, SLOT(flushState()));
    _flushState.setInterval(100);
}

void SceneEditorWidget::setMainRenderer(MainRenderer* r)
{
    _renderer = r;
    _flushState.start();
}

void SceneEditorWidget::addSceneObject(QString name, QString modelName, const QList<MeshElement>& model, mat4 trans)
{
    vec3 tr = trans.translation();

    float scale = (trans.to<3>() * vec3(1,0,0)).length();
    mat3 rot = trans.to<3>();
    rot[0][0] *= 1.f / scale;
    rot[1][1] *= 1.f / scale;
    rot[2][2] *= 1.f / scale;
    addSceneObject(name, modelName, model, rot, tr, vec3::construct(scale));
}

void SceneEditorWidget::addSceneObject(QString name, QString modelName, const QList<MeshElement>& model, const mat3& rot, const vec3& tr, const vec3& scale)
{
    SceneObject obj;
    obj.baseModel = modelName;
    obj.name = name;
    obj.materials = model;

    obj.rotate = rot;
    obj.scale = scale;
    obj.translate = tr;

    mat4 trans = constructTransformation(obj);

    _renderer->lock();
    obj.node = &_renderer->getScene(_renderer->getCurSceneIndex()).scene.add<MeshInstance>(trans);
    obj.sceneIndex = _renderer->getCurSceneIndex();
    _renderer->unlock();

    QString n;
    if(name.isEmpty()) n = "model : " + modelName;
    else if(model.isEmpty()) n = name;
    else n = name + " (" + modelName + ")";

    obj.listItem = new QListWidgetItem(n, ui->listSceneObject);
    ui->listSceneObject->addItem(obj.listItem);

    _objects += obj;

    _renderer->addEvent([=]{
        Mesh m;
        for(int i=0 ; i<obj.materials.size() ; ++i)
            m.addElement(MeshEditorWidget::constructMeshElement(obj.materials[i]));

        obj.node->setMesh(m);
    });
}

mat4 SceneEditorWidget::constructTransformation(const SceneObject& obj)
{
    mat4 trans = obj.rotate.to<4>() * mat4::Scale(obj.scale);
    trans.setTranslation(obj.translate);
    return trans;
}

void SceneEditorWidget::activateObject(int index)
{
    if(index >= _objects.size())
        return;

    flushItemUi(index);

    _curItemIndex = index;

    _renderer->waitNoEvent();

    _renderer->lock();
    if(_highlightedMeshInstance == nullptr)
    {
        _highlightedMeshInstance = &_renderer->getScene(_renderer->getCurSceneIndex()).scene.add<MeshInstance>(mat4::IDENTITY());
    }

    _highlightedMeshInstance->setEnable(true);
    _highlightedMeshInstance->setMatrix(_objects[index].node->matrix());
    _highlightedMeshInstance->setMesh(MeshEditorWidget::highlightMesh(_objects[index].node->mesh()));

    _renderer->unlock();

    _meshEditor->setEditedMesh(_objects[index].node, _highlightedMeshInstance, &_objects[index].materials, _objects[index].baseModel);
}

void SceneEditorWidget::flushItemUi(int index)
{
    ui->translate_x->blockSignals(true);
    ui->translate_y->blockSignals(true);
    ui->translate_z->blockSignals(true);
    ui->scale_x->blockSignals(true);
    ui->scale_y->blockSignals(true);
    ui->scale_z->blockSignals(true);
    ui->name->blockSignals(true);

    if(index < 0)
    {
        ui->name->setText("");

        ui->translate_x->setValue(0);
        ui->translate_y->setValue(0);
        ui->translate_z->setValue(0);

        ui->scale_x->setValue(1);
        ui->scale_y->setValue(1);
        ui->scale_z->setValue(1);
    }
    else
    {
        ui->name->setText(_objects[index].name);

        ui->translate_x->setValue(_objects[index].translate.x());
        ui->translate_y->setValue(_objects[index].translate.y());
        ui->translate_z->setValue(_objects[index].translate.z());

        ui->scale_x->setValue(_objects[index].scale.x());
        ui->scale_y->setValue(_objects[index].scale.y());
        ui->scale_z->setValue(_objects[index].scale.z());
    }


    ui->translate_x->blockSignals(false);
    ui->translate_y->blockSignals(false);
    ui->translate_z->blockSignals(false);
    ui->scale_x->blockSignals(false);
    ui->scale_y->blockSignals(false);
    ui->scale_z->blockSignals(false);
    ui->name->blockSignals(false);
}

void SceneEditorWidget::updateSelectedMeshMatrix()
{
    _renderer->addEvent([=]{
        mat4 m = constructTransformation(_objects[_curItemIndex]);
        _objects[_curItemIndex].node->setMatrix(m);
        _highlightedMeshInstance->setMatrix(m);
    });
}

void SceneEditorWidget::activateLastAdded()
{
    if(_objects.isEmpty()) return;

    activateObject(_objects.size()-1);
}

/* slots */
void SceneEditorWidget::sceneItemActivated(QListWidgetItem* item)
{
    for(int i=0 ; i<_objects.size() ; ++i)
    {
        if(_objects[i].listItem == item)
        {
            activateObject(i);
            return;
        }
    }
}

void SceneEditorWidget::on_translate_x_editingFinished()
{
    if(_curItemIndex < 0)
        return;

    _objects[_curItemIndex].translate.set(ui->translate_x->value(), 0);
    updateSelectedMeshMatrix();
}

void SceneEditorWidget::on_translate_y_editingFinished()
{
    if(_curItemIndex < 0)
        return;

    _objects[_curItemIndex].translate.set(ui->translate_y->value(), 1);
    updateSelectedMeshMatrix();
}

void SceneEditorWidget::on_translate_z_editingFinished()
{
    if(_curItemIndex < 0)
        return;

    _objects[_curItemIndex].translate.set(ui->translate_z->value(), 2);
    updateSelectedMeshMatrix();
}

void SceneEditorWidget::on_scale_x_editingFinished()
{
    if(_curItemIndex < 0 && ui->scale_x->value() != 0)
        return;

    _objects[_curItemIndex].scale.set(ui->scale_x->value(), 0);
    updateSelectedMeshMatrix();
}

void SceneEditorWidget::on_scale_y_editingFinished()
{
    if(_curItemIndex < 0 && ui->scale_y->value() != 0)
        return;

    _objects[_curItemIndex].scale.set(ui->scale_y->value(), 1);
    updateSelectedMeshMatrix();
}

void SceneEditorWidget::on_scale_z_editingFinished()
{
    if(_curItemIndex < 0 && ui->scale_z->value() != 0)
        return;

    _objects[_curItemIndex].scale.set(ui->scale_z->value(), 2);
    updateSelectedMeshMatrix();
}

void SceneEditorWidget::edit_cameraPosX(double x)
{
    _renderer->addEvent([=](){
        vec3 dir = _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.dir - _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.pos;
        _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.pos.x() = x;
        _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.dir = _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.pos + dir;
    });
}

void SceneEditorWidget::edit_cameraPosY(double y)
{
    _renderer->addEvent([=](){
        vec3 dir = _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.dir - _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.pos;
        _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.pos.y() = y;
        _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.dir = _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.pos + dir;
    });
}

void SceneEditorWidget::edit_cameraPosZ(double z)
{
    _renderer->addEvent([=](){
        vec3 dir = _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.dir - _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.pos;
        _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.pos.z() = z;
        _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.dir = _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.pos + dir;
    });
}

void SceneEditorWidget::on_name_editingFinished()
{
    if(_curItemIndex < 0)
        return;

    _objects[_curItemIndex].name = ui->name->text();

    QString n;
    if(_objects[_curItemIndex].name.isEmpty())
        n = "model : " + _objects[_curItemIndex].baseModel;
    else if(_objects[_curItemIndex].baseModel.isEmpty())
        n = _objects[_curItemIndex].name;
    else
        n = _objects[_curItemIndex].name + " (" + _objects[_curItemIndex].baseModel + ")";

    _objects[_curItemIndex].listItem->setText(n);
}

float computeGrade(float dist, float ray)
{
    if(dist > 50) return 100.f / (1000000 * ray);
    if(ray > 20) return 100.f / (1000 * dist);
    return 100.f / (dist * ray);
}

void SceneEditorWidget::selectSceneObject(vec3 pos, vec3 dir)
{
    vector<std::reference_wrapper<MeshInstance>> result;
    _renderer->getScene(_renderer->getCurSceneIndex()).scene.query<MeshInstance>(RayCast(pos, dir),
                                                                                  VectorInserter<vector<std::reference_wrapper<MeshInstance>>>(result));

    if(result.empty())
        return;

    float curGrade = -1;
    uint index = 0;

    for(size_t i=0 ; i<result.size() ; ++i)
    {
        float grade = computeGrade((pos-result[i].get().volume().center()).length(), result[i].get().volume().radius());
        if(grade > curGrade && &result[i].get() != _highlightedMeshInstance)
        {
            curGrade = grade;
            index = i;
        }
    }

    for(int i=0 ; i<_objects.size() ; ++i)
    {
        if(_objects[i].node == &result[index].get())
        {
            activateObject(i);
            _objects[i].listItem->setSelected(true);
            return;
        }
    }
}

float combine(float x, float y)
{
    return x+y;
    if(x <= 0 && y <= 0)
        return x+y;
    else if(x >= 0 && y >= 0)
        return x+y;
    else
    {
        if(fabsf(x) > fabs(y))
            return x;
        else return y;
    }
}

void SceneEditorWidget::translateMouse(float x, float y, int mode)
{
    if(_curItemIndex == -1)
        return;

    Camera& cam = _renderer->getSceneView(_renderer->getCurSceneIndex()).camera;

    float hsize = (_objects[_curItemIndex].translate - cam.pos).length() * tanf(toRad(cam.fov));

    const float SCALE_FACTOR = 5;
    const float ROTATE_FACTOR = 5;

    if(mode == RendererWidget::TRANSLATE_MODE)
    {
        vec3 forward = (cam.dir - cam.pos);
        vec3 x_dir = forward.cross(cam.up).normalized();
        vec3 y_dir = x_dir.cross(forward).normalized();

        _objects[_curItemIndex].translate -= x_dir * x * hsize;
        _objects[_curItemIndex].translate += y_dir * y * (hsize/cam.ratio);
    }
    else if(mode == RendererWidget::TRANSLATE_X_MODE)
    {
        _objects[_curItemIndex].translate.x() -= combine(x,y) * hsize;
    }
    else if(mode == RendererWidget::TRANSLATE_Y_MODE)
    {
        _objects[_curItemIndex].translate.y() -= combine(x,y) * hsize;
    }
    else if(mode == RendererWidget::TRANSLATE_Z_MODE)
    {
        _objects[_curItemIndex].translate.z() -= combine(x,y) * hsize;
    }

    else if(mode == RendererWidget::SCALE_MODE)
    {
        _objects[_curItemIndex].scale *= (1.f + combine(x,y)*SCALE_FACTOR);
    }
    else if(mode == RendererWidget::SCALE_X_MODE)
    {
        _objects[_curItemIndex].scale.x() *= (1.f + combine(x,y)*SCALE_FACTOR);
    }
    else if(mode == RendererWidget::SCALE_Y_MODE)
    {
        _objects[_curItemIndex].scale.y() *= (1.f + combine(x,y)*SCALE_FACTOR);
    }
    else if(mode == RendererWidget::SCALE_Z_MODE)
    {
        _objects[_curItemIndex].scale.z() *= (1.f + combine(x,y)*SCALE_FACTOR);
    }

    else if(mode == RendererWidget::ROTATE_MODE)
    {
        vec3 forward = (cam.dir - cam.pos);
        QMatrix3x3 m = QQuaternion::fromAxisAndAngle(QVector3D(forward[0], forward[1], forward[2]).normalized(),
                                                     toDeg(combine(x,y)*ROTATE_FACTOR)).toRotationMatrix();
        mat3 tim_mat(m.constData());

        _objects[_curItemIndex].rotate = tim_mat * _objects[_curItemIndex].rotate;
    }
    else if(mode == RendererWidget::ROTATE_X_MODE)
    {
        _objects[_curItemIndex].rotate = mat3::RotationX(combine(x,y)*ROTATE_FACTOR) * _objects[_curItemIndex].rotate;
    }
    else if(mode == RendererWidget::ROTATE_Y_MODE)
    {
        _objects[_curItemIndex].rotate = mat3::RotationY(combine(x,y)*ROTATE_FACTOR) * _objects[_curItemIndex].rotate;
    }
    else if(mode == RendererWidget::ROTATE_Z_MODE)
    {
        _objects[_curItemIndex].rotate = mat3::RotationZ(combine(x,y)*ROTATE_FACTOR) * _objects[_curItemIndex].rotate;
    }

    updateSelectedMeshMatrix();
}

void SceneEditorWidget::flushState()
{
    ui->camera_pos_x->blockSignals(true);
    ui->camera_pos_y->blockSignals(true);
    ui->camera_pos_z->blockSignals(true);

    ui->camera_pos_x->setValue(_renderer->getSceneView(_renderer->getCurSceneIndex()).camera.pos.x());
    ui->camera_pos_y->setValue(_renderer->getSceneView(_renderer->getCurSceneIndex()).camera.pos.y());
    ui->camera_pos_z->setValue(_renderer->getSceneView(_renderer->getCurSceneIndex()).camera.pos.z());

    ui->camera_pos_x->blockSignals(false);
    ui->camera_pos_y->blockSignals(false);
    ui->camera_pos_z->blockSignals(false);

    vec3 dir = _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.dir - _renderer->getSceneView(_renderer->getCurSceneIndex()).camera.pos;
    dir.normalize();
    ui->camera_dir_x->setText(QString::number(dir.x(), 'g', 4));
    ui->camera_dir_y->setText(QString::number(dir.y(), 'g', 4));
    ui->camera_dir_z->setText(QString::number(dir.z(), 'g', 4));
}

void SceneEditorWidget::cancelSelection()
{
    flushItemUi(-1);
    _curItemIndex = -1;

    if(_highlightedMeshInstance != nullptr)
        _highlightedMeshInstance->setEnable(false);

    _meshEditor->setEditedMesh(nullptr, nullptr, nullptr, "");
}

void SceneEditorWidget::saveCurMeshTrans()
{
    if(_curItemIndex == -1)
        return;

    saved_rotate = _objects[_curItemIndex].rotate;
    saved_translate = _objects[_curItemIndex].translate;
    saved_scale = _objects[_curItemIndex].scale;
}

void SceneEditorWidget::restoreCurMeshTrans()
{
    if(_curItemIndex == -1)
        return;

    _objects[_curItemIndex].rotate = saved_rotate;
    _objects[_curItemIndex].translate = saved_translate;
    _objects[_curItemIndex].scale = saved_scale;

    updateSelectedMeshMatrix();
}

void SceneEditorWidget::flushUiAccordingState(int state)
{
    for(int i=0 ; i<3 ; ++i)
    {
        if(_translateLine[i] != nullptr)
            _translateLine[i]->setEnable(false);
    }

    if(state >= RendererWidget::TRANSLATE_X_MODE && state <= RendererWidget::TRANSLATE_Z_MODE)
    {
        if(_curItemIndex < 0)
            return;

        int index = state - static_cast<int>(RendererWidget::TRANSLATE_X_MODE);

        if(_translateLine[index] == nullptr)
        {
            _renderer->lock();
            _translateLine[index] = &_renderer->getScene(_renderer->getCurSceneIndex()).scene.add<MeshInstance>(_renderer->lineMesh(index), mat4::IDENTITY());
            _renderer->unlock();
        }

        _translateLine[index]->setMatrix(mat4::Translation(_objects[_curItemIndex].translate));
        _translateLine[index]->setEnable(true);

    }
    else if(state >= RendererWidget::SCALE_X_MODE && state <= RendererWidget::SCALE_Z_MODE)
    {
        if(_curItemIndex < 0)
            return;

        int index = state - static_cast<int>(RendererWidget::SCALE_X_MODE);

        if(_translateLine[index] == nullptr)
        {
            _renderer->lock();
            _translateLine[index] = &_renderer->getScene(_renderer->getCurSceneIndex()).scene.add<MeshInstance>(_renderer->lineMesh(index), mat4::IDENTITY());
            _renderer->unlock();
        }

        _translateLine[index]->setMatrix(_objects[_curItemIndex].node->matrix());
        _translateLine[index]->setEnable(true);

    }
    else if(state >= RendererWidget::ROTATE_X_MODE && state <= RendererWidget::ROTATE_Z_MODE)
    {
        if(_curItemIndex < 0)
            return;

        int index = state - static_cast<int>(RendererWidget::ROTATE_X_MODE);

        if(_translateLine[index] == nullptr)
        {
            _renderer->lock();
            _translateLine[index] = &_renderer->getScene(_renderer->getCurSceneIndex()).scene.add<MeshInstance>(_renderer->lineMesh(index), mat4::IDENTITY());
            _renderer->unlock();
        }

        _translateLine[index]->setMatrix(mat4::Translation(_objects[_curItemIndex].translate));
        _translateLine[index]->setEnable(true);

    }

    if(state >= RendererWidget::TRANSLATE_MODE && state <= RendererWidget::ROTATE_Z_MODE)
    {
        if(_highlightedMeshInstance)
        {
            Mesh m = _highlightedMeshInstance->mesh();
            for(uint i=0 ; i<m.nbElements() ; ++i)
                m.element(i).drawState().setShader(ShaderPool::instance().get("highlightedMoving"));
            _highlightedMeshInstance->setMesh(m);
        }
    }
    else
    {
        if(_highlightedMeshInstance)
        {
            Mesh m = _highlightedMeshInstance->mesh();
            for(uint i=0 ; i<m.nbElements() ; ++i)
                m.element(i).drawState().setShader(ShaderPool::instance().get("highlighted"));
            _highlightedMeshInstance->setMesh(m);
        }
    }
}

void SceneEditorWidget::resetRotation()
{
    if(_curItemIndex < 0)
        return;

    _objects[_curItemIndex].rotate = mat3::IDENTITY();
    updateSelectedMeshMatrix();
}


void SceneEditorWidget::deleteCurrentObject()
{
    if(_curItemIndex < 0)
        return;

    if(_objects[_curItemIndex].node)
        _renderer->getScene(_renderer->getCurSceneIndex()).scene.remove(*_objects[_curItemIndex].node);

    delete _objects[_curItemIndex].listItem;

    _objects.removeAt(_curItemIndex);
    cancelSelection();

}

void SceneEditorWidget::copyObject()
{

}
