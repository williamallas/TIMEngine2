#ifndef RTSCAMERA_H
#define RTSCAMERA_H

#include "core.h"
#include "core\Camera.h"

class RTSCamera
{
public:
    RTSCamera() = default;

    void setResolution(ivec2 r) { _resolution = r; }
    void setMouseParameter(ivec2 mousePos, int mouseWheel);
    void update(float, tim::core::Camera&);

private:
    ivec2 _resolution;
    vec3 _position = {0,0,100};
    vec3 _direction = {0,3,-5};

    ivec2 _mousePos;
    int _mouseWheel;

    // internal state
    int _zoomDirection=0;
    float _zoomAnimation=0; // delay to smooth the zoom/dezoom

    // user parameter
    vec2 _mouseScrollSensivity = {100,100};
    float _mouseZoomSensivity = 200;

    float adaptZoomSpeed() const;

};

#endif // RTSCAMERA_H
