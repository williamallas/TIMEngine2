#ifndef DIRLIGHTCULLING_NODE_H
#define DIRLIGHTCULLING_NODE_H

#include "interface/Pipeline.h"
#include "interface/pipeline/SceneCullingNode.h"
#include "Frustum.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
namespace pipeline
{
    template <class SceneType>
    class DirLightCullingNode : public SceneCullingNode<MeshInstance, SceneType>
    {
    public:
        using Type = MeshInstance;
        using SceneCullingNode<MeshInstance, SceneType>::SceneCullingNode;

        void setShadowLightRange(const vector<float>& v)
        {
            _orthoRange = std::vector<float>(v.begin(), v.end());
            _orthoRange.resize(std::min(v.size(), renderer::MAX_SHADOW_MAP_LVL));
        }

        void setDepthMapResolution(uint res) { _depthMapResolution = res; }

        void prepare() override
        {
            if(!Pipeline::CollectObjectNode<Type>::tryPrepare()) return;

            for(uint i=0 ; i<renderer::MAX_SHADOW_MAP_LVL ; ++i)
                _results[i].clear();

            if(!_scene || !_sceneView) return;

            setupRelativeLightPosition();
            for(uint i=0 ; i<_orthoRange.size() ; ++i)
            {
                vec3 sizeOrtho = vec3(_orthoRange[i], _orthoRange[i], 1000);

                Frustum frustum;
                frustum.buildOrthoFrustum(-sizeOrtho[0], sizeOrtho[0], -sizeOrtho[1], sizeOrtho[1], -sizeOrtho[2], sizeOrtho[2],
                                          mat4::View(_sceneView->dirLightView.realPos[i],
                                                     _sceneView->dirLightView.realPos[i] + _sceneView->dirLightView.lightDir,
                                                     _sceneView->dirLightView.up));

                _scene->scene.template query<Type>(FrustumCulling(frustum), VectorInserter<vector<std::reference_wrapper<Type>>>(_results[i]));
            }
        }

        const vector<std::reference_wrapper<Type>>& get(int index) const override
        {
            return _results[index];
        }

    private:
        using SceneCullingNode<MeshInstance, SceneType>::_scene;
        using SceneCullingNode<MeshInstance, SceneType>::_sceneView;
        vector<std::reference_wrapper<Type>> _results[renderer::MAX_SHADOW_MAP_LVL];
        std::vector<float> _orthoRange = {50,150,500};
        uint _depthMapResolution = 1024;

        void setupRelativeLightPosition()
        {
            mat4 view_nopos = mat4::View(vec3(), _sceneView->dirLightView.lightDir,
                                                 _sceneView->dirLightView.up);

            mat4 inv_view_nopos = view_nopos.inverted();

            for(uint i=0 ; i<_orthoRange.size() ; ++i)
            {
                vec3 pos  = _sceneView->dirLightView.camPos + _sceneView->dirLightView.lightDir*_orthoRange[i];

                float fWorldUnitsPerTexel = _orthoRange[i]*2 / _depthMapResolution;

                vec3 pos_t = view_nopos * pos;
                pos_t = pos_t.mod(vec3::construct(fWorldUnitsPerTexel));
                pos = inv_view_nopos * pos_t;

                _sceneView->dirLightView.realPos[i] = pos;
            }
        }
    };
}
}
}
#include "MemoryLoggerOff.h"

#endif // DIRLIGHTCULLING_NODE_H
