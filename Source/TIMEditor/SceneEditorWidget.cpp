#include "SceneEditorWidget.h"
#include "ui_SceneEditor.h"
#include "AssetViewWidget.h"
#include "EditorWindow.h"

#include "MeshEditorWidget.h"
#include "RendererWidget.h"
#include "core/Matrix.h"

#include <QQuaternion>
#include <QMatrix3x3>

#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QProgressDialog>

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

    Pipeline::DirectionalLight l;
    l.color = {1,1,1,1};
    l.direction = vec3(-0.86,-0.26,-0.43);
    l.projectShadow = true;

    for(uint i=0 ; i<NB_SCENE ; ++i)
    {
        _directionalLights[i] += l;
        r->setDirectionalLight(i+1, l);
    }
}

void SceneEditorWidget::setSkybox(uint sceneIndex, const QList<QString>& skyboxs)
{
    _skyboxs[sceneIndex] = skyboxs;
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

void SceneEditorWidget::addSceneObject(QString name, QString modelName, const QList<MeshElement>& model,
                                       const mat3& rot, const vec3& tr, const vec3& scale)
{
    addSceneObject(_curSceneIndex, true, name, modelName, model, rot, tr, scale);
}

void SceneEditorWidget::addSceneObject(int sceneIndex, bool lock, QString name, QString modelName, const QList<MeshElement>& model,
                                       const mat3& rot, const vec3& tr, const vec3& scale)
{
    SceneObject obj;
    obj.baseModel = modelName;
    obj.name = name;
    obj.materials = model;

    obj.rotate = rot;
    obj.scale = scale;
    obj.translate = tr;

    mat4 trans = mat4::constructTransformation(obj.rotate, obj.translate, obj.scale);

    if(lock) _renderer->lock();
    obj.node = &_renderer->getScene(sceneIndex+1).scene.add<MeshInstance>(trans);
    if(lock) _renderer->unlock();

    QString n;
    if(name.isEmpty()) n = "model : " + modelName;
    else if(model.isEmpty()) n = name;
    else n = name + " (" + modelName + ")";

    obj.listItem = new QListWidgetItem(n, ui->listSceneObject);
    if(sceneIndex == _renderer->getCurSceneIndex()-1)
        ui->listSceneObject->addItem(obj.listItem);

    _objects[sceneIndex] += obj;

    _renderer->addEvent([=]{
        Mesh m;
        for(int i=0 ; i<obj.materials.size() ; ++i)
            m.addElement(MeshEditorWidget::constructMeshElement(obj.materials[i]));

//        if(modelName == "portal" && sceneIndex < 2)
//        {
//            int sceneTo = sceneIndex==0 ? 1:0;
//            std::cout << "Portal detected\n";
//            MultipleSceneHelper::Edge edge;
//            edge.sceneFrom = &_renderer->getScene(sceneIndex+1);
//            edge.sceneTo = &_renderer->getScene(sceneTo+1);
//            edge.portal = obj.node;
//            edge.portalGeom = m.element(0).geometry();

//            _renderer->portalsManager()->addEdge(edge);
//            m.element(0).drawState().setCullBackFace(true);
//            m.element(0).drawState().setShader(ShaderPool::instance().get("portalShader"));
//        }

        obj.node->setMesh(m);
    });
}

void SceneEditorWidget::activateObject(int index)
{
    if(index >= _objects[_curSceneIndex].size())
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
    _highlightedMeshInstance->setMatrix(_objects[_curSceneIndex][index].node->matrix());
    _highlightedMeshInstance->setMesh(MeshEditorWidget::highlightMesh(_objects[_curSceneIndex][index].node->mesh()));

    _renderer->unlock();

    _meshEditor->setEditedMesh(_objects[_curSceneIndex][index].node, _highlightedMeshInstance,
                               &_objects[_curSceneIndex][index].materials, _objects[_curSceneIndex][index].baseModel);
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
        ui->name->setText(_objects[_curSceneIndex][index].name);

        ui->translate_x->setValue(_objects[_curSceneIndex][index].translate.x());
        ui->translate_y->setValue(_objects[_curSceneIndex][index].translate.y());
        ui->translate_z->setValue(_objects[_curSceneIndex][index].translate.z());

        ui->scale_x->setValue(_objects[_curSceneIndex][index].scale.x());
        ui->scale_y->setValue(_objects[_curSceneIndex][index].scale.y());
        ui->scale_z->setValue(_objects[_curSceneIndex][index].scale.z());
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
        mat4 m = mat4::constructTransformation(_objects[_curSceneIndex][_curItemIndex].rotate,
                                               _objects[_curSceneIndex][_curItemIndex].translate,
                                               _objects[_curSceneIndex][_curItemIndex].scale);
        _objects[_curSceneIndex][_curItemIndex].node->setMatrix(m);
        _highlightedMeshInstance->setMatrix(m);
    });
}

