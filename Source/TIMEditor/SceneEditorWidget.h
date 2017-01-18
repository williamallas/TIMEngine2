#ifndef SCENEEDITORWIDGET_H
#define SCENEEDITORWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTimer>
#include "MainRenderer.h"
#include "MeshElement.h"
#include "MeshEditorWidget.h"
#include "interface/XmlMeshAssetLoader.h"

#include "ObjectInstancingDialog.h"
#include "SimpleSpecProbeImportExport.h"

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
    void setLocalCB(QCheckBox* rot, QCheckBox* trans) { _localRotation = rot; _localTranslation = trans; }

    void setSkybox(uint sceneIndex, const QList<QString>&);
    void setSunDirection(int sceneIndex, vec3 dir);

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

    QCheckBox* _localRotation = nullptr;
    QCheckBox* _localTranslation = nullptr;

    struct Collider
    {
        enum { NONE, AUTO_BOX, AUTO_SPHERE, CONVEX_HULL, USER_DEFINED };
        int type = NONE;

        float mass=1,
              friction=0.75,
              rollingFriction=0.1,
              restitution=0.8;
        // todo add list of user defined shape (multishape component)
    };

    int sceneObject_id_generator = 0;
    struct SceneObject
    {
        int unique_id = 0;
        QString name;
        QString baseModel;
        QListWidgetItem* listItem;

        vec3 scale     = {1,1,1};
        mat3 rotate    = mat3::IDENTITY();
        vec3 translate = {0,0,0};

        /* collider */
        Collider collider;

        /* MESH object */
        interface::MeshInstance* node;
        QList<MeshElement> materials;
        bool isStatic = true, isPhysic = true, isVisible = true;

        int exportHelper;
    };
    QList<SceneObject> _objects[NB_SCENE];
    int _curSceneIndex = 0;

    QList<tim::interface::Pipeline::DirectionalLight> _directionalLights[NB_SCENE];
    QList<QString> _skyboxs[NB_SCENE];

    struct Selection
    {
        int index = -1;
        vec3 saved_scale     = {1,1,1};
        mat3 saved_rotate    = mat3::IDENTITY();
        vec3 saved_translate = {0,0,0};
        tim::interface::MeshInstance* highlightedMeshInstance = nullptr;
    };
    QList<Selection> _selections;
    float _accumulator = 0;

    tim::interface::MeshInstance* _translateLine[3] = {nullptr};

    vector<LightProbeUtils> _allSpecProbe[NB_SCENE];

    ObjectInstancingDialog _instancingDialog;
    QList<int> _lastAddedInstancing;
    int _indexSceneLastInstancing = 0;

    /* copy past trans */
    vec3 copy_scale     = {1,1,1};far
    mat3 copy_rotate    = mat3::IDENTITY();
    vec3 copy_translate = {0,0,0};
    bool somethingCopied = false;

    void addSceneObject(bool lock, QString name, QString model, const QList<MeshElement>&, const mat3&, const vec3&, const vec3&);
    void addSceneObject(int scene, bool lock, QString name, QString model, const QList<MeshElement>&, const mat3&, const vec3&, const vec3&);
    void addSceneObject(int scene, bool lock, SceneObject);

    void activateObject(int, bool, bool lock);
    void flushItemUi(int);
    void updateSelectedMeshMatrix();
    static void parseTransformation(TiXmlElement*, vec3& tr, vec3& sc, mat3&, Collider* collider);
    static QList<QString> parseSkyboxXmlElement(TiXmlElement*);
    bool hasCurrentSelection() const;
    bool isHighlightedInstance(interface::MeshInstance*) const;
    void applyRelRot(const mat3&, bool);

    QTimer _flushState;

    void remove_n_first_lightProb(size_t n);
    void removeSceneObject(const SceneObject&, int);

public slots:
    void sceneItemActivated(QListWidgetItem*);

    void on_translate_x_editingFinished();
    void on_translate_y_editingFinished();
    void on_translate_z_editingFinished();

    void on_scale_x_editingFinished();
    void on_scale_y_editingFinished();
    void on_scale_z_editingFinished();

    void on_copyTransButton_clicked();
    void on_pastTransButton_clicked();
    void on_instancing_clicked();

    void on_meshc_isStatic_clicked(bool);
    void on_meshc_isPhysic_clicked(bool);
    void on_meshc_isVisible_clicked(bool);

    void on_mc_mass_editingFinished();
    void on_mc_restitution_editingFinished();
    void on_mc_friction_editingFinished();
    void on_mc_rfriction_editingFinished();

    void on_name_editingFinished();
    void on_colliderList_currentIndexChanged(int);
    void selectSceneObject(vec3,vec3,bool);
    void translateMouse(float,float,int);
    void saveCurMeshTrans();
    void restoreCurMeshTrans();
    void flushUiAccordingState(int);
    void flushState();
    void cancelSelection();

    void resetRotation();
    void deleteCurrentObjects();
    void copyObject();
    void changeBaseModelName(QString);

    void edit_cameraPosX(double);
    void edit_cameraPosY(double);
    void edit_cameraPosZ(double);

    void exportScene(QString, int);
    void importScene(QString, int);
    void clearScene(int);

    void renderSpecularProbe();
    void removeAllLightProbe();
    void removeLastLightProbe();
    void regenAllLightProb();

signals:
    void editTransformation(int);
    void feedbackTransformation(QString);

private:
    void internalRenderLightProb(vec3 pos, float radius, float farDist, int iterations, int res, std::string pathRD, std::string pathSkybox, bool addToScene, bool exportAsRawData, bool exportAsSkybox);
};

inline bool SceneEditorWidget::hasCurrentSelection() const
{
    return !_selections.empty() && _selections[0].index >= 0;
}

#endif // MESHEDITORWIDGET_H
