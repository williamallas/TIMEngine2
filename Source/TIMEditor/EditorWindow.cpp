#include "EditorWindow.h"
#include "ui_EditorWindow.h"
#include "QGLWidget"
#include <QDateTime>
#include <QFile>
#include <QTextStream>

#include <QMessageBox>
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

    this->setWindowTitle("TIMEditor - Scene 1");

    ui->resourceWidget->viewport()->setAcceptDrops(true);
    ui->sceneEditorWidget->setLocalCB(ui->localRot, ui->localTrans);

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

    ui->statusBar->showMessage("Welcome !", 10000);

    connect(ui->glWidget, SIGNAL(F11_pressed()), ui->viewDockWidget, SLOT(switchFullScreen()));
    connect(ui->glWidget, SIGNAL(pressedMouseMoved(int,int)), ui->meshEditorWidget, SLOT(rotateEditedMesh(int,int)));
    connect(ui->glWidget, SIGNAL(clickInEditor(vec3,vec3,bool)), ui->sceneEditorWidget, SLOT(selectSceneObject(vec3,vec3,bool)));
    connect(ui->glWidget, SIGNAL(translateMouse(float,float,int)), ui->sceneEditorWidget, SLOT(translateMouse(float,float,int)));
    connect(ui->glWidget, SIGNAL(escapePressed()), ui->sceneEditorWidget, SLOT(cancelSelection()));
    connect(ui->glWidget, SIGNAL(deleteCurrent()), ui->sceneEditorWidget, SLOT(deleteCurrentObjects()));

    connect(ui->meshEditorWidget, SIGNAL(saveMeshClicked()), this, SLOT(addMeshToAsset()));
    connect(ui->meshEditorWidget, SIGNAL(changeCurBaseModelName(QString)), ui->sceneEditorWidget, SLOT(changeBaseModelName(QString)));

    connect(ui->glWidget, SIGNAL(addAssetToScene(QString)), this, SLOT(addAssetToScene(QString)));
    connect(ui->glWidget, SIGNAL(addGeometryToScene(QString,QString)), this, SLOT(addGeometryToScene(QString,QString)));

    connect(ui->glWidget, SIGNAL(startEdit()), ui->sceneEditorWidget, SLOT(saveCurMeshTrans()));
    connect(ui->glWidget, SIGNAL(cancelEdit()), ui->sceneEditorWidget, SLOT(restoreCurMeshTrans()));
    connect(ui->glWidget, SIGNAL(stateChanged(int)), ui->sceneEditorWidget, SLOT(flushUiAccordingState(int)));
    connect(ui->sceneEditorWidget, SIGNAL(feedbackTransformation(QString)), this, SLOT(flushFeedbackTrans(QString)));

    _copySC = new QShortcut(QKeySequence("Ctrl+C"), ui->glWidget);
    connect(_copySC, SIGNAL(activated()), ui->sceneEditorWidget, SLOT(copyObject()));

    connect(ui->sceneEditorWidget, SIGNAL(editTransformation(int)), ui->glWidget, SLOT(enableTransformationMode(int)));

    loadParameter("editorParameter.txt");
}

EditorWindow::~EditorWindow()
{
    delete ui;
    delete _copySC;
}

void EditorWindow::loadParameter(QString filename)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
        return;

    QTextStream in(&file);
    QString line;

    while(in.readLineInto(&line))
    {
        auto vec = StringUtils(line.toStdString()).splitWord('=');
        if(vec.size() == 2)
        {
            if(vec[0] == "mouseSensitivity")
            {
                float sens = StringUtils(vec[1]).toFloat();
                if(sens <= 0)
                    sens = 1;

                ui->glWidget->setMouseSensitivity(sens);
            }
        }
    }
}

QString EditorWindow::genTitle() const
{
    QString title = "TIMEditor - Scene " + QString::number(ui->sceneEditorWidget->activeScene()+1);
    if(!_savePath[ui->sceneEditorWidget->activeScene()].isEmpty())
        title += " - " + _savePath[ui->sceneEditorWidget->activeScene()];

    return title;
}

/** SLOTS **/

void EditorWindow::EditorWindow::flushFeedbackTrans(QString str)
{
    ui->feedbackTrans->setText(str);
}

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

