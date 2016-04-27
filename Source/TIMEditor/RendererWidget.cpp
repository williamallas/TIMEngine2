#include "MainRenderer.h"
#include "RendererWidget.h"

RendererWidget::RendererWidget(QWidget *parent) :
    QGLWidget(parent)
{
    setMinimumSize(100,100);
    setAutoBufferSwap(false);
    doneCurrent();
}

void RendererWidget::closeEvent(QCloseEvent*)
{
    closeContext();
}

void RendererWidget::closeContext()
{
    _renderer->stop();
    _renderer->lock();
    _renderer->stop();
    _renderer->unlock();

    _renderer=nullptr;
}

void RendererWidget::resizeEvent(QResizeEvent* ev)
{
    if(_renderer)
        _renderer->updateSize(uivec2(ev->size().width(), ev->size().height()));
}

void RendererWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(_leftMouse)
    {
        event->accept();

        QPoint p = event->pos() - _lastMousePos;
        _lastMousePos = event->pos();

        _renderer->updateCamera_MeshEditor(p.x(), p.y(),0);
    }
}

void RendererWidget::wheelEvent(QWheelEvent* event)
{
    event->accept();
    _renderer->updateCamera_MeshEditor(0,0,event->delta());
}
