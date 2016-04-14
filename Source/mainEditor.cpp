#include "TIMEditor/EditorWindow.h"
#include <QApplication>
#include "core/core.h"
#include "renderer/renderer.h"
#undef main

int main(int argc, char *argv[])
{
    tim::core::init();
    tim::renderer::init();

    QApplication a(argc, argv);
    EditorWindow w;
    w.show();

//    tim::renderer::close();
//    tim::core::quit();

    return a.exec();
}
