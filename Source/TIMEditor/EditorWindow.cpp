#include "EditorWindow.h"
#include "ui_EditorWindow.h"
#include "QGLWidget"

EditorWindow::EditorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditorWindow)
{
    ui->setupUi(this);
    _rendererThread = new RendererThread(ui->widget);

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

//void EditorWindow::makeCurrent() {
//    ui->widget->makeCurrent();
//}