void EditorWindow::on_actionSunDirection_triggered()
{
    int sceneIndex = _rendererThread->mainRenderer()->getCurSceneIndex();
    if(!_rendererThread->mainRenderer()->getScene(sceneIndex).globalLight.dirLights.empty())
    {
        interface::Pipeline::DirectionalLight l = _rendererThread->mainRenderer()->getScene(sceneIndex).globalLight.dirLights[0];
        l.direction = _rendererThread->mainRenderer()->getSceneView(sceneIndex).camera.dir - _rendererThread->mainRenderer()->getSceneView(sceneIndex).camera.pos;
        _rendererThread->mainRenderer()->setDirectionalLight(sceneIndex, l);

        if(sceneIndex > 0)
            ui->sceneEditorWidget->setSunDirection(sceneIndex-1, l.direction);
    }

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

    if(_rendererThread->mainRenderer()->getCurSceneIndex() > 0)
        ui->sceneEditorWidget->setSkybox(_rendererThread->mainRenderer()->getCurSceneIndex()-1, list);
}

interface::XmlMeshAssetLoader::MeshElementModel convertEditorModel(MeshElement model)
{
    interface::XmlMeshAssetLoader::MeshElementModel out;
    out.color = vec3(model.color.red(), model.color.green(), model.color.blue()) / 255.f;
    out.geometry = model.geometry.toStdString();
    out.material = model.material;
    out.textureScale = model.textureScale;

    out.advanced = model.advanced;
    out.advancedShader = model.advancedShader.toStdString();
    out.useAdvanced = model.useAdvanced;
    out.castShadow = model.castShadow;

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

        elem.materials = ui->meshEditorWidget->convertFromEngine(p.second);
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
    ui->sceneEditorWidget->switchScene(0);

    setWindowTitle(genTitle());
}

void EditorWindow::on_actionScene_2_triggered()
{
    _rendererThread->mainRenderer()->lock();
    _rendererThread->mainRenderer()->setCurSceneIndex(2);
    ui->glWidget->setEditMode(RendererWidget::SCENE_EDITOR);
    _rendererThread->mainRenderer()->unlock();

    ui->meshEditorWidget->setEditedMesh(nullptr, nullptr, nullptr, "");
    ui->sceneEditorWidget->switchScene(1);

    setWindowTitle(genTitle());
}

void EditorWindow::on_actionScene_3_triggered()
{
    _rendererThread->mainRenderer()->lock();
    _rendererThread->mainRenderer()->setCurSceneIndex(3);
    ui->glWidget->setEditMode(RendererWidget::SCENE_EDITOR);
    _rendererThread->mainRenderer()->unlock();

    ui->meshEditorWidget->setEditedMesh(nullptr, nullptr, nullptr, "");
    ui->sceneEditorWidget->switchScene(2);

    setWindowTitle(genTitle());
}

void EditorWindow::on_actionScene_4_triggered()
{
    _rendererThread->mainRenderer()->lock();
    _rendererThread->mainRenderer()->setCurSceneIndex(4);
    ui->glWidget->setEditMode(RendererWidget::SCENE_EDITOR);
    _rendererThread->mainRenderer()->unlock();

    ui->meshEditorWidget->setEditedMesh(nullptr, nullptr, nullptr, "");
    ui->sceneEditorWidget->switchScene(3);

    setWindowTitle(genTitle());
}


void EditorWindow::on_actionLoad_collada_triggered()
{
    QString file = QFileDialog::getOpenFileName(this, "Import mesh assets", ".", "Collada files (*.dae)");
    if(file.isEmpty())
        return;

    AssimpLoader loader;
    loader.load(file.toStdString());

    vector<AssimpLoader::Node> nodes = loader.nodes();

    if(_rendererThread->mainRenderer()->getCurSceneIndex() == 0)
    {
        switch(ui->sceneEditorWidget->activeScene())
        {
            case 0: on_actionScene_1_triggered(); break;
            case 1: on_actionScene_2_triggered(); break;
            case 2: on_actionScene_3_triggered(); break;
            case 3: on_actionScene_4_triggered(); break;
        }
    }

    for(const AssimpLoader::Node& elem : nodes)
    {
        AssetViewWidget::Element asset;
        if(ui->assetViewWidget->getElement(QString::fromStdString(elem.idName), &asset))
        {
            ui->sceneEditorWidget->addSceneObject(QString::fromStdString(elem.name), QString::fromStdString(elem.idName), asset.materials, elem.matrix);
        }
    }

    ui->statusBar->showMessage("Collada scene loaded", 1000 * 60 * 10);
}

