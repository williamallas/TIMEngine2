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

    ui->resourceWidget->addDir(".");

    connect(ui->glWidget, SIGNAL(pressedMouseMoved(int,int)), ui->meshEditorWidget, SLOT(rotateEditedMesh(int,int)));
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
