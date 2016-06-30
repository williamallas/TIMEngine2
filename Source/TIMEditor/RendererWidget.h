#ifndef RENDERERWIDGET_H
#define RENDERERWIDGET_H

#include <QGLWidget>
#include <QMouseEvent>
#include <QMessageBox>
#include <QShortcut>

#include "core/Vector.h"

namespace tim{
class MainRenderer;
}

class RendererWidget : public QGLWidget
{
    Q_OBJECT
public:
    enum { MESH_EDITOR, SCENE_EDITOR };

    enum
    {
        NO_INTERACTION,

        TRANSLATE_MODE,
        TRANSLATE_X_MODE,
        TRANSLATE_Y_MODE,
        TRANSLATE_Z_MODE,

        SCALE_MODE,
        SCALE_X_MODE,
        SCALE_Y_MODE,
        SCALE_Z_MODE,

        ROTATE_MODE,
        ROTATE_X_MODE,
        ROTATE_Y_MODE,
        ROTATE_Z_MODE,
    };

    explicit RendererWidget(QWidget *parent = 0);

    void setMainRenderer(tim::MainRenderer* mr) { _renderer = mr; }
    void setEditMode(int mode) { _editMode = mode; }

protected:
    tim::MainRenderer* _renderer = nullptr;

    vec2 _inMeshEditorCameraAngle = {-90,0};

    vec2 _inSceneEditorCameraAngle = {-90, 0};

    virtual void glInit() override {}
    virtual void glDraw() override {}

    virtual void resizeGL(int, int) override {}
    virtual void resizeEvent(QResizeEvent *) override;

    virtual void initializeGL() override {}
    virtual void paintGL() override {}
    virtual void paintEvent(QPaintEvent *) override {}

    virtual void closeEvent(QCloseEvent *event) override;

    virtual void mousePressEvent(QMouseEvent * event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

    virtual void mouseMoveEvent(QMouseEvent*) override;
    virtual void wheelEvent(QWheelEvent*) override;

    virtual void keyPressEvent(QKeyEvent*) override;
    virtual void keyReleaseEvent(QKeyEvent*) override;

    void dropEvent(QDropEvent*) override;
    void dragMoveEvent(QDragMoveEvent*) override;
    void dragEnterEvent(QDragEnterEvent*) override;

    bool _leftMouse=false, _rightMouse=false;
    QPoint _lastMousePos;
    bool _shiftPressed=false;


    int _editMode = MESH_EDITOR;

    /* state machine */
    int _stateReady = 0;


signals:
    void pressedMouseMoved(int, int);
    void dropEnterAsset(QString, QDragEnterEvent*);
    void addAssetToScene(QString);
    void clickInEditor(vec3, vec3);
    void translateMouse(float,float,int);
    void startEdit();
    void cancelEdit();
    void stateChanged(int);
    void escapePressed();
    void deleteCurrent();

public slots:
    void closeContext();
};

#endif // RENDERERWIDGET_H
