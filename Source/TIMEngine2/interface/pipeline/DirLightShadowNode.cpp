
#include "DirLightShadowNode.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
namespace pipeline
{

DirLightShadowNode::DirLightShadowNode(renderer::MeshRenderer& meshDrawer)
    : Pipeline::DepthMapRendererNode(), _meshDrawer(meshDrawer), _buffer(renderer::texBufferPool)
{
    _defaultDrawState.setShader(renderer::depthPassShader);
    _defaultDrawState.setCullBackFace(true);

    const float arr[3] = {50,150,500};
    for(uint i=0 ; i<3 ; ++i)
    {
        _sizeOrtho[i] = vec3(arr[i], arr[i], 1000);
        _orthoMatrix[i] = mat4::Ortho(-_sizeOrtho[i].x(), _sizeOrtho[i].x(),
                                      -_sizeOrtho[i].y(), _sizeOrtho[i].y(),
                                      -_sizeOrtho[i].z(), _sizeOrtho[i].z());
    }
}

void DirLightShadowNode::acquire(int)
{
    if(_needUpdate)
    {
        _matrix.resize(_resolution.z());
        _needUpdate = false;

        renderer::TextureBufferPool::Key k;
        k.type = renderer::TextureBufferPool::Key::DEPTH_MAP_ARRAY;
        k.res = _resolution;
        _buffer.setParameter(k);
    }

    _buffer.acquire();
}

void DirLightShadowNode::release(int)
{
    _buffer.release();
}

void DirLightShadowNode::setShadowLightRange(const vector<float>& r)
{
    for(uint i=0 ; i<std::min(renderer::MAX_SHADOW_MAP_LVL, r.size()) ; ++i)
    {
        _sizeOrtho[i] = vec3(r[i], r[i], 1000);
        _orthoMatrix[i] = mat4::Ortho(-_sizeOrtho[i].x(), _sizeOrtho[i].x(),
                                      -_sizeOrtho[i].y(), _sizeOrtho[i].y(),
                                      -_sizeOrtho[i].z(), _sizeOrtho[i].z());
    }

    if(_resolution.z() != std::min(renderer::MAX_SHADOW_MAP_LVL, r.size()))
    {
        _needUpdate = true;
        _resolution.z() = std::min(renderer::MAX_SHADOW_MAP_LVL, r.size());
    }
}

void DirLightShadowNode::setDepthMapResolution(uint res)
{
    if(_resolution.x() != res || _resolution.y() != res)
    {
        _resolution = {res,res,_resolution.z()};
        _needUpdate = true;
    }
}

DirLightShadowNode::~DirLightShadowNode()
{

}

renderer::Texture* DirLightShadowNode::buffer(uint index) const
{
    return _buffer.buffer(index);
}

void DirLightShadowNode::prepare()
{
    if(!tryPrepare()) return;

    if(!_sceneView)
        return;


    for(uint i=0 ; i<renderer::MAX_SHADOW_MAP_LVL ; ++i)
        _toDraw[i].clear();

    for(size_t i=0 ; i<_meshInstanceSource.size() ; ++i)
    {
        if(!_meshInstanceSource[i]) continue;
        _meshInstanceSource[i]->prepare();

        for(uint j=0 ; j<_resolution.z() ; ++j)
        {
            const auto& culledMesh =_meshInstanceSource[i]->get(j);

            for(const MeshInstance& m : culledMesh)
            {
                for(uint i=0 ; i<m.mesh().nbElements() ; ++i)
                    if(m.mesh().element(i).isEnable() && m.mesh().element(i).castShadow())
                        _toDraw[j].push_back({&(m.mesh().element(i)), &(m.matrix())});
            }
        }
    }
}

void DirLightShadowNode::render()
{
    if(!tryRender()) return;

    if(!_sceneView)
        return;

    for(size_t i=0 ; i<_resolution.z() ; ++i)
    {
#warning OPTIMIZE SHADOW FOR LAST LEVEL
        _buffer.fbo()->attachDepthTexture(_buffer.buffer(0), i);
        _buffer.fbo()->bind();
        renderer::openGL.clearDepth();

        if(!_toDraw[i].empty())
        {
            vector<mat4> accMatr;
            vector<renderer::MeshBuffers*> accMesh;
            //vector<renderer::Material> accMate;

            for(uint index=0 ; index < _toDraw[i].size() ; ++index)
            {
                if(_toDraw[i][index].first->geometry().buffers() && !_toDraw[i][index].first->geometry().buffers()->isNull())
                {
                    accMatr.push_back(_toDraw[i][index].second->transposed());
                    accMesh.push_back(_toDraw[i][index].first->geometry().buffers());
                    //accMate.push_back(_toDraw[i][curIndex].first->internalMaterial());
                }
            }
            if(!accMesh.empty())
            {
                _meshDrawer.setDrawState(_defaultDrawState);

                mat4 viewMat = mat4::View(_sceneView->dirLightView.realPos[i], _sceneView->dirLightView.realPos[i] + _sceneView->dirLightView.lightDir,
                                          _sceneView->dirLightView.up);

                mat4 projView = _orthoMatrix[i] * viewMat;
                _matrix[i] = mat4::BIAS() * projView;
                _defaultDrawState.shader()->bind();
                _defaultDrawState.shader()->setUniform(projView,
                                                       _defaultDrawState.shader()->engineUniformId(renderer::Shader::PROJVIEW));

                _meshDrawer.draw(accMesh, accMatr, {}, {}, false);
            }
        }

        _buffer.fbo()->unbind();
    }
}

}
}
}
