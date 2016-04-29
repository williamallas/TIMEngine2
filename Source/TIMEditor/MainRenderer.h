#ifndef MAINRENDERER_H
#define MAINRENDERER_H

#include "FullPipeline.h"
#include "interface/ShaderPool.h"
#include "resource/MeshLoader.h"
#include "resource/AssetManager.h"
#include "interface/pipeline/pipeline.h"

#include <QMutex>
#include <QQueue>
#include <functional>

#include "RendererWidget.h"
#undef interface
#include "MemoryLoggerOn.h"
namespace tim{
class MainRenderer
{
public:
    MainRenderer(RendererWidget*);
    void main();
    float elapsedTime() const { return _time; }

    FullPipeline& pipeline() { return _pipeline; }
    void lock() const { _mutex.lock(); }
    void unlock() const { _mutex.unlock(); }
    void stop() { _running=false; }

    void updateSize(uivec2);

    void updateCamera_MeshEditor(int wheel);

    tim::interface::Pipeline::SceneView& getSceneView(int index) { return _view[index]; }

    tim::interface::Pipeline::SceneEntity<tim::interface::SimpleScene>&
        getScene(int index) { return _scene[index]; }

    void setCurSceneIndex(int index) { _curScene = index; }

    void addEvent(std::function<void()>);

    void setSkybox(int sceneIndex, QList<QString>);

private:
    RendererWidget* _parent;
    bool _running;
    float _time = 0;

    bool _newSize=false;
    uivec2 _currentSize;

    FullPipeline::Parameter _renderingParameter;
    FullPipeline _pipeline;
    mutable QMutex _mutex;


    /* Shared state */
    const int NB_SCENE=2;
    tim::interface::Pipeline::SceneView _view[2];
    tim::interface::Pipeline::SceneEntity<tim::interface::SimpleScene> _scene[2];
    //std::pair<Texture>
    int _curScene=0;

    mutable QMutex _eventMutex;
    QQueue<std::function<void()>> _events;

    void resize();
};

}
#include "MemoryLoggerOff.h"

#endif // MAINRENDERER_H
