#ifndef SPINLOCK_H_INCLUDED
#define SPINLOCK_H_INCLUDED

#include <boost/atomic.hpp>

namespace tim
{
namespace core
{
    class SpinLock
    {
    public:
        SpinLock() : _state(Unlocked) {}

        void lock()
        {
            while (_state.exchange(Locked, boost::memory_order_acquire) == Locked);
        }

        void unlock()
        {
            _state.store(Unlocked, boost::memory_order_release);
        }

    private:
        typedef enum {Locked, Unlocked} LockState;
        boost::atomic<LockState> _state;
    };
}
}

#endif // SPINLOCK_H_INCLUDED
