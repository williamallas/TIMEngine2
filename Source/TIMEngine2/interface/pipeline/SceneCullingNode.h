#ifndef SCENECULLING_NODE_H
#define SCENECULLING_NODE_H

#include "interface/Pipeline.h"
#include "Camera.h"
#include "Frustum.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
namespace pipeline
{
    template <class Type, class SceneType>
    class SceneCullingNode : public Pipeline::CollectObjectNode<Type>
    {
    public:
        SceneCullingNode(Pipeline::SceneEntity<SceneType>& scene, Pipeline::SceneView& view) : _scene(&scene), _sceneView(&view) {}
        SceneCullingNode() : _scene(nullptr), _sceneView(nullptr) {}

        void setScene(Pipeline::SceneEntity<SceneType>& scene) { _scene = &scene; }
        void setSceneView(Pipeline::SceneView& view) { _sceneView = &view; }

        virtual void prepare() override
        {
            if(!Pipeline::CollectObjectNode<Type>::tryPrepare()) return;

            _result.clear();
            if(!_scene || !_sceneView) return;

            Frustum frustum;
            frustum.buildCameraFrustum(_sceneView->camera);
            _scene->scene.template query<Type>(FrustumCulling(frustum), VectorInserter<vector<std::reference_wrapper<Type>>>(_result));
        }

        virtual const vector<std::reference_wrapper<Type>>& get(int) const override
        {
            return _result;
        }

    protected:
         Pipeline::SceneEntity<SceneType>* _scene;
         Pipeline::SceneView* _sceneView;

         vector<std::reference_wrapper<Type>> _result;
    };

    using SimpleSceneMeshCullingNode = SceneCullingNode<MeshInstance, SimpleScene>;
    using SimpleSceneLightCullingNode = SceneCullingNode<LightInstance, SimpleScene>;
}
}
}
#include "MemoryLoggerOff.h"

#endif // SCENECULLING_NODE_H
