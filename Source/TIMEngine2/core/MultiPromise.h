#ifndef MULTIPROMISE_H
#define MULTIPROMISE_H

#include <future>
#include "type.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{
    template <class T>
    class MultiPromise
    {
    public:
        template<class TT> using future = std::future<TT>;
        template<class TT> using promise = std::promise<TT>;

        MultiPromise() = default;

        MultiPromise(MultiPromise&& p) : _promises(std::move(p._promises)), _isComplete(p._isComplete)
        {}

        ~MultiPromise()
        {
            complete(T());
        }

        MultiPromise& operator=(MultiPromise&& p)
        {
            _promises = std::move(p._promises);
            _isComplete = p._isComplete;
        }

        future<T> getFuture()
        {
            _promises.push_back(std::move(promise<T>()));
            return _promises.back().get_future();
        }

        void complete(const T& t)
        {
            if(_isComplete)
                return;

            _isComplete = true;
            for(size_t i=0 ; i<_promises.size() ; ++i)
                _promises[i].set_value(t);
        }

    private:
        vector<promise<T>> _promises;
        bool _isComplete=false;
    };
}
}
#include "MemoryLoggerOff.h"

#endif // MULTIPROMISE_H
