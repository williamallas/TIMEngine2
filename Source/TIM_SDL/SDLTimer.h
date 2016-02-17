#ifndef SDLTIMER_H_INCLUDED
#define SDLTIMER_H_INCLUDED

#include <SDL.h>
#include "core/core.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    class SDLTimer
    {
    public:
        SDLTimer()
        {
            reset();
        }

        float elapsed() const
        {
            float res = (float)(SDL_GetPerformanceCounter()-_counter)*1000 / SDL_GetPerformanceFrequency();
            return ((int)(res*100))/100.0;
        }

        void reset()
        {
            _counter = SDL_GetPerformanceCounter();
        }



    private:
        Uint64 _counter;
    };

}
#include "MemoryLoggerOff.h"

#endif // SDLTIMER_H_INCLUDED
