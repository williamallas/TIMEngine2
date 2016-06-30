#include "MainRenderer.h"
#include "RendererWidget.h"

#include <QMimeData>

RendererWidget::RendererWidget(QWidget *parent) :
    QGLWidget(parent)
{
    setMinimumSize(100,100);
    setAutoBufferSwap(false);
    doneCurrent();
}

void RendererWidget::closeEvent(QCloseEvent*)
{
    closeContext();
}

void RendererWidget::closeContext()
{
    _renderer->stop();
    _renderer->lock();
    _renderer->stop();
    _renderer->unlock();

    _renderer=nullptr;
}

void RendererWidget::resizeEvent(QResizeEvent* ev)
{
    if(_renderer)
        _renderer->updateSize(uivec2(ev->size().width(), ev->size().height()));
}

void RendererWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(_leftMouse || _rightMouse)
    {
        event->accept();

        QPoint p = event->pos() - _lastMousePos;

        if(_editMode == MESH_EDITOR && _leftMouse)
        {
            emit pressedMouseMoved(p.x(), p.y());
        }
        else if(_editMode == MESH_EDITOR && _rightMouse)
        {
            float time = _renderer->elapsedTime();
            _inMeshEditorCameraAngle.x() -= p.x() * time * 20;
            _inMeshEditorCameraAngle.y() += p.y() * time * 20;

            vec3 dir = {cosf(_inMeshEditorCameraAngle[1]*PI/180)*cosf(_inMeshEditorCameraAngle[0]*PI/180),
                        cosf(_inMeshEditorCameraAngle[1]*PI/180)*sinf(_inMeshEditorCameraAngle[0]*PI/180),
                        sinf(_inMeshEditorCameraAngle[1]*PI/180)};

            _renderer->lock();
            _renderer->getSceneView(0).camera.pos = dir.resized(_renderer->getSceneView(0).camera.pos.length());
            _renderer->unlock();
        }
        else if(_editMode == SCENE_EDITOR && _rightMouse && _stateReady == NO_INTERACTION)
        {
            float time = _renderer->elapsedTime();
            _inSceneEditorCameraAngle.x() -= p.x() * time * 30;
            _inSceneEditorCameraAngle.y() -= p.y() * time * 30;
            _inSceneEditorCameraAngle.y() = std::max(-89.9f, std::min(89.9f, _inSceneEditorCameraAngle.y()));

            vec3 dir = {cosf(_inSceneEditorCameraAngle[1]*PI/180)*cosf(_inSceneEditorCameraAngle[0]*PI/180),
                        cosf(_inSceneEditorCameraAngle[1]*PI/180)*sinf(_inSceneEditorCameraAngle[0]*PI/180),
                        sinf(_inSceneEditorCameraAngle[1]*PI/180)};

            _renderer->lock();
            _renderer->getSceneView(1).camera.dir = _renderer->getSceneView(1).camera.pos + dir;
            _renderer->unlock();
        }
    }

    if(_editMode == SCENE_EDITOR && (_stateReady >= TRANSLATE_MODE &&  _stateReady <= ROTATE_Z_MODE))
    {
        event->accept();
        QPoint p = event->pos() - _lastMousePos;
        float relX = float(p.x()) / this->size().width();
        float relY = float(p.y()) / this->size().height();

        if(_shiftPressed)
        {
            relX *= 0.1;
            relY *= 0.1;
        }

        emit translateMouse(relX,relY,_stateReady);
    }

    _lastMousePos = event->pos();

    if(!hasFocus())
        setFocus();
}

void RendererWidget::wheelEvent(QWheelEvent* event)
{
    event->accept();
    if(_editMode == MESH_EDITOR)
    {
        _renderer->updateCamera_MeshEditor(event->delta());
    }
    else if(_editMode == SCENE_EDITOR)
    {
        _renderer->updateCamera_SceneEditor(event->delta());
    }
}

