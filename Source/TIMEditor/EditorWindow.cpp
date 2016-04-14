#include "EditorWindow.h"
#include "ui_EditorWindow.h"
#include "QGLWidget"

EditorWindow::EditorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditorWindow)
{

    _rendererThread = new RendererThread(ui->widget);
    ui->setupUi(this);

    _rendererThread->start();

    while(!_rendererThread->isInitialized()) {
        qApp->processEvents();
    }

    //delete contextCreator;
}

EditorWindow::~EditorWindow()
{
    delete ui;
}

void EditorWindow::makeCurrent() {
    ui->widget->makeCurrent();
}

