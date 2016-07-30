#ifndef HMD_SCENEVIEW_H
#define HMD_SCENEVIEW_H

#include "interface/pipeline/pipeline.h"
#include "OpenVR/VR_Device.h"

namespace tim {
    struct HmdSceneView
    {
    public:
        HmdSceneView(float fov, float ratio, float far=500)
        {
            _renderFov = fov;
            _cullingView.camera.ratio = ratio;
            _cullingView.camera.fov = fov+10;
            _cullingView.camera.clipDist = { .02f, far };

            for (int i=0 ; i<2 ; ++i)
            {
                _eyeView[i].camera.ratio = ratio;
                _eyeView[i].camera.clipDist = { .02f, far };
                _eyeView[i].camera.fov = fov;
            }
        }

        interface::View& cullingView() { return _cullingView; }
        const interface::View& cullingView() const { return _cullingView; }

        interface::View& eyeView(int eye) { return _eyeView[eye]; }
        const interface::View& eyeView(int eye) const { return _eyeView[eye]; }

        void addOffset(const mat4& o)
        {
            _offset *= o;
        }

        const mat4& offset() const { return _offset; }

        void update(const VR_Device& hmdDevice)
        {
            if(!hmdDevice.isHmdConnected())
                return;

            _eyeView[VR_Device::LEFT].camera.useRawMat = true;
            _eyeView[VR_Device::RIGHT].camera.useRawMat = true;

            mat4 inv_o = _offset.inverted();

            _eyeView[VR_Device::LEFT].camera.raw_proj = hmdDevice.camera().eyeProjection(VR_Device::LEFT);
            _eyeView[VR_Device::LEFT].camera.raw_view = hmdDevice.camera().eyeView(VR_Device::LEFT) * inv_o;
            _eyeView[VR_Device::RIGHT].camera.raw_proj = hmdDevice.camera().eyeProjection(VR_Device::RIGHT);
            _eyeView[VR_Device::RIGHT].camera.raw_view = hmdDevice.camera().eyeView(VR_Device::RIGHT) * inv_o;

            _cullingView.camera.pos = _offset * hmdDevice.camera().hmdView().inverted().translation();

            _eyeView[VR_Device::RIGHT].camera.pos = _eyeView[VR_Device::RIGHT].camera.raw_view.inverted().translation();
            _eyeView[VR_Device::LEFT].camera.pos = _eyeView[VR_Device::LEFT].camera.raw_view.inverted().translation();

            vec3 dir =  _offset.to<3>() * hmdDevice.camera().hmdView()[2].to<3>();
            _cullingView.camera.dir = _cullingView.camera.pos - dir;

            _eyeView[VR_Device::LEFT].camera.dir = _eyeView[VR_Device::LEFT].camera.pos + dir;
            _eyeView[VR_Device::RIGHT].camera.dir = _eyeView[VR_Device::RIGHT].camera.pos + dir;
        }

        void update(const Camera& cam)
        {
            _cullingView.camera = cam;

            mat4 proj = mat4::Projection(_renderFov, cam.ratio, cam.clipDist.x(), cam.clipDist.y());
            mat4 view = mat4::View(_cullingView.camera.pos, _cullingView.camera.dir, _cullingView.camera.up);
            _eyeView[VR_Device::LEFT].camera.useRawMat = true;
            _eyeView[VR_Device::RIGHT].camera.useRawMat = true;
            _eyeView[VR_Device::LEFT].camera.raw_proj = proj;
            _eyeView[VR_Device::RIGHT].camera.raw_proj = proj;
            _eyeView[VR_Device::RIGHT].camera.raw_view = mat4::Translation(vec3(0.04,0,0))*view;
            _eyeView[VR_Device::LEFT].camera.raw_view = mat4::Translation(vec3(-0.04,0,0))*view;

            _eyeView[VR_Device::RIGHT].camera.pos = _eyeView[VR_Device::RIGHT].camera.raw_view.inverted().translation();
            _eyeView[VR_Device::LEFT].camera.pos = _eyeView[VR_Device::LEFT].camera.raw_view.inverted().translation();

            vec3 dir = _cullingView.camera.dir - _cullingView.camera.pos;

            _eyeView[VR_Device::LEFT].camera.dir = _eyeView[VR_Device::LEFT].camera.pos + dir;
            _eyeView[VR_Device::RIGHT].camera.dir = _eyeView[VR_Device::RIGHT].camera.pos + dir;
        }

    private:
        interface::View _cullingView;
        interface::View _eyeView[2];
        float _renderFov = 110;

        mat4 _offset = mat4::IDENTITY();
    };
}

#endif // HMD_SCENEVIEW_H
