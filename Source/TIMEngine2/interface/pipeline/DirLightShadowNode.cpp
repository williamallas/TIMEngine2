
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
    : Pipeline::DepthMapRendererNode(), _meshDrawer(meshDrawer)
{
    _defaultDrawState.setShader(renderer::depthPassShader);
    _defaultDrawState.setCullBackFace(false);

    const float arr[3] = {50,150,500};
    for(uint i=0 ; i<3 ; ++i)
    {
        _sizeOrtho[i] = vec3(arr[i], arr[i], 1000);
        _orthoMatrix[i] = mat4::Ortho(-_sizeOrtho[i].x(), _sizeOrtho[i].x(),
                                      -_sizeOrtho[i].y(), _sizeOrtho[i].y(),
                                      -_sizeOrtho[i].z(), _sizeOrtho[i].z());
    }
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
    for(size_t i=0 ; i<_depthBuffer.size() ; ++i)
        delete _depthBuffer[i];
}

renderer::Texture* DirLightShadowNode::buffer(uint index) const
{
    if(_depthBuffer.empty())
        return nullptr;
    else
    {
        index = std::min(0u, _depthBuffer.size()-1);
        return _depthBuffer[index];
    }
}

void DirLightShadowNode::prepare()
{
    if(!tryPrepare()) return;

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
                    _toDraw[j].push_back({&(m.mesh().element(i)), &(m.matrix())});
            }
        }
    }
}

void DirLightShadowNode::render()
{
    if(!tryRender()) return;

    if(_needUpdate)
    {
        _matrix.resize(_resolution.z());
        _needUpdate = false;

        for(size_t i=0 ; i<_depthBuffer.size() ; ++i)
            delete _depthBuffer[i];

        _depthBuffer.clear();
        //_depthBuffer.resize(_resolution.z());
        _depthBuffer.resize(1); // we use 1 internal array texture

        for(size_t i=0 ; i<_depthBuffer.size() ; ++i)
        {
            renderer::Texture::GenTexParam p;
            p.format = renderer::Texture::Format::DEPTHCOMPONENT;
            p.nbLevels = 1;
            p.size = _resolution;

            _depthBuffer[i] = renderer::Texture::genTextureArray2D(p);
        }

        _fbo.setResolution(_resolution.to<2>());
    }

    for(size_t i=0 ; i<_resolution.z() ; ++i)
    {
        _fbo.attachDepthTexture(_depthBuffer[0], i);
        _fbo.bind();
        renderer::openGL.clearDepth();

        if(!_toDraw[i].empty())
        {
            vector<mat4> accMatr;
            vector<renderer::MeshBuffers*> accMesh;
            //vector<renderer::Material> accMate;

            for(uint index=0 ; index < _toDraw[i].size() ; ++index)
            {
                if(_toDraw[i][index].first->geometry().buffers())
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

                _meshDrawer.draw(accMesh, accMatr, {}, false);
            }
        }

        _fbo.unbind();
    }
}

}
}
}
