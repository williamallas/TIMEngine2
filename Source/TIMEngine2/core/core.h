#ifndef CORE_H_INCLUDED
#define CORE_H_INCLUDED

#include "Vector.h"
#include "Logger.h"
#include "MemoryLogger.h"

#define RENDERABLE_OBJECT_NB_LOD 4

#define TIM_DEBUG_PROFILE

#ifdef TIM_DEBUG_PROFILE
#define PROFILE(a) boost::timer::auto_cpu_timer(a + std::string(" : %w s\n")),
#else
void noneFun() {}
#define PROFILE(a) noneFun(),
#endif

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{
    void init();
    void quit();
}
}

#include "MemoryLoggerOff.h"

#endif // CORE_H_INCLUDED
