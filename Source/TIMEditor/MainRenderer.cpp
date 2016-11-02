#include "MainRenderer.h"
#include "Rand.h"
#include <QThread>
#include <QModelIndex>
#include <QElapsedTimer>
#include <QWaitCondition>
#include <QImage>

#include "MemoryLoggerOn.h"
namespace tim{

#undef interface
#undef DrawState
using namespace core;
using namespace renderer;
using namespace resource;
using namespace interface;

MainRenderer::MainRenderer(RendererWidget* parent) : _parent(parent)
{
    _renderingParameter.useShadow = true;
    _renderingParameter.shadowCascad = {4,15,50};
    _renderingParameter.shadowResolution = 2048;
    _renderingParameter.usePointLight = true;
    _renderingParameter.useSSReflexion = false;
    _currentSize = {200,200};
    _newSize = true;

    _running = true;

    _view[0].camera.ratio = 1;
    _view[0].camera.clipDist = {.01,100};
    _view[0].camera.pos = {0,-3,0};
    _view[0].camera.dir = {0,0,0};

    for(int i=0 ; i<NB_SCENE-1 ; ++i)
    {
        _view[i+1].camera.ratio = 1;
        _view[i+1].camera.clipDist = {.01,500};
        _view[i+1].camera.fov = 110;
    }
}

void MainRenderer::main()
{
    lock();

    ShaderPool::instance().add("gPass", "shader/gBufferPass.vert", "shader/gBufferPass.frag").value();
    ShaderPool::instance().add("gPassAlphaTest", "shader/gBufferPass.vert", "shader/gBufferPass.frag", "", {"ALPHA_TEST"}).value();
    ShaderPool::instance().add("water", "shader/gBufferPass.vert", "shader/gBufferPass.frag", "", {"WATER_SHADER"}).value();
    ShaderPool::instance().add("portalShader", "shader/gBufferPass.vert", "shader/gBufferPass.frag", "", {"PORTAL_SHADER"}).value();
    ShaderPool::instance().add("highlighted", "shader/overlayObject.vert", "shader/overlayObject.frag").value();
    ShaderPool::instance().add("highlightedMoving", "shader/overlayObject.vert", "shader/overlayObject2.frag").value();
    ShaderPool::instance().add("fxaa", "shader/fxaa.vert", "shader/fxaa.frag").value();
    ShaderPool::instance().add("combineScene", "shader/combineScene.vert", "shader/combineScene.frag").value();
    ShaderPool::instance().add("processSpecularCubeMap", "shader/processCubemap.vert", "shader/processCubemap.frag").value();

    {
    const float lineLength = 1000;
    VNCT_Vertex vDataX[3] = {{vec3(-lineLength,0,0),vec3(),vec2(),vec3()},
                             {vec3(0    ,0,0),vec3(),vec2(),vec3()},
                             {vec3(lineLength ,0,0),vec3(),vec2(),vec3()}};

    VNCT_Vertex vDataY[3] = {{vec3(0,-lineLength,0),vec3(),vec2(),vec3()},
                             {vec3(0,0    ,0),vec3(),vec2(),vec3()},
                             {vec3(0,lineLength ,0),vec3(),vec2(),vec3()}};

    VNCT_Vertex vDataZ[3] = {{vec3(0,0,-lineLength),vec3(),vec2(),vec3()},
                             {vec3(0,0,0    ),vec3(),vec2(),vec3()},
                             {vec3(0,0,lineLength ),vec3(),vec2(),vec3()}};

    uint iData[6] = {0,1,2};

    VBuffer* tmpVB = renderer::vertexBufferPool->alloc(3);
    IBuffer* tmpIB = renderer::indexBufferPool->alloc(3);
    tmpVB->flush(reinterpret_cast<float*>(vDataX),0,3);
    tmpIB->flush(iData,0,3);

    _lineMesh[0] = Mesh(Mesh::Element(Geometry(new MeshBuffers(tmpVB,tmpIB, Sphere(vec3(), lineLength)))));

    tmpVB = renderer::vertexBufferPool->alloc(3);
    tmpIB = renderer::indexBufferPool->alloc(3);
    tmpVB->flush(reinterpret_cast<float*>(vDataY),0,3);
    tmpIB->flush(iData,0,3);

    _lineMesh[1] = Mesh(Mesh::Element(Geometry(new MeshBuffers(tmpVB,tmpIB,Sphere(vec3(), lineLength)))));

    tmpVB = renderer::vertexBufferPool->alloc(3);
    tmpIB = renderer::indexBufferPool->alloc(3);
    tmpVB->flush(reinterpret_cast<float*>(vDataZ),0,3);
    tmpIB->flush(iData,0,3);

    _lineMesh[2] = Mesh(Mesh::Element(Geometry(new MeshBuffers(tmpVB,tmpIB,Sphere(vec3(), lineLength)))));

    for(int i=0 ; i<3 ; ++i)
    {
        vec4 col; col[i] = 1;
        _lineMesh[i].element(0).setColor(col);
        _lineMesh[i].element(0).setSpecular(0);
        _lineMesh[i].element(0).setRoughness(1);
        _lineMesh[i].element(0).setEmissive(0.2);
        _lineMesh[i].element(0).drawState().setShader(ShaderPool::instance().get("gPass"));
        _lineMesh[i].element(0).drawState().setPrimitive(DrawState::LINE_STRIP);
    }
    }

    //_scenePortalsManager = new MultipleSceneHelper(_renderingParameter, _pipeline);

//    for(int i=0 ; i<NB_SCENE-1 ; ++i)
//    {
//        _scenePortalsManager->registerDirLightView(&_scene[i+1], &_dirLightView[i+1]);
//    }

    resize();

    QList<QString> skybox;
    skybox += "skybox/simple4/x.png";
    skybox += "skybox/simple4/nx.png";
    skybox += "skybox/simple4/y.png";
    skybox += "skybox/simple4/ny.png";
    skybox += "skybox/simple4/z.png";
    skybox += "skybox/simple4/nz.png";

    setSkybox(0, skybox);
    //setSkybox(1, skybox);

    unlock();
    float totalTime=0;
    while(_running)
    {
        lock();

        QElapsedTimer timer;
        timer.start();

        _eventMutex.lock();
        while(!_events.empty())
            _events.dequeue()();

        _waitNoEvent.wakeAll();
        _eventMutex.unlock();

        updateCamera_SceneEditor();

        if(_pipeline.pipeline())
        {
//            _scenePortalsManager->setCurScene(_scene[_curScene]);
//            _scenePortalsManager->setView(_view[_curScene]);

//            interface::Scene* sceneCrossed = nullptr;
//            if(_scenePortalsManager->update(sceneCrossed, ))
//            {
//                for(int i=0 ; i<NB_SCENE ; ++i)
//                {
//                    if(&_scene[i] == sceneCrossed)
//                    {
//                        _view[i] = _view[_curScene];
//                        _curScene = i;
//                        break;
//                    }
//                }
//            }

            setupScene(_curScene, _view[_curScene]);

            _pipeline.pipeline()->prepare();
            _pipeline.pipeline()->render();
        }

        resize();

        GL_ASSERT();
        _parent->swapBuffers();

        _time = float(std::max(timer.elapsed(), qint64(1))) / 1000;
        totalTime += _time;
        _pipeline.pipeline()->meshRenderer().frameState().setTime(totalTime, _time);
        unlock();
    }
}

void MainRenderer::setupScene(int index, tim::interface::View& v)
{
    _pipeline.setScene(_scene[index], v, 0);

    for(int i=0 ; i<NB_SCENE ; ++i)
    {
        if(_scene[i].globalLight.dirLights.size() > 0 && _scene[i].globalLight.dirLights[0].projectShadow)
        {
            _dirLightView[i].dirLightView.camPos = v.camera.pos;
            _dirLightView[i].dirLightView.lightDir = _scene[index].globalLight.dirLights[0].direction;

            if(i == index)
                _pipeline.setDirLightView(_dirLightView[index], 0);
        }
    }
}

void MainRenderer::waitNoEvent()
{
    _eventMutex.lock();
    _waitNoEvent.wait(&_eventMutex);
    _eventMutex.unlock();
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
        openGL.applyAll();
        _pipeline.create(_currentSize, _renderingParameter);

//        _scenePortalsManager->setResolution(_currentSize);
//        _scenePortalsManager->rebuild(_scene[_curScene]);
        //_pipeline.setScene(_scene[_curScene], _view[_curScene], 0);

        //_pipeline.rendererEntity->envLightRenderer().setSkybox(_scene[0].globalLight.skybox.first, _scene[0].globalLight.skybox.second);
    }
    _newSize=false;
}

