#include "HmdSceneView.h"

using namespace tim;

HmdSceneView::HmdSceneView(float fov, float ratio, float far)
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

void HmdSceneView::addOffset(const mat4& o)
{
    _offset = o * _offset;
}

mat4 HmdSceneView::scaleTransform(const mat4& v) const
{
    mat4 res = v.inverted();
    vec3 tr = res.translation();
    res.setTranslation(vec3(0,0,0));
    res.invert();
    return res * mat4::Translation(-tr*_scaleRoom);
}

void HmdSceneView::update(const VR_Device& hmdDevice)
{
    if(!hmdDevice.isHmdConnected())
        return;

    _eyeView[VR_Device::LEFT].camera.useRawMat = true;
    _eyeView[VR_Device::RIGHT].camera.useRawMat = true;

    mat4 inv_o = _offset.inverted();
    _transform = scaleTransform(hmdDevice.camera().hmdView()) * inv_o;

    _eyeView[VR_Device::LEFT].camera.raw_proj = hmdDevice.camera().eyeProjection(VR_Device::LEFT);
    _eyeView[VR_Device::LEFT].camera.raw_view = scaleTransform(hmdDevice.camera().eyeView(VR_Device::LEFT)) * inv_o;
    _eyeView[VR_Device::RIGHT].camera.raw_proj = hmdDevice.camera().eyeProjection(VR_Device::RIGHT);
    _eyeView[VR_Device::RIGHT].camera.raw_view = scaleTransform(hmdDevice.camera().eyeView(VR_Device::RIGHT)) * inv_o;

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

void HmdSceneView::update(const VRDebugCamera& cam, float ratio)
{
    _eyeView[VR_Device::LEFT].camera.useRawMat = true;
    _eyeView[VR_Device::RIGHT].camera.useRawMat = true;

    mat4 inv_o = _offset.inverted();
    _transform = scaleTransform(cam.viewMat()) * inv_o;

    _eyeView[VR_Device::LEFT].camera.raw_proj = mat4::Projection(110, ratio, 0.02, 300);
    _eyeView[VR_Device::LEFT].camera.raw_view = scaleTransform(cam.eyeView(VR_Device::LEFT)) * inv_o;
    _eyeView[VR_Device::RIGHT].camera.raw_proj = _eyeView[VR_Device::LEFT].camera.raw_proj;
    _eyeView[VR_Device::RIGHT].camera.raw_view = scaleTransform(cam.eyeView(VR_Device::RIGHT)) * inv_o;

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
