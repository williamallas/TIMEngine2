#include "RendererWidget.h"

RendererWidget::RendererWidget(QWidget *parent) :
    QGLWidget(parent)
{
    setAutoBufferSwap(false);
    doneCurrent();
    setMinimumSize(200, 200);
}
