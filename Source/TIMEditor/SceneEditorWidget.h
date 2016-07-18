#ifndef SCENEEDITORWIDGET_H
#define SCENEEDITORWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTimer>
#include "MainRenderer.h"
#include "MeshElement.h"
#include "MeshEditorWidget.h"
#include "interface/XmlMeshAssetLoader.h"

class ResourceViewWidget;

using namespace tim;

namespace Ui {
class SceneEditor;
}

class SceneEditorWidget : public QWidget
{
    Q_OBJECT

public:
    SceneEditorWidget(QWidget* parent = nullptr);

    void setMainRenderer(tim::MainRenderer* r);
    void setMeshEditor(MeshEditorWidget* r) { _meshEditor = r; }

    void setSkybox(uint sceneIndex, const QList<QString>&);

    void addSceneObject(QString name, QString model, const QList<MeshElement>&, mat4);
    void addSceneObject(QString name, QString model, const QList<MeshElement>&, const mat3&, const vec3&, const vec3&);

    void switchScene(int);
    int activeScene() const { return _curSceneIndex; }

    void activateLastAdded();

    static const uint NB_SCENE = 4;

protected:
    Ui::SceneEditor *ui;
    tim::MainRenderer* _renderer;
    MeshEditorWidget* _meshEditor;

    struct SceneObject
    {
        QString name;
        QString baseModel;

        vec3 scale     = {1,1,1};
        mat3 rotate    = mat3::IDENTITY();
        vec3 translate = {0,0,0};

        interface::MeshInstance* node;
        QList<MeshElement> materials;
        QListWidgetItem* listItem;

        int exportHelper;
    };
    QList<SceneObject> _objects[NB_SCENE];
    int _curSceneIndex = 0;
    int _curItemIndex = -1;


    QList<tim::interface::Pipeline::DirectionalLight> _directionalLights[NB_SCENE];
    QList<QString> _skyboxs[NB_SCENE];

    vec3 saved_scale     = {1,1,1};
    mat3 saved_rotate    = mat3::IDENTITY();
    vec3 saved_translate = {0,0,0};

    tim::interface::MeshInstance* _translateLine[3] = {nullptr};
    tim::interface::MeshInstance* _highlightedMeshInstance = nullptr;

    void addSceneObject(bool lock, QString name, QString model, const QList<MeshElement>&, const mat3&, const vec3&, const vec3&);
    void addSceneObject(int scene, bool lock, QString name, QString model, const QList<MeshElement>&, const mat3&, const vec3&, const vec3&);
    void activateObject(int);
    void flushItemUi(int);
    void updateSelectedMeshMatrix();
    static mat4 constructTransformation(const SceneObject&);
    static void parseTransformation(TiXmlElement*, vec3& tr, vec3& sc, mat3&);
    static QList<QString> parseSkyboxXmlElement(TiXmlElement*);

    QTimer _flushState;

public slots:
    void sceneItemActivated(QListWidgetItem*);

    void on_translate_x_editingFinished();
    void on_translate_y_editingFinished();
    void on_translate_z_editingFinished();

    void on_scale_x_editingFinished();
    void on_scale_y_editingFinished();
    void on_scale_z_editingFinished();

    void on_name_editingFinished();
    void selectSceneObject(vec3,vec3);
    void translateMouse(float,float,int);
    void saveCurMeshTrans();
    void restoreCurMeshTrans();
    void flushUiAccordingState(int);
    void flushState();
    void cancelSelection();

    void resetRotation();
    void deleteCurrentObject();
    void copyObject();

    void edit_cameraPosX(double);
    void edit_cameraPosY(double);
    void edit_cameraPosZ(double);

    void exportScene(QString, int);
    void importScene(QString, int);
    void clearScene(int);

signals:
    void editTransformation(int);
};

#endif // MESHEDITORWIDGET_H
