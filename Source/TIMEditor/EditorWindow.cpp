#include "EditorWindow.h"
#include "ui_EditorWindow.h"
#include "QGLWidget"

#include <QFileDialog>
#include "SelectSkyboxDialog.h"
#include "AssimpLoader.h"

using namespace tim;

EditorWindow::EditorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditorWindow)
{
    ui->setupUi(this);
    this->setCentralWidget(nullptr);

    this->tabifyDockWidget(ui->assetDockWidget, ui->resourceDockWidget);
    this->tabifyDockWidget(ui->sceneDockWidget, ui->meshDockWidget);

    ui->resourceWidget->viewport()->setAcceptDrops(true);

    _rendererThread = new RendererThread(ui->glWidget);
    _rendererThread->start();

    while(!_rendererThread->isInitialized()) {
        qApp->processEvents();
    }

    ui->glWidget->setMainRenderer(_rendererThread->mainRenderer());
    ui->meshEditorWidget->setMainRenderer(_rendererThread->mainRenderer());
    ui->meshEditorWidget->setResourceWidget(ui->resourceWidget);

    ui->sceneEditorWidget->setMainRenderer(_rendererThread->mainRenderer());
    ui->sceneEditorWidget->setMeshEditor(ui->meshEditorWidget);

    ui->assetViewWidget->setMeshEditorWidget(ui->meshEditorWidget);

    ui->resourceWidget->addDir(".");

    connect(ui->glWidget, SIGNAL(pressedMouseMoved(int,int)), ui->meshEditorWidget, SLOT(rotateEditedMesh(int,int)));
    connect(ui->meshEditorWidget, SIGNAL(saveMeshClicked()), this, SLOT(addMeshToAsset()));
    connect(ui->glWidget, SIGNAL(dropEnterAsset(QString,QDragEnterEvent*)), ui->assetViewWidget, SLOT(confirmAssetDrop(QString,QDragEnterEvent*)));
    connect(ui->glWidget, SIGNAL(addAssetToScene(QString)), this, SLOT(addAssetToScene(QString)));
    connect(ui->glWidget, SIGNAL(clickInEditor(vec3,vec3)), ui->sceneEditorWidget, SLOT(selectSceneObject(vec3,vec3)));
    connect(ui->glWidget, SIGNAL(translateMouse(float,float,int)), ui->sceneEditorWidget, SLOT(translateMouse(float,float,int)));
    connect(ui->glWidget, SIGNAL(startEdit()), ui->sceneEditorWidget, SLOT(saveCurMeshTrans()));
    connect(ui->glWidget, SIGNAL(cancelEdit()), ui->sceneEditorWidget, SLOT(restoreCurMeshTrans()));
    connect(ui->glWidget, SIGNAL(stateChanged(int)), ui->sceneEditorWidget, SLOT(flushUiAccordingState(int)));
    connect(ui->glWidget, SIGNAL(escapePressed()), ui->sceneEditorWidget, SLOT(cancelSelection()));
    connect(ui->glWidget, SIGNAL(deleteCurrent()), ui->sceneEditorWidget, SLOT(deleteCurrentObject()));
}

EditorWindow::~EditorWindow()
{
    delete ui;
}

/** SLOTS **/

void EditorWindow::addResourceFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory",
                                                    ".",
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    ui->resourceWidget->addDir(dir);
}

void EditorWindow::addResourceFolderRec()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory",
                                                    ".",
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    ui->resourceWidget->addDir(dir, true);
}

void EditorWindow::on_actionClose_Context_triggered()
{
    ui->glWidget->closeContext();
}

void EditorWindow::on_actionAdd_folder_triggered()
{
    addResourceFolder();
}

void EditorWindow::on_actionAdd_folder_recursively_triggered()
{
    addResourceFolderRec();
}

void EditorWindow::on_actionSet_skybox_triggered()
{
    SelectSkyboxDialog dialog(this, ui->resourceWidget);
    dialog.exec();

    QList<QString> list = dialog.getSkydirPaths();

    if(list.empty()) return;

    _rendererThread->mainRenderer()->addEvent([=](){
        _rendererThread->mainRenderer()->setSkybox(_rendererThread->mainRenderer()->getCurSceneIndex(), list);
    });
}

interface::XmlMeshAssetLoader::MeshElementModel convertEditorModel(MeshElement model)
{
    interface::XmlMeshAssetLoader::MeshElementModel out;
    out.color = vec3(model.color.red(), model.color.green(), model.color.blue()) / 255.f;
    out.geometry = model.geometry.toStdString();
    out.material = model.material;

    for(int i=0 ; i<MeshElement::NB_TEXTURES ; ++i)
    {
        out.textures[i] = model.textures[i].toStdString();
    }
    out.type = 0;

    return out;
}

