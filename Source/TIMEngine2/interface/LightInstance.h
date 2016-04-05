#ifndef LIGHTINSTANCE_H
#define LIGHTINSTANCE_H

#include "Matrix.h"
#include "renderer/LightContextRenderer.h"
#include "SimpleScene.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace interface
{
    class LightInstance : public scene::Transformable
    {
        friend class scene::SceneContainer<scene::Transformable>;

    public:
        void set(const renderer::LightContextRenderer::Light& l) { _light=l; setVolume(Sphere(_light.position, _light.radius)); }
        void setPosition(const vec3& p) { _light.position = p; setVolume(Sphere(_light.position, _light.radius)); }
        void setRadius(float r) { _light.radius = r; setVolume(Sphere(_light.position, _light.radius)); }
        void setColor(const vec4& c) { _light.color = c; }
        void setPower(float p) { _light.power = p; }

        const renderer::LightContextRenderer::Light& get() const { return _light; }

    protected:
        renderer::LightContextRenderer::Light _light;

        LightInstance() = default;
        ~LightInstance() = default;

        LightInstance(const renderer::LightContextRenderer::Light& l) : _light(l) { setVolume(Sphere(_light.position, _light.radius)); }
    };
}
}
#include "MemoryLoggerOff.h"

#endif // MESHINSTANCE_H
