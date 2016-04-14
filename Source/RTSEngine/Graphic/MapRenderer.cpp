#include "MapRenderer.h"

MapRenderer::MapRenderer(float unitSize, tim::interface::SimpleScene& scene) : _unitSize(unitSize), _scene(scene)
{

}

MapRenderer::Element::Element(MapRenderer& r) : _renderer(r)
{

}

MapRenderer::Element::~Element()
{
    for(auto& obj : _graphicObject)
    {
        _renderer._scene.remove(obj);
    }
}

