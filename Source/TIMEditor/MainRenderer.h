#ifndef MAINRENDERER_H
#define MAINRENDERER_H

#include "FullPipeline.h"
#include "interface/ShaderPool.h"
#include "resource/MeshLoader.h"
#include "resource/AssetManager.h"
#include "interface/pipeline/pipeline.h"

#include <QMutex>

#include "RendererWidget.h"

#include "MemoryLoggerOn.h"
namespace tim{
class MainRenderer
{
public:
    MainRenderer(RendererWidget*);
    void main();

    FullPipeline& pipeline() { return _pipeline; }
    void lock() const { _mutex.lock(); }
    void unlock() const { _mutex.unlock(); }
    void stop() { _running=false; }

    void updateSize(uivec2);

private:
    RendererWidget* _parent;
    bool _running;

    bool _newSize=false;
    uivec2 _currentSize;

    FullPipeline::Parameter _renderingParameter;
    FullPipeline _pipeline;
    mutable QMutex _mutex;

    void resize();
};

}
#include "MemoryLoggerOff.h"

#endif // MAINRENDERER_H