void MainRenderer::updateCamera_SceneEditor()
{
    if(_curScene == 0 || !_enableMove) return;

    vec3 newPos    = _view[_curScene].camera.pos,
         newRelDir = _view[_curScene].camera.dir - _view[_curScene].camera.pos;

    vec3 forward = newRelDir.normalized() * _time * (_speedBoost ? 10:1);
    vec3 side    = newRelDir.cross(-_view[_curScene].camera.up).normalized() * _time * (_speedBoost ? 10:1);

    if(_moveDirection[0])
        newPos += forward;
    else if(_moveDirection[2])
        newPos -= forward;

    if(_moveDirection[1])
        newPos += side;
    else if(_moveDirection[3])
        newPos -= side;

    _view[_curScene].camera.pos = newPos;
    _view[_curScene].camera.dir = _view[_curScene].camera.pos + newRelDir;
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
        float dist = _view[0].camera.pos.length();
        dist += 0.05 * float(wheel) *
                (std::max(_time, 0.001f) * fabs(dist));
        dist = dist < 0.1 ? 0.1 : dist;
        dist = dist > 100 ? 100 : dist;

        _view[0].camera.pos.resize(dist);
    }
    unlock();
}

void MainRenderer::updateCamera_SceneEditor(int wheel)
{
    lock();
    if(_curScene > 0)
    {
        float dist = 0.1 * float(wheel) * std::max(_time, 0.001f);
        if(_speedBoost)
            dist *= 0.1;

        _view[_curScene].camera.pos.z() += dist;
        _view[_curScene].camera.dir.z() += dist;
    }
    unlock();
}