void SceneEditorWidget::activateLastAdded()
{
    if(_objects[_curSceneIndex].isEmpty()) return;

    activateObject(_objects[_curSceneIndex].size()-1);
}

/* slots */
void SceneEditorWidget::sceneItemActivated(QListWidgetItem* item)
{
    for(int i=0 ; i<_objects[_curSceneIndex].size() ; ++i)
    {
        if(_objects[_curSceneIndex][i].listItem == item)
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

    _objects[_curSceneIndex][_curItemIndex].translate.set(ui->translate_x->value(), 0);
    updateSelectedMeshMatrix();
}

void SceneEditorWidget::on_translate_y_editingFinished()
{
    if(_curItemIndex < 0)
        return;

    _objects[_curSceneIndex][_curItemIndex].translate.set(ui->translate_y->value(), 1);
    updateSelectedMeshMatrix();
}

void SceneEditorWidget::on_translate_z_editingFinished()
{
    if(_curItemIndex < 0)
        return;

    _objects[_curSceneIndex][_curItemIndex].translate.set(ui->translate_z->value(), 2);
    updateSelectedMeshMatrix();
}

void SceneEditorWidget::on_scale_x_editingFinished()
{
    if(_curItemIndex < 0 && ui->scale_x->value() != 0)
        return;

    _objects[_curSceneIndex][_curItemIndex].scale.set(ui->scale_x->value(), 0);
    updateSelectedMeshMatrix();
}

void SceneEditorWidget::on_scale_y_editingFinished()
{
    if(_curItemIndex < 0 && ui->scale_y->value() != 0)
        return;

    _objects[_curSceneIndex][_curItemIndex].scale.set(ui->scale_y->value(), 1);
    updateSelectedMeshMatrix();
}

void SceneEditorWidget::on_scale_z_editingFinished()
{
    if(_curItemIndex < 0 && ui->scale_z->value() != 0)
        return;

    _objects[_curSceneIndex][_curItemIndex].scale.set(ui->scale_z->value(), 2);
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

    _objects[_curSceneIndex][_curItemIndex].name = ui->name->text();

    QString n;
    if(_objects[_curSceneIndex][_curItemIndex].name.isEmpty())
        n = "model : " + _objects[_curSceneIndex][_curItemIndex].baseModel;
    else if(_objects[_curSceneIndex][_curItemIndex].baseModel.isEmpty())
        n = _objects[_curSceneIndex][_curItemIndex].name;
    else
        n = _objects[_curSceneIndex][_curItemIndex].name + " (" + _objects[_curSceneIndex][_curItemIndex].baseModel + ")";

    _objects[_curSceneIndex][_curItemIndex].listItem->setText(n);
}

void SceneEditorWidget::on_copyTransButton_clicked()
{
    if(_curItemIndex >= 0)
    {
        copy_rotate = _objects[_curSceneIndex][_curItemIndex].rotate;
        copy_scale = _objects[_curSceneIndex][_curItemIndex].scale;
        copy_translate = _objects[_curSceneIndex][_curItemIndex].translate;
        somethingCopied = true;
    }
}

void SceneEditorWidget::on_pastTransButton_clicked()
{
    if(_curItemIndex >= 0 && somethingCopied)
    {
        _objects[_curSceneIndex][_curItemIndex].rotate = copy_rotate;
        _objects[_curSceneIndex][_curItemIndex].scale = copy_scale;
        _objects[_curSceneIndex][_curItemIndex].translate = copy_translate;
        updateSelectedMeshMatrix();
    }
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

    for(int i=0 ; i<_objects[_curSceneIndex].size() ; ++i)
    {
        if(_objects[_curSceneIndex][i].node == &result[index].get())
        {
            activateObject(i);
            _objects[_curSceneIndex][i].listItem->setSelected(true);
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

    float hsize = (_objects[_curSceneIndex][_curItemIndex].translate - cam.pos).length() * tanf(toRad(cam.fov));

    const float SCALE_FACTOR = 5;
    const float ROTATE_FACTOR = 8;

    vec3 forward = (cam.dir - cam.pos);
    vec3 x_dir = forward.cross(cam.up).normalized();
    vec3 y_dir = x_dir.cross(forward).normalized();

    if(mode == RendererWidget::TRANSLATE_MODE)
    {
        _objects[_curSceneIndex][_curItemIndex].translate -= x_dir * x * hsize;
        _objects[_curSceneIndex][_curItemIndex].translate += y_dir * y * (hsize/cam.ratio);
    }
    else if(mode == RendererWidget::TRANSLATE_X_MODE)
    {
        if(x_dir.x() > 0) x *= -1;
        if(y_dir.x() < 0) y *= -1;
        _objects[_curSceneIndex][_curItemIndex].translate.x() += combine(x,y) * hsize;
    }
    else if(mode == RendererWidget::TRANSLATE_Y_MODE)
    {
        if(x_dir.y() > 0) x *= -1;
        if(y_dir.y() < 0) y *= -1;
        _objects[_curSceneIndex][_curItemIndex].translate.y() += combine(x,y) * hsize;
    }
    else if(mode == RendererWidget::TRANSLATE_Z_MODE)
    {
        if(x_dir.z() > 0) x *= -1;
        if(y_dir.z() < 0) y *= -1;
        _objects[_curSceneIndex][_curItemIndex].translate.z() += combine(x,y) * hsize;
    }

    else if(mode == RendererWidget::SCALE_MODE)
    {
        _objects[_curSceneIndex][_curItemIndex].scale *= (1.f + combine(x,y)*SCALE_FACTOR);
    }
    else if(mode == RendererWidget::SCALE_X_MODE)
    {
        _objects[_curSceneIndex][_curItemIndex].scale.x() *= (1.f + combine(x,y)*SCALE_FACTOR);
    }
    else if(mode == RendererWidget::SCALE_Y_MODE)
    {
        _objects[_curSceneIndex][_curItemIndex].scale.y() *= (1.f + combine(x,y)*SCALE_FACTOR);
    }
    else if(mode == RendererWidget::SCALE_Z_MODE)
    {
        _objects[_curSceneIndex][_curItemIndex].scale.z() *= (1.f + combine(x,y)*SCALE_FACTOR);
    }

    else if(mode == RendererWidget::ROTATE_MODE)
    {
        QMatrix3x3 m = QQuaternion::fromAxisAndAngle(QVector3D(forward[0], forward[1], forward[2]).normalized(),
                                                     toDeg(combine(x,y)*ROTATE_FACTOR)).toRotationMatrix();
        mat3 tim_mat(m.constData());

        _objects[_curSceneIndex][_curItemIndex].rotate = tim_mat * _objects[_curSceneIndex][_curItemIndex].rotate;
    }
    else if(mode == RendererWidget::ROTATE_X_MODE)
    {
        _objects[_curSceneIndex][_curItemIndex].rotate = mat3::RotationX(combine(x,y)*ROTATE_FACTOR) * _objects[_curSceneIndex][_curItemIndex].rotate;
    }
    else if(mode == RendererWidget::ROTATE_Y_MODE)
    {
        _objects[_curSceneIndex][_curItemIndex].rotate = mat3::RotationY(combine(x,y)*ROTATE_FACTOR) * _objects[_curSceneIndex][_curItemIndex].rotate;
    }
    else if(mode == RendererWidget::ROTATE_Z_MODE)
    {
        _objects[_curSceneIndex][_curItemIndex].rotate = mat3::RotationZ(combine(x,y)*ROTATE_FACTOR) * _objects[_curSceneIndex][_curItemIndex].rotate;
    }

    updateSelectedMeshMatrix();
    flushItemUi(_curItemIndex);
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

    saved_rotate = _objects[_curSceneIndex][_curItemIndex].rotate;
    saved_translate = _objects[_curSceneIndex][_curItemIndex].translate;
    saved_scale = _objects[_curSceneIndex][_curItemIndex].scale;
}

void SceneEditorWidget::restoreCurMeshTrans()
{
    if(_curItemIndex == -1)
        return;

    _objects[_curSceneIndex][_curItemIndex].rotate = saved_rotate;
    _objects[_curSceneIndex][_curItemIndex].translate = saved_translate;
    _objects[_curSceneIndex][_curItemIndex].scale = saved_scale;

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

        _translateLine[index]->setMatrix(mat4::Translation(_objects[_curSceneIndex][_curItemIndex].translate));
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

        _translateLine[index]->setMatrix(_objects[_curSceneIndex][_curItemIndex].node->matrix());
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

        _translateLine[index]->setMatrix(mat4::Translation(_objects[_curSceneIndex][_curItemIndex].translate));
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

    _objects[_curSceneIndex][_curItemIndex].rotate = mat3::IDENTITY();
    updateSelectedMeshMatrix();
}


void SceneEditorWidget::deleteCurrentObject()
{
    if(_curItemIndex < 0)
        return;

    if(_objects[_curSceneIndex][_curItemIndex].node)
        _renderer->getScene(_renderer->getCurSceneIndex()).scene.remove(*_objects[_curSceneIndex][_curItemIndex].node);

    delete _objects[_curSceneIndex][_curItemIndex].listItem;

    _objects[_curSceneIndex].removeAt(_curItemIndex);
    cancelSelection();

}

void SceneEditorWidget::copyObject()
{
    if(_curItemIndex < 0 || _renderer->getCurSceneIndex() == 0)
        return;

    QString n = _objects[_curSceneIndex][_curItemIndex].name;
    if(!n.isEmpty())
    {
        int index = n.lastIndexOf('_');
        if(index < 0)
            n += "_1";
        else
        {
            index++;
            if(index < n.size())
            {
                QString numb = n.mid(index, n.size()-index);
                int i = numb.toInt()+1;
                n.resize(index);
                n += QString::number(i);
            }
        }
    }

    addSceneObject(n, _objects[_curSceneIndex][_curItemIndex].baseModel,
                      _objects[_curSceneIndex][_curItemIndex].materials,
                      _objects[_curSceneIndex][_curItemIndex].rotate,
                      _objects[_curSceneIndex][_curItemIndex].translate,
                      _objects[_curSceneIndex][_curItemIndex].scale);

    _renderer->waitNoEvent();
    activateObject(_objects[_curSceneIndex].size()-1);

    emit editTransformation(0);
}

void SceneEditorWidget::exportScene(QString filePath, int sceneIndex)
{
    QFile file(filePath);
    QDir destDir(".");
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);

        if(_skyboxs[sceneIndex].size() == 6)
        {
            stream << "<Skybox>\n";
            stream << "   <x>" << destDir.relativeFilePath(_skyboxs[sceneIndex][0]) << "</x>\n";
            stream << "   <nx>" << destDir.relativeFilePath(_skyboxs[sceneIndex][1]) << "</nx>\n";
            stream << "   <y>" << destDir.relativeFilePath(_skyboxs[sceneIndex][2]) << "</y>\n";
            stream << "   <ny>" << destDir.relativeFilePath(_skyboxs[sceneIndex][3]) << "</ny>\n";
            stream << "   <z>" << destDir.relativeFilePath(_skyboxs[sceneIndex][4]) << "</z>\n";
            stream << "   <nz>" << destDir.relativeFilePath(_skyboxs[sceneIndex][5]) << "</nz>\n";
            stream << "</Skybox>\n";
            stream << "\n";
        }

        for(int i=0 ; i<_directionalLights[sceneIndex].size() ; ++i)
        {
            vec4 col = _directionalLights[sceneIndex][i].color;
            vec3 dir = _directionalLights[sceneIndex][i].direction;
            stream << "<DirLight shadows=" << _directionalLights[sceneIndex][i].projectShadow
                   << " color=\"" << col.x() << "," << col.y() << "," << col.z() << "\" "
                   << "direction=\"" << dir.x() << "," << dir.y() << "," << dir.z() << "\" />\n";
        }
        stream << "\n";

        QList<QList<MeshElement>> alreadySaved;

        for(int i=0 ; i<_objects[sceneIndex].size() ; ++i)
        {
            QList<MeshElement> sortedElem = _objects[sceneIndex][i].materials;
            qSort(sortedElem);
            int index = alreadySaved.indexOf(sortedElem);
            if(index == -1)
            {
                alreadySaved += sortedElem;
                _objects[sceneIndex][i].exportHelper = alreadySaved.size()-1;

                stream << "<MeshAsset name=\"" << _objects[sceneIndex][i].baseModel << "\" index=" << alreadySaved.size()-1 << " >\n";
                for(auto m : sortedElem)
                    AssetViewWidget::writeMaterial(m, stream, destDir, "\t");
                stream << "</MeshAsset>\n";
            }
            else
            {
                _objects[sceneIndex][i].exportHelper = index;
            }
        }

        stream << "\n";
        for(int i=0 ; i<_objects[sceneIndex].size() ; ++i)
        {
            stream << "<Object name=\"" << _objects[sceneIndex][i].name << "\" model=" <<  _objects[sceneIndex][i].exportHelper << " >\n";
            stream << "   <translate>" << _objects[sceneIndex][i].translate[0] << "," << _objects[sceneIndex][i].translate[1] << "," << _objects[sceneIndex][i].translate[2] << "</translate>\n";
            stream << "   <scale>" << _objects[sceneIndex][i].scale[0] << "," << _objects[sceneIndex][i].scale[1] << "," << _objects[sceneIndex][i].scale[2] << "</scale>\n";

            stream << "   <rotate>";
            for(int j=0 ; j<9 ; ++j)
                stream << _objects[sceneIndex][i].rotate.get(j) << ((j!=8)?",":"");
            stream << "</rotate>\n";
            stream << "</Object>\n";
        }

    }
}

void SceneEditorWidget::parseTransformation(TiXmlElement* elem, vec3& tr, vec3& sc, mat3& rot)
{
    tr={0,0,0};
    sc={1,1,1};
    rot=mat3::IDENTITY();

    elem = elem->FirstChildElement();

    while(elem)
    {
        if(elem->ValueStr() == std::string("translate"))
            tr = toVec<3>(StringUtils::str(elem->GetText()));
        else if(elem->ValueStr() == std::string("scale"))
            sc = toVec<3>(StringUtils::str(elem->GetText()));
        else if(elem->ValueStr() == std::string("rotate"))
        {
            Vector<float, 9> r = toVec<9>(StringUtils::str(elem->GetText()));

            for(int i=0 ; i<9 ; ++i)
                rot.get(i) = r[i];
        }

        elem = elem->NextSiblingElement();
    }
}

QList<QString> SceneEditorWidget::parseSkyboxXmlElement(TiXmlElement* elem)
{
    QVector<QString> res(6);

    elem = elem->FirstChildElement();

    while(elem)
    {
        if(elem->ValueStr() == std::string("x"))
            res[0] = QString::fromStdString(StringUtils::str(elem->GetText()));
        else if(elem->ValueStr() == std::string("nx"))
            res[1] = QString::fromStdString(StringUtils::str(elem->GetText()));
        else if(elem->ValueStr() == std::string("y"))
            res[2] = QString::fromStdString(StringUtils::str(elem->GetText()));
        else if(elem->ValueStr() == std::string("ny"))
            res[3] = QString::fromStdString(StringUtils::str(elem->GetText()));
        else if(elem->ValueStr() == std::string("z"))
            res[4] = QString::fromStdString(StringUtils::str(elem->GetText()));
        else if(elem->ValueStr() == std::string("nz"))
            res[5] = QString::fromStdString(StringUtils::str(elem->GetText()));

        elem = elem->NextSiblingElement();
    }

    return QList<QString>::fromVector(res);
}

void SceneEditorWidget::importScene(QString file, int sceneIndex)
{
    clearScene(sceneIndex);

    TiXmlDocument doc(file.toStdString());

    if(!doc.LoadFile())
        return;

    TiXmlElement* root=doc.FirstChildElement();
    TiXmlElement* elem = root;

    // first parse meshasset
    QMap<int, QList<MeshElement>> meshAssets;
    QMap<int, QString> meshAssetsName;

    int nbObject=0;
    _renderer->getScene(sceneIndex+1).globalLight.dirLights.clear();

    while(elem)
    {
        if(elem->ValueStr() == std::string("MeshAsset"))
        {
            std::string name;
            auto asset = interface::XmlMeshAssetLoader::parseMeshAssetElement(elem, name);

            int index=-1;
            elem->QueryIntAttribute("index", &index);

            meshAssets[index] = _meshEditor->convertFromEngine(asset);
            meshAssetsName[index] = QString::fromStdString(name);
        }
        else if(elem->ValueStr() == std::string("Object"))
        {
            nbObject++;
        }

        elem=elem->NextSiblingElement();
    }

    elem = root;
    QList<QString> skybox;

    _renderer->waitNoEvent();
    _renderer->lock();
    while(elem)
    {
        if(elem->ValueStr() == std::string("Object"))
        {
            std::string name;
            elem->QueryStringAttribute("name", &name);

            int index=-1;
            elem->QueryIntAttribute("model", &index);

            if(index < 0 || !meshAssets.contains(index))
                continue;

            vec3 tr, sc;
            mat3 rot;
            parseTransformation(elem, tr, sc, rot);
            addSceneObject(sceneIndex, false, QString::fromStdString(name), meshAssetsName[index], meshAssets[index], rot, tr, sc);
        }
        else if(elem->ValueStr() == std::string("Skybox"))
        {
            skybox = parseSkyboxXmlElement(elem);
        }
        else if(elem->ValueStr() == std::string("DirLight"))
        {
            int shadow=0;
            vec3 color = {1,1,1};
            vec3 dir={0,0,-1};

            elem->QueryIntAttribute("shadows", &shadow);
            std::string strColor = StringUtils::str(elem->Attribute("color"));
            std::string strDir = StringUtils::str(elem->Attribute("direction"));
            if(!strColor.empty()) color = toVec<3>(strColor);
            if(!strDir.empty()) dir = toVec<3>(strDir);

            _renderer->getScene(sceneIndex+1).globalLight.dirLights.push_back({dir, vec4(color,1), shadow==1});
        }

        elem=elem->NextSiblingElement();
    }

    if(skybox.size() == 6)
    {
        setSkybox(sceneIndex, skybox);
        _renderer->addEvent([=](){
            _renderer->setSkybox(sceneIndex+1, skybox);
        });
    }

    _renderer->unlock();
    _renderer->waitNoEvent();
}

void SceneEditorWidget::clearScene(int index)
{
    cancelSelection();

    _renderer->waitNoEvent();
    _renderer->lock();

    if(_curSceneIndex == index)
        ui->listSceneObject->clear();

    for(int i=0 ; i<_objects[index].size() ; ++i)
    {
        if(_curSceneIndex != index)
            delete _objects[index][i].listItem;
        _renderer->getScene(index+1).scene.remove(*_objects[index][i].node);
    }
    _renderer->unlock();

    setSkybox(uint(index), QList<QString>());
    _renderer->addEvent([=](){
        _renderer->setSkybox(index+1, QList<QString>());
    });

    _objects[index].clear();
}

void SceneEditorWidget::switchScene(int index)
{
    if(_curSceneIndex == index)
        return;

    cancelSelection();

    if(_highlightedMeshInstance)
    {
        _renderer->getScene(_curSceneIndex).scene.remove(*_highlightedMeshInstance);
        _highlightedMeshInstance = nullptr;
    }

    _curSceneIndex = index;

    while(ui->listSceneObject->count() > 0)
        ui->listSceneObject->takeItem(0);

    for(int i=0 ; i<_objects[_curSceneIndex].size() ; ++i)
    {
        ui->listSceneObject->addItem(_objects[_curSceneIndex][i].listItem);
    }
}
