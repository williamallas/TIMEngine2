#ifndef DEBUGCAMERA_H_INCLUDED
#define DEBUGCAMERA_H_INCLUDED

#include "TIM_SDL/SDLInputManager.h"
#include "core\Camera.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    class DebugCamera
    {
    public:
        DebugCamera(SDLInputManager* im) : _input(im) {}

        void update(float time, Camera& cam)
        {
            vec3 curdir = cam.dir-cam.pos;

            _angle[0] += -30*time*_input->mouseRel().x();
            _angle[1] += -30*time*_input->mouseRel().y();
            _angle[1] = std::min(89.0f, std::max(-89.0f, _angle[1]));

            float boostv=5;
            if(_input->keyState(SDLK_SPACE).pressed)
                boostv*=4;
            if(_input->keyState(SDLK_v).pressed)
                boostv*=4;
            if(_input->keyState(SDLK_LSHIFT).pressed)
                boostv*=5;

            vec3 dir = {cosf(_angle[1]*PI/180)*cosf(_angle[0]*PI/180),
                        cosf(_angle[1]*PI/180)*sinf(_angle[0]*PI/180),
                        sinf(_angle[1]*PI/180)};

            if(_input->mouseRel().x()==0 && _input->mouseRel().y()==0)
            {
                dir = curdir;
            }

            //bool pressed=false;
            if(_input->keyState(SDLK_w).pressed)
            {
                cam.pos+=dir*time*boostv*0.3;
                //pressed=true;
            }
            else if(_input->keyState(SDLK_s).pressed)
            {
                cam.pos-=dir*time*boostv*0.3;
                //pressed=true;
            }

            if(_input->keyState(SDLK_a).pressed)
            {
                cam.pos-=dir.cross(cam.up).normalized()*time*boostv*0.3;
                //pressed=true;
            }
            else if(_input->keyState(SDLK_d).pressed)
            {
                cam.pos+=dir.cross(cam.up).normalized()*time*boostv*0.3;
                //pressed=true;
            }

            cam.dir = dir + cam.pos;
        }

    private:
        SDLInputManager* _input;
        vec2 _angle;
    };

}
#include "MemoryLoggerOff.h"

#endif // DEBUGCAMERA_H_INCLUDED
