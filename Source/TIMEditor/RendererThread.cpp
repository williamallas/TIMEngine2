#include "RendererThread.h"
#include "EditorWindow.h"
#include <QDebug>

class GLContextWidget : public QGLWidget
{
    public:
        GLContextWidget(const QGLFormat &format) : QGLWidget(format) {
            setAutoBufferSwap(false);
            //doneCurrent();
            //setVisible(false);
        }

    protected:
        virtual void glInit() override {}
        virtual void glDraw() override {}

        virtual void initializeGL() override {}
        virtual void resizeGL(int, int) override {}
        virtual void paintGL() override {}

        virtual void paintEvent(QPaintEvent *) override {}
        virtual void resizeEvent(QResizeEvent *) override {}
};

RendererThread::RendererThread(RendererWidget* renderer) : _renderer(renderer), _init(false) {
    _contextCreator = new GLContextWidget(QGLFormat());
    _contextCreator->doneCurrent();
    _contextCreator->moveToThread(this);
}

bool RendererThread::isInitialized() const {
    return _init;
}

void RendererThread::run() {
    QGLContext* glContext = new QGLContext(QGLFormat());
    if(_contextCreator) {
        glContext->create(_contextCreator->context());
    }
    _renderer->setContext(glContext);
    qDebug() << "hi";
    //_renderer->makeCurrent();
    _init = true;

    while (true) {

    }

    //_editor->makeCurrent();
/*
    tim::core::init();
    qDebug() << "hi";
    tim::renderer::init();
    qDebug() << "hi";

    while (true) {
        qDebug() << "hi";
    }

    tim::renderer::close();
    tim::core::quit();*/
}
