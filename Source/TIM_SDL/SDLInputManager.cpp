#include "SDLInputManager.h"

#include "MemoryLoggerOn.h"
namespace tim
{

SDLInputManager::SDLInputManager() : InputManager()
{

}

SDLInputManager::~SDLInputManager()
{

}

void SDLInputManager::getEvent()
{
    for(auto& key : _keyState)
        key.second.firstPress = key.second.firstRelease = false;

    for(auto& key : _physicalState)
        key.second.firstPress = key.second.firstRelease = false;

    for(size_t i=0 ; i<3 ; ++i)
        _mouseState[i].firstPress = _mouseState[i].firstRelease = false;

    SDL_Event event;
    _mouseRel = {0,0};
    _mouseWheel = {0,0};
    _quit = false;

	while(SDL_PollEvent(&event))
	{
		switch (event.type)
		{
            case SDL_KEYDOWN:
                _keyState[event.key.keysym.sym]=STATE_PRESS;
                _physicalState[event.key.keysym.scancode]=STATE_PRESS;
            break;
            case SDL_KEYUP:
                _keyState[event.key.keysym.sym]=STATE_RELEASE;
                _physicalState[event.key.keysym.scancode]=STATE_RELEASE;
            break;

            case SDL_MOUSEMOTION:
                _mousePos = {event.motion.x,event.motion.y};
                _mouseRel = {event.motion.xrel, event.motion.yrel};
            break;
            case SDL_MOUSEBUTTONDOWN:
                _mouseState[event.button.button-1]=STATE_PRESS;
                _mousePos={event.button.x, event.button.y};
            break;

            case SDL_MOUSEBUTTONUP:
                _mouseState[event.button.button-1]=STATE_RELEASE;
                _mousePos={event.button.x, event.button.y};
            break;
            case SDL_MOUSEWHEEL:
                _mouseWheel = {event.wheel.x, event.wheel.y};
            break;

            case SDL_QUIT:
                _quit=true;
            break;
		}
	}
}

}
