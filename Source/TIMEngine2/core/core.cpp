#include "core.h"
#include "Rand.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{

void init()
{
    std::freopen("err.txt", "w+", stderr);
    Rand::seed(time(0));

    static_assert(sizeof(sbyte)==1, "Assertion type failed.");
    static_assert(sizeof(ubyte)==1, "Assertion type failed.");
    static_assert(sizeof(real)==4, "Assertion type failed.");
    static_assert(sizeof(integer)==4, "Assertion type failed.");
    static_assert(sizeof(uint)==4, "Assertion type failed.");
}

void quit()
{
    MemoryLogger::instance().printLeak();
    free(&MemoryLogger::instance());
}

}
}
