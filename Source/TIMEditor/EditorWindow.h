#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include "RendererThread.h"

namespace Ui {
class EditorWindow;
}

class EditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditorWindow(QWidget *parent = 0);
    ~EditorWindow();

    //void makeCurrent();

private:
    Ui::EditorWindow *ui;
    RendererThread* _rendererThread;
};

#endif // EDITORWINDOW_H
