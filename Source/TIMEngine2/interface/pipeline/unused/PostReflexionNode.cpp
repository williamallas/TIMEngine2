
#include "PostReflexionNode.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
    using namespace renderer;
namespace interface
{
namespace pipeline
{

PostReflexionNode::PostReflexionNode(const FrameParameter& param)
    : _reflexionRenderer(param)
{

}

void PostReflexionNode::prepare()
{
    Pipeline::InBuffersNode::prepare();
}

void PostReflexionNode::render()
{
    if(!tryRender()) return;

    for(uint i=0 ; i<_input.size() ; ++i)
    {
        if(_input[i])
            _input[i]->render();
    }

    vector<Texture*> tex(_input.size());
    for(size_t i=0 ; i<_input.size() ; ++i)
        tex[i] = _input[i]->buffer();

    _reflexionRenderer.draw(tex);
}

}
}
}