void EditorWindow::on_actionSave_triggered()
{
    if(_savePath[ui->sceneEditorWidget->activeScene()].isEmpty())
        on_actionSave_As_triggered();
    else
        ui->sceneEditorWidget->exportScene(_savePath[ui->sceneEditorWidget->activeScene()],
                                           ui->sceneEditorWidget->activeScene());

    ui->statusBar->showMessage(QString("Scene saved : ") + QDateTime::currentDateTime().toString("hh:mm:ss"), 1000 * 60 * 10);
    setWindowTitle(genTitle());
}

void EditorWindow::on_actionLoad_triggered()
{
    QString file = QFileDialog::getOpenFileName(this, "Load scene", ".", "XML files (*.xml)");
    if(file.isEmpty())
        return;

    if(_rendererThread->mainRenderer()->getCurSceneIndex() == 0)
    {
        switch(ui->sceneEditorWidget->activeScene())
        {
            case 0: on_actionScene_1_triggered(); break;
            case 1: on_actionScene_2_triggered(); break;
            case 2: on_actionScene_3_triggered(); break;
            case 3: on_actionScene_4_triggered(); break;
        }
    }

    ui->sceneEditorWidget->importScene(file, ui->sceneEditorWidget->activeScene());
    _savePath[ui->sceneEditorWidget->activeScene()] = file;

    ui->statusBar->showMessage("Scene loaded", 1000 * 60 * 10);
    setWindowTitle(genTitle());
}

void EditorWindow::on_actionSave_As_triggered()
{
    QString file = QFileDialog::getSaveFileName(this, "Save scene", ".", "XML Files (*.xml)");
    if(file.isEmpty())
        return;

    ui->sceneEditorWidget->exportScene(file, ui->sceneEditorWidget->activeScene());
    _savePath[ui->sceneEditorWidget->activeScene()] = file;

    ui->statusBar->showMessage(QString("Scene saved at ") + QDateTime::currentDateTime().toString("hh:mm:ss"), 1000 * 60 * 10);
    setWindowTitle(genTitle());
}

void EditorWindow::on_actionNew_triggered()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "New scene", "Are you sure to clear the scene ?",
                                                              QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes)
        ui->sceneEditorWidget->clearScene(ui->sceneEditorWidget->activeScene());

    ui->statusBar->showMessage("New scene", 10000);

    _savePath[ui->sceneEditorWidget->activeScene()] = "";
    setWindowTitle(genTitle());
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

void EditorWindow::addGeometryToScene(QString geomPath, QString name)
{
    AssetViewWidget::Element asset;
    MeshElement elem;
    elem.color = QColor(255,255,255);
    elem.material = {0.5,0,0.15,0};
    elem.geometry = geomPath;
    asset.materials += elem;

    vec3 camPos = _rendererThread->mainRenderer()->getSceneView(_rendererThread->mainRenderer()->getCurSceneIndex()).camera.pos;
    vec3 camDir = _rendererThread->mainRenderer()->getSceneView(_rendererThread->mainRenderer()->getCurSceneIndex()).camera.dir;
    ui->sceneEditorWidget->addSceneObject("", name, asset.materials, mat3::IDENTITY(), camPos + (camDir-camPos).resize(2), vec3(1,1,1));

    ui->sceneEditorWidget->activateLastAdded();
}

void EditorWindow::on_actionSkybox_triggered()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Save scene", ".");
    if(dir.isEmpty())
        return;

    vec3 camPos = _rendererThread->mainRenderer()->getSceneView(_rendererThread->mainRenderer()->getCurSceneIndex()).camera.pos;
    MainRenderer* mr = _rendererThread->mainRenderer();
    mr->addEvent( [=](){
        mr->renderCubemapAndExportFaces(camPos, 1024, _rendererThread->mainRenderer()->getCurSceneIndex(), dir.toStdString() + "/");
    });
}
