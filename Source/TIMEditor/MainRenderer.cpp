#include "MainRenderer.h"
#include "Rand.h"
#include <QThread>

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

    _running = true;
}

void MainRenderer::main()
{
    lock();

    Geometry geometry[3];
    geometry[0] = AssetManager<Geometry>::instance().load<false>("tor.obj").value();
    geometry[1] = AssetManager<Geometry>::instance().load<false>("sphere.tim").value();
    geometry[2] = AssetManager<Geometry>::instance().load<false>("cube_uv.obj").value();

    Shader* gPass = ShaderPool::instance().add("gPass", "shader/gBufferPass.vert", "shader/gBufferPass.frag").value();

    Mesh mesh[3];
    for(int i=0 ; i<3 ; ++i)
    {
        Mesh::Element elem;
        elem.drawState().setShader(gPass);
        elem.setGeometry(geometry[i]);
        elem.setRougness(0.5);
        elem.setMetallic(0);
        elem.setSpecular(0.1);
        elem.setColor(vec4(vec3(Rand::frand(), Rand::frand(), Rand::frand()).saturated(),1));

        mesh[i].addElement(elem);
    }

    /* Pipeline entity */
    Pipeline::SceneEntity<SimpleScene> sceneEntity;
    sceneEntity.globalLight.dirLights.push_back({vec3(1,1,-1), vec4::construct(1), true});

    Pipeline::SceneView sceneView;
    sceneView.camera.ratio = 1;
    sceneView.camera.clipDist = {.1,1000};
    sceneView.camera.pos = {5,5,5};
    sceneView.camera.dir = {0,0,0};

    sceneEntity.scene.add<MeshInstance>(mesh[0], mat4::Translation({-3,0,0}));
    sceneEntity.scene.add<MeshInstance>(mesh[1], mat4::Translation({0,0,0}));
    sceneEntity.scene.add<MeshInstance>(mesh[2], mat4::Translation({3,0,0}));

    _pipeline.setScene(sceneEntity, sceneView);

    unlock();
    while(_running)
    {
        lock();

        if(_pipeline.pipeline)
        {
            openGL.clearColor({1,0,0,0});
            _pipeline.pipeline->prepare();
            _pipeline.pipeline->render();
        }

        GL_ASSERT();

        _parent->swapBuffers();

        resize();
        _pipeline.setScene(sceneEntity, sceneView);
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
    }
    unlock();
}

void MainRenderer::resize()
{
    if(_newSize)
    {
        openGL.resetStates();
        _pipeline.create(_currentSize, _renderingParameter);
        //openGL.resetStates();
    }
    _newSize=false;
}

}

