#include <QApplication>
#include "TIMEditor/EditorWindow.h"
#undef main

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    EditorWindow w;
    w.show();

    return a.exec();
}
