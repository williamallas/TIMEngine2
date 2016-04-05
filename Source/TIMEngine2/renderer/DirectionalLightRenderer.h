#ifndef DIRECTIONALLIGHTRENDERER_H
#define DIRECTIONALLIGHTRENDERER_H

#include "LightContextRenderer.h"
#include "DrawState.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace renderer
{

    class DirectionalLightRenderer : boost::noncopyable
    {
    public:

        struct Light
        {
            vec3 direction;
            vec3 color;
            renderer::Texture* depthMap = nullptr;
            vector<mat4> matrix;
        };

        DirectionalLightRenderer(LightContextRenderer&, Shader* shader=nullptr);
        ~DirectionalLightRenderer();

        void draw(const vector<Light>&) const;
        Shader* shader() const { return _shader; }

    protected:
        LightContextRenderer& _context;
        Shader* _shader;
        DrawState _state;

        int _uniformNbLight, _uniformLightDir, _uniformLightColor, _uniformLightMatrix;
    };

}
}
#include "MemoryLoggerOff.h"

#endif // DIRECTIONALLIGHTRENDERER_H
