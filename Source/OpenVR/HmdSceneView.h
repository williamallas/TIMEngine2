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
            _cullingView.camera.ratio = ratio;
            _cullingView.camera.fov = fov+10;
            _cullingView.camera.clipDist = { .03f, far };

            for (int i=0 ; i<2 ; ++i)
            {
                _eyeView[i].camera.ratio = ratio;
                _eyeView[i].camera.clipDist = { .03f, far };
                _eyeView[i].camera.fov = fov;
            }
        }

        interface::View& cullingView() { return _cullingView; }
        const interface::View& cullingView() const { return _cullingView; }

        interface::View& eyeView(int eye) { return _eyeView[eye]; }
        const interface::View& eyeView(int eye) const { return _eyeView[eye]; }

        void update(const VR_Device& hmdDevice, vec3 offsetPos)
        {
            _eyeView[VR_Device::LEFT].camera.useRawMat = true;
            _eyeView[VR_Device::RIGHT].camera.useRawMat = true;
            _eyeView[VR_Device::LEFT].camera.raw_proj = hmdDevice.camera().eyeProjection(VR_Device::LEFT);
            _eyeView[VR_Device::LEFT].camera.raw_view = hmdDevice.camera().eyeView(VR_Device::LEFT) * mat4::Translation(-offsetPos);
            _eyeView[VR_Device::RIGHT].camera.raw_proj = hmdDevice.camera().eyeProjection(VR_Device::RIGHT);
            _eyeView[VR_Device::RIGHT].camera.raw_view = hmdDevice.camera().eyeView(VR_Device::RIGHT) * mat4::Translation(-offsetPos);

            mat4 matInv = hmdDevice.camera().hmdView().inverted();
            _cullingView.camera.pos = matInv.translation() + offsetPos;
            _eyeView[VR_Device::RIGHT].camera.pos = _eyeView[VR_Device::RIGHT].camera.raw_view.inverted().translation() + offsetPos;
            _eyeView[VR_Device::LEFT].camera.pos = _eyeView[VR_Device::LEFT].camera.raw_view.inverted().translation() + offsetPos;

            vec3 dir = matInv * vec3(0, 0, -5);
            _cullingView.camera.dir = _cullingView.camera.pos + dir;

            _eyeView[VR_Device::LEFT].camera.dir = _eyeView[VR_Device::LEFT].camera.pos + dir;
            _eyeView[VR_Device::RIGHT].camera.dir = _eyeView[VR_Device::RIGHT].camera.pos + dir;
        }

        void update(const Camera& cam)
        {
            _cullingView.camera = cam;

            mat4 proj = mat4::Projection(cam.fov, cam.ratio, cam.clipDist.x(), cam.clipDist.y());
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
    };
}

#endif // HMD_SCENEVIEW_H
