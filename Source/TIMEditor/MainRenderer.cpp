#include "MainRenderer.h"
#include "Rand.h"
#include <QThread>
#include <QModelIndex>
#include <QElapsedTimer>

#include "MemoryLoggerOn.h"
namespace tim{

#undef interface
using namespace core;
using namespace renderer;
using namespace resource;
using namespace interface;

MainRenderer::MainRenderer(RendererWidget* parent) : _parent(parent)
{
    _renderingParameter.useShadow = false;
    _renderingParameter.usePointLight = false;
    _currentSize = {200,200};
    _newSize = true;

    _running = true;

    _view[0].camera.ratio = 1;
    _view[0].camera.clipDist = {.1,100};
    _view[0].camera.pos = {0,-3,0};
    _view[0].camera.dir = {0,0,0};
}

void MainRenderer::main()
{
    lock();

    ShaderPool::instance().add("gPass", "shader/gBufferPass.vert", "shader/gBufferPass.frag").value();

    resize();

    QList<QString> skybox;
    skybox += "skybox/simple4/x.png";
    skybox += "skybox/simple4/nx.png";
    skybox += "skybox/simple4/y.png";
    skybox += "skybox/simple4/ny.png";
    skybox += "skybox/simple4/z.png";
    skybox += "skybox/simple4/nz.png";

    setSkybox(0, skybox);

    unlock();
    while(_running)
    {
        lock();

        QElapsedTimer timer;
        timer.start();

        _eventMutex.lock();
        while(!_events.empty())
            _events.dequeue()();
        _eventMutex.unlock();

        if(_pipeline.pipeline)
        {
            _pipeline.pipeline->prepare();
            _pipeline.pipeline->render();
        }

        resize();

        GL_ASSERT();
        _parent->swapBuffers();

        _time = float(std::max(timer.elapsed(), qint64(1))) / 1000;

        unlock();
    }
}

void MainRenderer::updateSize(uivec2 s)
{
    lock();
    if(_currentSize != s)
    {
        _newSize = true;
        _currentSize = s;
        for(int i=0 ; i<NB_SCENE ; ++i)
            _view[i].camera.ratio = float(s.x()) / s.y();
    }
    unlock();
}

void MainRenderer::resize()
{
    if(_newSize)
    {
        openGL.resetStates();
        _pipeline.create(_currentSize, _renderingParameter);
        _pipeline.rendererEntity->envLightRenderer().setEnableGI(true);
        _pipeline.setScene(_scene[0], _view[0]);
        _pipeline.rendererEntity->envLightRenderer().setSkybox(_scene[0].globalLight.skybox.first, _scene[0].globalLight.skybox.second);
    }
    _newSize=false;
}

void MainRenderer::addEvent(std::function<void()> f)
{
    _eventMutex.lock();
    _events.append(f);
    _eventMutex.unlock();
}

void MainRenderer::updateCamera_MeshEditor(int wheel)
{
    lock();
    if(_curScene == 0)
    {
        _view[0].camera.pos.y() -= 0.05 * float(wheel) *
                                (std::max(_time, 0.001f) * fabs(_view[0].camera.pos.y()));
        _view[0].camera.pos.y() = _view[0].camera.pos.y() > -1 ? -1 : _view[0].camera.pos.y();
        _view[0].camera.pos.y() = _view[0].camera.pos.y() < -100 ? -100 : _view[0].camera.pos.y();
    }
    unlock();
}

void MainRenderer::setSkybox(int sceneIndex, QList<QString> list)
{
    resource::TextureLoader::ImageFormat formatTex;
    vector<std::string> imgSkybox(6);
    for(size_t i=0 ; i<list.size() ; ++i)
        imgSkybox[i] = list[i].toStdString();

    vector<ubyte*> dataSkybox = resource::textureLoader->loadImageCube(imgSkybox, formatTex);

    renderer::Texture::GenTexParam skyboxParam;
    skyboxParam.format = renderer::Texture::RGBA8;
    skyboxParam.nbLevels = 1;
    skyboxParam.size = uivec3(formatTex.size,0);
    renderer::Texture* skybox = renderer::Texture::genTextureCube(skyboxParam, dataSkybox, 4);
    renderer::Texture* processedSky =
            renderer::IndirectLightRenderer::processSkybox(skybox,
            pipeline().rendererEntity->envLightRenderer().processSkyboxShader());

    for(uint j=0 ; j<dataSkybox.size() ; ++j)
        delete[] dataSkybox[j];

    getScene(sceneIndex).globalLight.skybox = {skybox, processedSky};
}

}