void RendererWidget::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_Shift)
        _shiftPressed=true;

    if(_editMode == SCENE_EDITOR && _rightMouse && _stateReady == NO_INTERACTION)
    {
        if(event->key() == Qt::Key_W) _renderer->setMoveDirection(0, true);
        else if(event->key() == Qt::Key_A) _renderer->setMoveDirection(1, true);
        else if(event->key() == Qt::Key_S) _renderer->setMoveDirection(2, true);
        else if(event->key() == Qt::Key_D) _renderer->setMoveDirection(3, true);
        else if(event->key() == Qt::Key_Shift) _renderer->setSpeedBoost(true);

//        else if(event->key() == Qt::Key_G)
//        {
//            _stateReady = TRANSLATE_MODE;
//            emit startEdit();
//            emit stateChanged(_stateReady);
//        }
//        else if(event->key() == Qt::Key_F)
//        {
//            _stateReady = SCALE_MODE;
//            emit startEdit();
//            emit stateChanged(_stateReady);
//        }
//        else if(event->key() == Qt::Key_R)
//        {
//            _stateReady = ROTATE_MODE;
//            emit startEdit();
//            emit stateChanged(_stateReady);
//        }

        event->accept();
    }
    else if(_editMode == SCENE_EDITOR && _stateReady == NO_INTERACTION)
    {
        if(event->key() == Qt::Key_G)
        {
            _stateReady = TRANSLATE_MODE;
            emit startEdit();
            emit stateChanged(_stateReady);
        }
        else if(event->key() == Qt::Key_F)
        {
            _stateReady = SCALE_MODE;
            emit startEdit();
            emit stateChanged(_stateReady);
        }
        else if(event->key() == Qt::Key_R)
        {
            _stateReady = ROTATE_MODE;
            emit startEdit();
            emit stateChanged(_stateReady);
        }
        else if(event->key() == Qt::Key_Shift) _renderer->setSpeedBoost(true);
        else if(event->key() == Qt::Key_Escape) emit escapePressed();
        else if(event->key() == Qt::Key_Delete) emit deleteCurrent();

        event->accept();
    }
    else if(_editMode == SCENE_EDITOR && _stateReady >= TRANSLATE_MODE && _stateReady <= TRANSLATE_Z_MODE)
    {
        if(event->key() == Qt::Key_X)
            _stateReady = TRANSLATE_X_MODE;
        else if(event->key() == Qt::Key_Y)
            _stateReady = TRANSLATE_Y_MODE;
        else if(event->key() == Qt::Key_Z)
            _stateReady = TRANSLATE_Z_MODE;

        emit stateChanged(_stateReady);

        event->accept();
    }
    else if(_editMode == SCENE_EDITOR && _stateReady >= SCALE_MODE && _stateReady <= SCALE_Z_MODE)
    {
        if(event->key() == Qt::Key_X)
            _stateReady = SCALE_X_MODE;
        else if(event->key() == Qt::Key_Y)
            _stateReady = SCALE_Y_MODE;
        else if(event->key() == Qt::Key_Z)
            _stateReady = SCALE_Z_MODE;

        emit stateChanged(_stateReady);

        event->accept();
    }
    else if(_editMode == SCENE_EDITOR && _stateReady >= ROTATE_MODE && _stateReady <= ROTATE_Z_MODE)
    {
        if(event->key() == Qt::Key_X)
            _stateReady = ROTATE_X_MODE;
        else if(event->key() == Qt::Key_Y)
            _stateReady = ROTATE_Y_MODE;
        else if(event->key() == Qt::Key_Z)
            _stateReady = ROTATE_Z_MODE;

        emit stateChanged(_stateReady);

        event->accept();
    }
}

void RendererWidget::keyReleaseEvent(QKeyEvent* event)
{
    bool accept = true;

    if(event->key() == Qt::Key_W) _renderer->setMoveDirection(0, false);
    else if(event->key() == Qt::Key_A) _renderer->setMoveDirection(1, false);
    else if(event->key() == Qt::Key_S) _renderer->setMoveDirection(2, false);
    else if(event->key() == Qt::Key_D) _renderer->setMoveDirection(3, false);
    else if(event->key() == Qt::Key_Shift)
    {
        _renderer->setSpeedBoost(false);
        _shiftPressed=false;
    }
    else accept = false;

    if(accept)
        event->accept();
}

void RendererWidget::mousePressEvent(QMouseEvent * event)
{
    if(event->button() == Qt::LeftButton && _stateReady == NO_INTERACTION)
    {
        _leftMouse = true;

        if(_renderer->getCurSceneIndex() > 0)
        {
            float x_rel = event->localPos().x() / this->size().width();
            float y_rel = event->localPos().y() / this->size().height();

            vec2 ss = vec2(x_rel, y_rel) * 2 - vec2(1,1);
            ss.y() *= -1;

            Camera camera = _renderer->getSceneView(_renderer->getCurSceneIndex()).camera;
            mat4 proj = mat4::Projection(camera.fov, camera.ratio, camera.clipDist.x(), camera.clipDist.y());
            mat4 view = mat4::View(camera.pos, camera.dir, camera.up);
            mat4 invProjView = (proj*view).inverted();

            vec4 v = invProjView*vec4(ss.x(), ss.y(), 0, 1);
            vec3 dir = v.to<3>() / v.w();
            dir = (dir-camera.pos).normalized();

            emit clickInEditor(camera.pos, dir);
        }

        event->accept();
    }
    else if(_stateReady >= TRANSLATE_MODE)
    {
        _stateReady = NO_INTERACTION;
        emit stateChanged(_stateReady);
        if(event->button() == Qt::RightButton)
            emit cancelEdit();
    }
    else if(event->button() == Qt::RightButton && _stateReady == NO_INTERACTION){
        _rightMouse = true;
        _lastMousePos = event->pos();
        event->accept();
        this->grabKeyboard();

        _renderer->setEnableDirection(true);
    }
}

void RendererWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        _leftMouse = false;
        event->accept();
    }
    else if(event->button() == Qt::RightButton){
        _rightMouse = false;
        event->accept();
        this->releaseKeyboard();

        _renderer->setEnableDirection(false);

        _renderer->setMoveDirection(0, false);
        _renderer->setMoveDirection(1, false);
        _renderer->setMoveDirection(2, false);
        _renderer->setMoveDirection(3, false);
        _renderer->setSpeedBoost(false);
    }
}

void RendererWidget::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    event->acceptProposedAction();
    event->accept();

   if(mimeData->hasUrls())
   {
       QList<QUrl> urlList = event->mimeData()->urls();
       for(QUrl& url : urlList)
       {
           emit addAssetToScene(url.url());
       }
   }
}

void RendererWidget::dragMoveEvent(QDragMoveEvent* event)
{
    event->accept();
}

void RendererWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if(_renderer->getCurSceneIndex() == 0)
        return;

    if(event->mimeData()->hasUrls())
    {
        QList<QUrl> urlList = event->mimeData()->urls();

        if(urlList.size() > 0)
        {
            emit dropEnterAsset(urlList[0].url(), event);
        }
    }
}
