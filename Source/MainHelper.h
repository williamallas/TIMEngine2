#ifndef MAINHELPER_H_INCLUDED
#define MAINHELPER_H_INCLUDED

#include "TIM_SDL/SDLInputManager.h"
#include "TIM_SDL/SDLTimer.h"
#include "TIM_SDL/SDLTextureLoader.h"

#include "MemoryLoggerOn.h"

static const uint WIN_RES_X = 1800;
static const uint WIN_RES_Y = 1000;

//static const uint WIN_RES_X = 900;
//static const uint WIN_RES_Y = 1000;


void initContextSDL();
void delContextSDL();
void swapBuffer();

template <int I, int N = 100>
float countTime(float time)
{
    static uint counter = 0;
    static float totalTime = 0;
    static float fps = 0;

    counter++;

    totalTime += time;
    if (counter % N == 0)
    {
        fps = totalTime / N;
        counter = 0;
        totalTime = 0;
        return fps;
    }
    return -fps;
}

#include "MemoryLoggerOff.h"

#endif // MAINHELPER_H_INCLUDED
