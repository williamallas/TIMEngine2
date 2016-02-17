#include "Rand.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{
    boost::random::taus88 Rand::generator;
    boost::random::uniform_01<> Rand::frand_range;
}
}