void MainRenderer::setSkybox(int sceneIndex, QList<QString> list)
{
    if(list.isEmpty())
    {
        getScene(sceneIndex).globalLight.skybox = {nullptr, nullptr};
        return;
    }

    vector<std::string> files(list.size());
    for(int i=0 ; i<list.size() ; ++i)
        files[i] = list[i].toStdString();

    getScene(sceneIndex).globalLight.skybox = renderer::IndirectLightRenderer::loadAndProcessSkybox(files, ShaderPool::instance().get("processSpecularCubeMap"));
}

void MainRenderer::setDirectionalLight(uint sceneIndex, const tim::interface::Pipeline::DirectionalLight& l)
{
    if(getScene(sceneIndex).globalLight.dirLights.size() == 0)
        getScene(sceneIndex).globalLight.dirLights.push_back(l);
    else
    {
        getScene(sceneIndex).globalLight.dirLights[0] = l;
    }
    if(l.projectShadow)
        _dirLightView[sceneIndex].dirLightView.lightDir = l.direction;
}

renderer::Texture* MainRenderer::renderCubemap(vec3 pos, uint resolution, uint sceneId, int mode, float farDist)
{
    openGL.resetStates();
    openGL.applyAll();
    auto& node = _pipeline.create<interface::pipeline::InBufferRenderer>({resolution,resolution}, _renderingParameter);

    tim::interface::View v;
    v.camera.pos = pos;
    v.camera.clipDist = {0.1, farDist};
    v.camera.fov = 90;
    v.camera.ratio = 1;
    v.camera.useRawMat = false;

    setupScene(sceneId, v);

    renderer::Texture::GenTexParam param;
    param.nbLevels = 1;
    param.size = uivec3(resolution, resolution, 1);
    renderer::Texture* tex = renderer::Texture::genTextureCube(param);

    v.camera.dir = v.camera.pos + vec3(1,0,0);
    v.camera.up = mode==0 ? vec3(0,0,-1) : vec3(0,-1,0);
    node.fbo().attachTexture(0, tex, 0, 0);
    _pipeline.pipeline()->prepare();
    _pipeline.pipeline()->render();
    openGL.resetStates();

    v.camera.dir = v.camera.pos + vec3(-1,0,0);
    v.camera.up = mode==0 ? vec3(0,0,-1) : vec3(0,-1,0);
    node.fbo().attachTexture(0, tex, 0, 1);
    _pipeline.pipeline()->prepare();
    _pipeline.pipeline()->render();
    openGL.resetStates();

    v.camera.dir = v.camera.pos + vec3(0,1,0);
    v.camera.up = mode==0 ? vec3(0,0,-1) : vec3(0,0,1);
    node.fbo().attachTexture(0, tex, 0, 2);
    _pipeline.pipeline()->prepare();
    _pipeline.pipeline()->render();
    openGL.resetStates();

    v.camera.dir = v.camera.pos + vec3(0,-1,0);
    v.camera.up = mode==0 ? vec3(0,0,-1) : vec3(0,0,-1);
    node.fbo().attachTexture(0, tex, 0, 3);
    _pipeline.pipeline()->prepare();
    _pipeline.pipeline()->render();
    openGL.resetStates();

    v.camera.dir = v.camera.pos + vec3(0,0,1);
    v.camera.up = vec3(0,-1,0);
    node.fbo().attachTexture(0, tex, 0, 4);
    _pipeline.pipeline()->prepare();
    _pipeline.pipeline()->render();
    openGL.resetStates();

    v.camera.dir = v.camera.pos + vec3(0,0,-1);
    v.camera.up = vec3(0,-1,0);
    node.fbo().attachTexture(0, tex, 0, 5);
    _pipeline.pipeline()->prepare();
    _pipeline.pipeline()->render();
    openGL.resetStates();

    _newSize = true;
    resize();

    return tex;
}

