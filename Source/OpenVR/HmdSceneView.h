#ifndef HMD_SCENEVIEW_H
#define HMD_SCENEVIEW_H

#include "interface/pipeline/pipeline.h"
#include "OpenVR/VR_Device.h"
#include "OpenVR/VRDebugCamera.h"

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
            _offset = o * _offset;
        }

        const mat4& offset() const { return _offset; }

        void update(const VR_Device& hmdDevice)
        {
            if(!hmdDevice.isHmdConnected())
                return;

            _eyeView[VR_Device::LEFT].camera.useRawMat = true;
            _eyeView[VR_Device::RIGHT].camera.useRawMat = true;

            mat4 inv_o = _offset.inverted();
            _transform = hmdDevice.camera().hmdView() * inv_o;

            _eyeView[VR_Device::LEFT].camera.raw_proj = hmdDevice.camera().eyeProjection(VR_Device::LEFT);
            _eyeView[VR_Device::LEFT].camera.raw_view = hmdDevice.camera().eyeView(VR_Device::LEFT) * inv_o;
            _eyeView[VR_Device::RIGHT].camera.raw_proj = hmdDevice.camera().eyeProjection(VR_Device::RIGHT);
            _eyeView[VR_Device::RIGHT].camera.raw_view = hmdDevice.camera().eyeView(VR_Device::RIGHT) * inv_o;

            mat4 inv_l = _eyeView[VR_Device::LEFT].camera.raw_view.inverted();
            mat4 inv_r = _eyeView[VR_Device::RIGHT].camera.raw_view.inverted();

            _eyeView[VR_Device::RIGHT].camera.pos = inv_r.translation();
            _eyeView[VR_Device::LEFT].camera.pos = inv_l.translation();

            _eyeView[VR_Device::LEFT].camera.dir =  -_eyeView[VR_Device::LEFT].camera.raw_view[2].to<3>();
            _eyeView[VR_Device::RIGHT].camera.dir = -_eyeView[VR_Device::RIGHT].camera.raw_view[2].to<3>();
            _eyeView[VR_Device::LEFT].camera.dir +=  _eyeView[VR_Device::LEFT].camera.pos;
            _eyeView[VR_Device::RIGHT].camera.dir += _eyeView[VR_Device::RIGHT].camera.pos;

            mat4 inv_t = _transform.inverted();
            _cullingView.camera.pos = inv_t.translation();
            _cullingView.camera.dir = -_transform[2].to<3>() + _cullingView.camera.pos;
            _cullingView.camera.up = _transform[1].to<3>();
        }

        void update(const VRDebugCamera& cam, float ratio)
        {
            _eyeView[VR_Device::LEFT].camera.useRawMat = true;
            _eyeView[VR_Device::RIGHT].camera.useRawMat = true;

            mat4 inv_o = _offset.inverted();
            _transform = cam.viewMat() * inv_o;

            _eyeView[VR_Device::LEFT].camera.raw_proj = mat4::Projection(110, ratio, 0.02, 300);
            _eyeView[VR_Device::LEFT].camera.raw_view = cam.eyeView(VR_Device::LEFT) * inv_o;
            _eyeView[VR_Device::RIGHT].camera.raw_proj = _eyeView[VR_Device::LEFT].camera.raw_proj;
            _eyeView[VR_Device::RIGHT].camera.raw_view = cam.eyeView(VR_Device::RIGHT) * inv_o;

            mat4 inv_l = _eyeView[VR_Device::LEFT].camera.raw_view.inverted();
            mat4 inv_r = _eyeView[VR_Device::RIGHT].camera.raw_view.inverted();

            _eyeView[VR_Device::RIGHT].camera.pos = inv_r.translation();
            _eyeView[VR_Device::LEFT].camera.pos = inv_l.translation();

            _eyeView[VR_Device::LEFT].camera.dir =  -_eyeView[VR_Device::LEFT].camera.raw_view[2].to<3>();
            _eyeView[VR_Device::RIGHT].camera.dir = -_eyeView[VR_Device::RIGHT].camera.raw_view[2].to<3>();
            _eyeView[VR_Device::LEFT].camera.dir +=  _eyeView[VR_Device::LEFT].camera.pos;
            _eyeView[VR_Device::RIGHT].camera.dir += _eyeView[VR_Device::RIGHT].camera.pos;

            mat4 inv_t = _transform.inverted();
            _cullingView.camera.pos = inv_t.translation();
            _cullingView.camera.dir = -_transform[2].to<3>() + _cullingView.camera.pos;
            _cullingView.camera.up = _transform[1].to<3>();
        }

        void update(const Camera& cam)
        {
            throw Exception("update(const Camera&) from HmdSceneView not implemented");

//            _cullingView.camera = cam;

//            mat4 proj = mat4::Projection(_renderFov, cam.ratio, cam.clipDist.x(), cam.clipDist.y());
//            mat4 view = mat4::View(_cullingView.camera.pos, _cullingView.camera.dir, _cullingView.camera.up);
//            _eyeView[VR_Device::LEFT].camera.useRawMat = true;
//            _eyeView[VR_Device::RIGHT].camera.useRawMat = true;
//            _eyeView[VR_Device::LEFT].camera.raw_proj = proj;
//            _eyeView[VR_Device::RIGHT].camera.raw_proj = proj;
//            _eyeView[VR_Device::RIGHT].camera.raw_view = mat4::Translation(vec3(0.04,0,0))*view;
//            _eyeView[VR_Device::LEFT].camera.raw_view = mat4::Translation(vec3(-0.04,0,0))*view;

//            _eyeView[VR_Device::RIGHT].camera.pos = _eyeView[VR_Device::RIGHT].camera.raw_view.inverted().translation();
//            _eyeView[VR_Device::LEFT].camera.pos = _eyeView[VR_Device::LEFT].camera.raw_view.inverted().translation();

//            vec3 dir = _cullingView.camera.dir - _cullingView.camera.pos;
//            _transform = view;

//            _eyeView[VR_Device::LEFT].camera.dir = _eyeView[VR_Device::LEFT].camera.pos + dir;
//            _eyeView[VR_Device::RIGHT].camera.dir = _eyeView[VR_Device::RIGHT].camera.pos + dir;
        }

        const mat4& transform() const { return _transform; }

    private:
        interface::View _cullingView;
        interface::View _eyeView[2];
        float _renderFov = 110;

        mat4 _offset = mat4::IDENTITY();
        mat4 _transform = mat4::IDENTITY();
    };
}

#endif // HMD_SCENEVIEW_H
