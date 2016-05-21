#ifndef pipeline_NODE_H
#define pipeline_NODE_H

#include "interface/pipeline/DeferredRendererNode.h"
#include "interface/pipeline/DirLightShadowNode.h"
#include "interface/pipeline/SceneCullingNode.h"
#include "interface/pipeline/DirLightCullingNode.h"
#include "interface/pipeline/OnScreenRenderer.h"
#include "interface/pipeline/SimpleFilter.h"

namespace tim{
namespace interface{
    using Scene = Pipeline::SceneEntity<SimpleScene>;
    using View = Pipeline::SceneView;
}
}


#endif // pipeline_NODE_H