void EditorWindow::addMeshToAsset()
{
    if(ui->meshEditorWidget->currentMeshName().isEmpty())
    {
        QMessageBox::warning(ui->meshEditorWidget, "Mesh name is empty", "We can't save the mesh without a valid name.");
        return;
    }

    if(ui->meshEditorWidget->currentMesh().isEmpty())
    {
        QMessageBox::warning(ui->meshEditorWidget, "The mesh is empty", "The active mesh is empty.");
        return;
    }

    AssetViewWidget::Element elem;
    elem.materials = ui->meshEditorWidget->currentMesh();
    elem.name = ui->meshEditorWidget->currentMeshName();
    elem.type = AssetViewWidget::Element::MESH;

    vector<interface::XmlMeshAssetLoader::MeshElementModel> model;
    for(int i=0 ; i<elem.materials.size() ; ++i)
        model.push_back(convertEditorModel(elem.materials[i]));

    _assetLoader.addModel(elem.name.toStdString(), model);

    ui->assetViewWidget->addElement(elem);
}

void EditorWindow::loadMeshAssets(QString filename)
{
    interface::XmlMeshAssetLoader loader;
    if(!loader.load(filename.toStdString()))
        return;

    _assetLoader.load(filename.toStdString());

    for(auto p : loader.allAssets())
    {
        AssetViewWidget::Element elem;
        elem.name = p.first.c_str();
        elem.type = AssetViewWidget::Element::MESH;

        for(size_t i=0 ; i<p.second.size() ; ++i)
        {
            AssetViewWidget::Material mat;
            mat.color = QColor(255*p.second[i].color.x(), 255*p.second[i].color.y(), 255*p.second[i].color.z());
            mat.material = p.second[i].material;
            mat.geometry = p.second[i].geometry.c_str();

            for(int j=0 ; j<3 ; ++j)
            {
                mat.textures[j] = p.second[i].textures[j].c_str();
                QIcon ic = ui->resourceWidget->getResourceIconForPath(p.second[i].textures[j].c_str());
                if(!ic.isNull())
                    mat.texturesIcon[j] = ic;
                else
                    mat.texturesIcon[j] = QIcon(p.second[i].textures[j].c_str());

            }
            elem.materials.push_back(mat);
        }
        ui->assetViewWidget->addElement(elem);
    }
}

void EditorWindow::on_actionMesh_assets_triggered()
{
    QString file = QFileDialog::getSaveFileName(this, "Save mesh assets", ".");
    if(file.isEmpty())
        return;

    ui->assetViewWidget->exportMesh(file, "./");
}

void EditorWindow::on_actionMesh_assets_import_triggered()
{
    QString file = QFileDialog::getOpenFileName(this, "Import mesh assets", ".", "XML files (*.xml)");
    if(file.isEmpty())
        return;

    loadMeshAssets(file);
}

void EditorWindow::on_action_selectAE_triggered()
{
    _rendererThread->mainRenderer()->lock();
    _rendererThread->mainRenderer()->setCurSceneIndex(0);
    ui->glWidget->setEditMode(RendererWidget::MESH_EDITOR);
    _rendererThread->mainRenderer()->unlock();

    ui->meshEditorWidget->activeEditMode();
}

void EditorWindow::on_actionScene_1_triggered()
{
    _rendererThread->mainRenderer()->lock();
    _rendererThread->mainRenderer()->setCurSceneIndex(1);
    ui->glWidget->setEditMode(RendererWidget::SCENE_EDITOR);
    _rendererThread->mainRenderer()->unlock();

    ui->meshEditorWidget->setEditedMesh(nullptr, nullptr, nullptr, "");
}

void EditorWindow::on_actionLoad_collada_triggered()
{
    QString file = QFileDialog::getOpenFileName(this, "Import mesh assets", ".", "Collada files (*.dae)");
    if(file.isEmpty())
        return;

    AssimpLoader loader;
    loader.load(file.toStdString());

    vector<AssimpLoader::Node> nodes = loader.nodes();

    _rendererThread->mainRenderer()->lock();
    _rendererThread->mainRenderer()->setCurSceneIndex(1);
    ui->glWidget->setEditMode(RendererWidget::SCENE_EDITOR);
    _rendererThread->mainRenderer()->unlock();

    for(const AssimpLoader::Node& elem : nodes)
    {
        AssetViewWidget::Element asset;
        if(ui->assetViewWidget->getElement(QString::fromStdString(elem.idName), &asset))
        {
            ui->sceneEditorWidget->addSceneObject(QString::fromStdString(elem.name), QString::fromStdString(elem.idName), asset.materials, elem.matrix);
        }
    }
}

void EditorWindow::addAssetToScene(QString assetName)
{
    AssetViewWidget::Element asset;
    if(ui->assetViewWidget->getElement(assetName, &asset))
    {
        vec3 camPos = _rendererThread->mainRenderer()->getSceneView(_rendererThread->mainRenderer()->getCurSceneIndex()).camera.pos;
        vec3 camDir = _rendererThread->mainRenderer()->getSceneView(_rendererThread->mainRenderer()->getCurSceneIndex()).camera.dir;
        ui->sceneEditorWidget->addSceneObject("", assetName, asset.materials, mat3::IDENTITY(), camPos + (camDir-camPos).resize(2), vec3(1,1,1));

        ui->sceneEditorWidget->activateLastAdded();
    }
}
