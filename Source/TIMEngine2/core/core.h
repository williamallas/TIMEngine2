#ifndef CORE_H_INCLUDED
#define CORE_H_INCLUDED

#include "Vector.h"
#include "Logger.h"
#include "MemoryLogger.h"

#define RENDERABLE_OBJECT_NB_LOD 4

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
