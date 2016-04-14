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
    _contextCreator->context()->moveToThread(this);
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

    _renderer->makeCurrent();
    _init = true;

    tim::core::init();
    tim::renderer::init();

    while (true) {
        tim::renderer::openGL.clearColor(vec4(1,0,0,0));
        _renderer->swapBuffers();
    }

    tim::renderer::close();
    tim::core::quit();
}
