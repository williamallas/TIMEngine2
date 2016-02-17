#ifndef RAND_H_INCLUDED
#define RAND_H_INCLUDED

#include <boost/random/taus88.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include "Vector.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{

    class Rand
    {
    public:

        Rand(uint seed) { _generator.seed(seed); }
        float next_f() { return frand_range(_generator); }

        float next_f(const vec2& range)
        {
            if(range.x() >= range.y())
                return range.x();

            boost::random::uniform_real_distribution<> dis(range.x(), range.y());
            return dis(_generator);
        }

        size_t next_i() { return _generator(); }
        void setSeed(uint seed) { _generator.seed(seed); }

        /* Static */
        static float frand() { return frand_range(generator); }

        static float frand(const vec2& range)
        {
            if(range.x() >= range.y())
                return range.x();

            boost::random::uniform_real_distribution<> dis(range.x(), range.y());
            return dis(generator);
        }

        static size_t rand() { return generator(); }

        static int rand(const ivec2& range)
        {
            boost::random::uniform_int_distribution<> dis(range.x(), range.y());
            return dis(generator);
        }

        static void seed(uint seed) { generator.seed(seed); }

    private:
        boost::random::mt11213b _generator;

        static boost::random::taus88 generator;
        static boost::random::uniform_01<> frand_range;
    };

}
}
#include "MemoryLoggerOff.h"

#endif // RAND_H_INCLUDED
