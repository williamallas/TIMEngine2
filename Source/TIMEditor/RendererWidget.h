#ifndef RENDERERWIDGET_H
#define RENDERERWIDGET_H

#include <QGLWidget>
#include <QDebug>

class RendererWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit RendererWidget(QWidget *parent = 0);

protected:

    virtual void glInit() override {}
    virtual void glDraw() override {}

    virtual void initializeGL() override {}
    virtual void resizeGL(int, int) override {}
    virtual void paintGL() override {}
    virtual void resizeEvent(QResizeEvent *) override {}

    virtual void paintEvent(QPaintEvent *) override {}

signals:

public slots:

};

#endif // RENDERERWIDGET_H
