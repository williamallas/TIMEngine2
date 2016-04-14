#ifndef MAPRENDERER_H
#define MAPRENDERER_H

#include "interface/SimpleScene.h"
#include "interface/MeshInstance.h"

class MapRenderer
{
public:

    class Element
    {
    public:
        Element(MapRenderer&);
        virtual ~Element();

    private:
        vec3 _pos;
        vector<std::reference_wrapper<interface::MeshInstance>> _graphicObject;

        MapRenderer& _renderer;
    };

    MapRenderer(float, interface::SimpleScene&);

private:
    float _unitSize;
    interface::SimpleScene& _scene;
};

#endif // MAPRENDERER_H
