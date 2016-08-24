#ifndef SDLINPUTMANAGER_H__
#define SDLINPUTMANAGER_H__

#include <SDL.h>
#include "core/InputManager.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    class SDLInputManager : public interface::InputManager<SDL_Keycode, SDL_Scancode>
    {
    public:
        SDLInputManager();
        virtual ~SDLInputManager();

        virtual void getEvent();

    private:
    };
}
#include "MemoryLoggerOff.h"

#endif // SDLINPUTMANAGER_H
