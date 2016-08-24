#ifndef DEVICEFUN_H_INCLUDED
#define DEVICEFUN_H_INCLUDED

#ifdef USE_SDL
#include <SDL.h>
#endif

namespace tim
{
namespace renderer
{

#ifdef USE_SDL
    using ThreadID = SDL_threadID;
#endif

    inline ThreadID getThreadId()
    {
    #ifdef USE_SDL
        return SDL_ThreadID();
    #endif
    }
}
}

#endif //DEVICEFUN_H_INCLUDED
