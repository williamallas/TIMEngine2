#ifndef VRDEBUGCAMERA_H_INCLUDED
#define VRDEBUGCAMERA_H_INCLUDED

#include "TIM_SDL/SDLInputManager.h"
#include "core/Camera.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    class VRDebugCamera
    {
    public:
        VRDebugCamera(SDLInputManager* im, vec3 sizeRoom) : _input(im), _sizeRoom(sizeRoom) {}

        void update(float time)
        {
            vec3 hvec = _dir.cross(vec3(0,0,1)).normalized();
            mat3 rotZ = mat3::RotationZ(-time * _input->mouseRel().x() * 0.5);
            _dir = rotZ * _dir;
            _dir = mat3::AxisRotation(hvec, vec3(0,0,1), -time * _input->mouseRel().y() * 0.5) * _dir;
            _dir.normalize();

            float boostv = 1;
            if(_input->keyState(SDLK_SPACE).pressed)
            {
                boostv = 10;
            }
            if(_input->keyState(SDLK_w).pressed)
            {
                _pos += _dir*time*boostv;
            }
            else if(_input->keyState(SDLK_s).pressed)
            {
                _pos -= _dir*time*boostv;
            }

            if(_input->keyState(SDLK_a).pressed)
            {
                _pos -= hvec*time*boostv;
            }
            else if(_input->keyState(SDLK_d).pressed)
            {
                _pos += hvec*time*boostv;
            }

            _pos.z() += _input->mouseWheel().y() * 5 * time;

            _pos.z() = std::min(_sizeRoom.z(), std::max(0.f, _pos.z()));
            _pos.x() = std::min(_sizeRoom.x()*0.5f, std::max(-_sizeRoom.x()*0.5f, _pos.x()));
            _pos.y() = std::min(_sizeRoom.y()*0.5f, std::max(-_sizeRoom.y()*0.5f, _pos.y()));

            _eyeView[0] = mat4::View(_pos + hvec*0.05, _pos + hvec*0.05 + _dir, vec3(0,0,1));
            _eyeView[1] = mat4::View(_pos - hvec*0.05, _pos - hvec*0.05 + _dir, vec3(0,0,1));

            _view = mat4::View(_pos, _pos + _dir, vec3(0,0,1));
        }

        const mat4& viewMat() const { return _view; }
        const mat4& eyeView(int e) const { return _eyeView[e]; }

        vec3 pos() const { return _pos; }
        vec3 dir() const { return _dir; }

    private:
        SDLInputManager* _input;
        vec3 _sizeRoom;

        vec3 _pos = {0,0,1.6}, _dir = {0,1,0};

        mat4 _eyeView[2];
        mat4 _view;

    };

}
#include "MemoryLoggerOff.h"

#endif // VRDEBUGCAMERA_H_INCLUDED
