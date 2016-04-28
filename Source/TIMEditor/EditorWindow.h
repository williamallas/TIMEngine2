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

protected:

private:
    Ui::EditorWindow *ui;
    RendererThread* _rendererThread;

public slots:
    void addResourceFolder();
    void addResourceFolderRec();
private slots:
    void on_actionClose_Context_triggered();
    void on_actionAdd_folder_triggered();
    void on_actionAdd_folder_recursively_triggered();
};

#endif // EDITORWINDOW_H
