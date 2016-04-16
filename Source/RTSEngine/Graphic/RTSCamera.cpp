#include "RTSCamera.h"

using namespace tim::core;

void RTSCamera::update(float time, Camera& cam)
{
    float dx=0, dy=0;
    if(_mousePos.x() < 5)
        dx = -time*_mouseScrollSensivity.x() * adaptZoomSpeed();
    else if(_mousePos.x() > _resolution.x()-5)
        dx = time*_mouseScrollSensivity.x() * adaptZoomSpeed();

    if(_mousePos.y() < 5)
        dy = time*_mouseScrollSensivity.y() * adaptZoomSpeed();
    else if(_mousePos.y() > _resolution.y()-5)
        dy = -time*_mouseScrollSensivity.y() * adaptZoomSpeed();

    _position += vec3(dx, dy,0);

    /////////////////////////////////

    _zoomAnimation -= time;

    if(_mouseWheel != 0)
        _zoomAnimation += 0.2;

    if(_mouseWheel < 0 && _zoomDirection > 0)
    {
        _zoomDirection = 0;
        _zoomAnimation = 0;
    }
    else if(_mouseWheel > 0 && _zoomDirection < 0)
    {
        _zoomDirection = 0;
        _zoomAnimation = 0;
    }
    else
        _zoomDirection = _mouseWheel;

    if(_zoomAnimation > 0)
    {
        if(_zoomDirection > 0)
            _position += _direction.normalized() * time*_mouseZoomSensivity * adaptZoomSpeed();
        else if(_zoomDirection < 0)
            _position -= _direction.normalized() * time*_mouseZoomSensivity * adaptZoomSpeed();
    }
    else
    {
        _zoomDirection = 0;
        _zoomAnimation = 0;
    }

    /////////////////////////////////////
    cam.pos = _position;
    cam.dir = cam.pos + _direction;

    _mouseWheel=0;
}

void RTSCamera::setMouseParameter(ivec2 mousePos, int mouseWheel)
{
    _mousePos = mousePos;
    _mouseWheel = mouseWheel;
}

float RTSCamera::adaptZoomSpeed() const
{
    return std::max<float>((_position.z()+50) / 100, 1);
}

