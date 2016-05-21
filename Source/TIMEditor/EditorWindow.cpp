#include "EditorWindow.h"
#include "ui_EditorWindow.h"
#include "QGLWidget"

#include <QFileDialog>
#include "SelectSkyboxDialog.h"

using namespace tim;

EditorWindow::EditorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditorWindow)
{
    ui->setupUi(this);
    this->setCentralWidget(nullptr);

    ui->resourceWidget->viewport()->setAcceptDrops(true);

    _rendererThread = new RendererThread(ui->glWidget);
    _rendererThread->start();

    while(!_rendererThread->isInitialized()) {
        qApp->processEvents();
    }

    ui->glWidget->setMainRenderer(_rendererThread->mainRenderer());
    ui->meshEditorWidget->setMainRenderer(_rendererThread->mainRenderer());
    ui->meshEditorWidget->setResourceWidget(ui->resourceWidget);

    ui->assetViewWidget->setMeshEditorWidget(ui->meshEditorWidget);

    ui->resourceWidget->addDir(".");

    connect(ui->glWidget, SIGNAL(pressedMouseMoved(int,int)), ui->meshEditorWidget, SLOT(rotateEditedMesh(int,int)));
    connect(ui->meshEditorWidget, SIGNAL(saveMeshClicked()), this, SLOT(addMeshToAsset()));
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
        _rendererThread->mainRenderer()->setSkybox(0, list);
    });
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

    ui->assetViewWidget->addElement(elem);
}

void EditorWindow::on_actionMesh_assets_triggered()
{
    QString file = QFileDialog::getSaveFileName(this, "Save mesh asset", ".");
    if(file.isEmpty())
        return;

    ui->assetViewWidget->exportMesh(file, "./");
}
