#ifndef SPINLOCK_H_INCLUDED
#define SPINLOCK_H_INCLUDED

namespace tim
{
namespace core
{
    class NoMutex
    {
    public:
        NoMutex() = default;
        void lock(){}
        void unlock() {}
    };
}
}

#endif // SPINLOCK_H_INCLUDED
