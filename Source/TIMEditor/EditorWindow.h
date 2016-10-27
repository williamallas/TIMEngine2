#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include <QShortcut>
#include "RendererThread.h"
#include "interface/XmlMeshAssetLoader.h"
#include "MeshElement.h"
#include "SceneEditorWidget.h"

namespace Ui {
class EditorWindow;
}

class EditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditorWindow(QWidget *parent = 0);
    ~EditorWindow();

protected:

private:
    Ui::EditorWindow *ui;
    RendererThread* _rendererThread;
    tim::interface::XmlMeshAssetLoader _assetLoader;

    QShortcut* _copySC;
    QString _savePath[SceneEditorWidget::NB_SCENE];

    QString genTitle() const;

    void loadParameter(QString);

public slots:
    void addResourceFolder();
    void addResourceFolderRec();
    void addMeshToAsset();
    void loadMeshAssets(QString);

private slots:
    void on_actionClose_Context_triggered();
    void on_actionAdd_folder_triggered();
    void on_actionAdd_folder_recursively_triggered();
    void on_actionSet_skybox_triggered();
    void on_actionMesh_assets_triggered();
    void on_actionMesh_assets_import_triggered();
    void on_actionLoad_collada_triggered();
    void on_action_selectAE_triggered();
    void on_actionScene_1_triggered();
    void on_actionScene_2_triggered();
    void on_actionScene_3_triggered();
    void on_actionScene_4_triggered();
    void on_actionSave_triggered();
    void on_actionLoad_triggered();
    void on_actionSave_As_triggered();
    void on_actionNew_triggered();
    void on_actionSunDirection_triggered();
    void on_actionSkybox_triggered();

    void addAssetToScene(QString);
    void addGeometryToScene(QString, QString);

    void flushFeedbackTrans(QString);

};

#endif // EDITORWINDOW_H
