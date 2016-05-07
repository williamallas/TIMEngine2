#ifndef FRAMEPARAM_H_INCLUDED
#define FRAMEPARAM_H_INCLUDED

#include "core/core.h"
#include "Matrix.h"
#include "Camera.h"
#include "GpuBuffer.h"

namespace tim
{
    using namespace core;
namespace renderer
{
    class FrameParameter
    {
    public:
        FrameParameter()
        {
            _parameter.proj = mat4::Projection(70,1,1,100);
            _uboParameter.create(1, &_parameter, DrawMode::STREAM);
            _hasChanged = false;
        }

        FrameParameter& setCamera(const Camera& camera)
        {
			if (!camera.useRawMat)
			{
				_parameter.proj = mat4::Projection(camera.fov, camera.ratio, camera.clipDist.x(), camera.clipDist.y());
				_parameter.view = mat4::View(camera.pos, camera.dir, camera.up);
				_parameter.cameraPos = vec4(camera.pos, 0);
				_parameter.cameraDir = vec4(camera.dir, 0);
				_parameter.cameraUp = vec4(camera.up, 0);
			}
			else
			{
				_parameter.proj = camera.raw_proj;
				_parameter.view = camera.raw_view;
				_parameter.cameraPos = vec4(camera.pos, 0);
				_parameter.cameraDir = vec4(camera.dir, 0);
				_parameter.cameraUp = vec4(camera.up, 0);
			}
            
            _hasChanged = true;
            return *this;
        }

        FrameParameter& setViewMatrix(const mat4& view, const vec3& camPos)
        {
            _parameter.view = view;
            _parameter.cameraPos = vec4(camPos,0);
            _hasChanged = true;
            return *this;
        }

        FrameParameter& setWordlOrigin(const vec3& wo)
        {
            _parameter.worldOrigin = vec4(wo,0);
            _hasChanged = true;
            return *this;
        }

        FrameParameter& setProjectionMatrix(const mat4& proj)
        {
            _parameter.proj = proj;
            _hasChanged = true;
            return *this;
        }

        FrameParameter& setTime(float time, float frameTime)
        {
            _parameter.time = {time, frameTime,0,0 };
            _hasChanged = true;
            return *this;
        }

        const FrameParameter& bind(uint index) const
        {
            if(!_hasChanged)
            {
                openGL.bindUniformBuffer(_uboParameter.id(), index);
                return *this;
            }

            _parameter.invProj = _parameter.proj.inverted();
            _parameter.invView = _parameter.view.inverted();
            _parameter.projView = _parameter.proj * _parameter.view;
            _parameter.invViewProj = _parameter.projView.inverted();//_parameter.invView * _parameter.invProj;

            _parameter.worldOriginInvViewProj = _parameter.view;
            _parameter.worldOriginInvViewProj[0].w()=0;
            _parameter.worldOriginInvViewProj[1].w()=0;
            _parameter.worldOriginInvViewProj[2].w()=0;
            _parameter.worldOriginInvViewProj *= mat4::Translation(-_parameter.cameraPos.to<3>()+_parameter.worldOrigin.to<3>());
            _parameter.worldOriginInvViewProj = (_parameter.proj*_parameter.worldOriginInvViewProj).inverted();
            _hasChanged = false;

            Parameter tmp = _parameter;
            tmp.invProj.transpose();
            tmp.invView.transpose();
            tmp.invViewProj.transpose();
            tmp.projView.transpose();
            tmp.proj.transpose();
            tmp.view.transpose();
            tmp.worldOriginInvViewProj.transpose();
            _uboParameter.flush(&tmp, 0,1);

            openGL.bindUniformBuffer(_uboParameter.id(), index);
            return *this;
        }

        uint ubo() const { return _uboParameter.id(); }

    private:
        struct Parameter // std140
        {
            mat4 view, proj;
            mat4 projView, invView, invProj, invViewProj, worldOriginInvViewProj;
            vec4 cameraPos, cameraUp, cameraDir, worldOrigin;
            vec4 time;
        };

        mutable Parameter _parameter;
        mutable bool _hasChanged;

        UniformBuffer<Parameter> _uboParameter;
    };
}
}

#endif // FRAMEPARAM_H_INCLUDED
