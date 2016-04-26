#include "EditorWindow.h"
#include "ui_EditorWindow.h"
#include "QGLWidget"

#include <QFileDialog>

EditorWindow::EditorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditorWindow)
{
    //setAcceptDrops(true);
    ui->setupUi(this);

    this->setCentralWidget(nullptr);

    QMenu* editorMenu = ui->menubar->addMenu("Editor");
    QAction* actionCloseContext = new QAction("Close context", this);
    editorMenu->addAction(actionCloseContext);
    connect(actionCloseContext, SIGNAL(triggered()), ui->glWidget, SLOT(closeContext()));

    QMenu* resourceMenu = ui->menubar->addMenu("Resource");
    QAction* actionAddDir = new QAction("Add folder", this);
    resourceMenu->addAction(actionAddDir);
    connect(actionAddDir, SIGNAL(triggered()), this, SLOT(addResourceFolder()));
    QAction* actionAddDirRec = new QAction("Add folder recursively", this);
    resourceMenu->addAction(actionAddDirRec);
    connect(actionAddDirRec, SIGNAL(triggered()), this, SLOT(addResourceFolderRec()));

    _rendererThread = new RendererThread(ui->glWidget);
    _rendererThread->start();

    while(!_rendererThread->isInitialized()) {
        qApp->processEvents();
    }

    ui->glWidget->setMainRenderer(_rendererThread->mainRenderer());
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