void MainRenderer::exportSkybox(renderer::Texture* tex, std::string filepath)
{
    ubyte* dat = new ubyte[4 * tex->resolution().x() * tex->resolution().y()];
    tex->bind(0);
    {
        glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, GL_UNSIGNED_BYTE, dat);
        QImage img(dat, tex->resolution().x(), tex->resolution().y(), QImage::Format_RGBA8888);
        img.save(QString::fromStdString(filepath) + "x.png");
    }
    {
        glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, GL_UNSIGNED_BYTE, dat);
        QImage img(dat, tex->resolution().x(), tex->resolution().y(), QImage::Format_RGBA8888);
        img.save(QString::fromStdString(filepath) + "nx.png");
    }
    {
        glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, dat);
        QImage img(dat, tex->resolution().x(), tex->resolution().y(), QImage::Format_RGBA8888);
        img.save(QString::fromStdString(filepath) + "y.png");
    }
    {
        glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, dat);
        QImage img(dat, tex->resolution().x(), tex->resolution().y(), QImage::Format_RGBA8888);
        img.save(QString::fromStdString(filepath) + "ny.png");
    }
    {
        glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, GL_UNSIGNED_BYTE, dat);
        QImage img(dat, tex->resolution().x(), tex->resolution().y(), QImage::Format_RGBA8888);
        img.save(QString::fromStdString(filepath) + "z.png");
    }
    {
        glGetTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, GL_UNSIGNED_BYTE, dat);
        QImage img(dat, tex->resolution().x(), tex->resolution().y(), QImage::Format_RGBA8888);
        img.save(QString::fromStdString(filepath) + "nz.png");
    }

    delete[] dat;
}

}

