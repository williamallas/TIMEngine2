#include "TIMEditor/QtTextureLoader.h"
#include "RendererThread.h"

#undef interface
using namespace tim;
using namespace core;
using namespace interface;

class GLContextWidget : public QGLWidget
{
    public:
        GLContextWidget(const QGLFormat &format) : QGLWidget(format) {
            setAutoBufferSwap(false);
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

void RendererThread::run()
{
    initContext();
    SDL_Delay(100);
    _main->main();

    delete _main;
    resource::AssetManager<Geometry>::freeInstance();
    resource::AssetManager<Texture>::freeInstance();
    ShaderPool::freeInstance();
    renderer::openGL.execAllGLTask();

    delete resource::textureLoader;
    renderer::close();
    core::quit();
}

void RendererThread::initContext()
{
    QGLContext* glContext = new QGLContext(QGLFormat());

    if(_contextCreator) {
        glContext->create(_contextCreator->context());
    }
    _renderer->setContext(glContext);
    _renderer->makeCurrent();

    core::init();
    renderer::init();
    resource::textureLoader = new tim::QtTextureLoader;
    _main = new MainRenderer(_renderer);
    _init = true;
}
