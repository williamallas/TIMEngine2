#ifndef MULTIPROMISE_H
#define MULTIPROMISE_H

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

        MultiPromise(MultiPromise&& p) : _promises(std::move(p._promises)), _isComplete(p._isComplete)
        {}

        virtual ~MultiPromise()
        {
            complete(T());
        }

        MultiPromise& operator=(MultiPromise&& p)
        {
            _promises = std::move(p._promises);
            _isComplete = p._isComplete;
        }

        boost::unique_future<T> future()
        {
            _promises.push_back(std::move(boost::promise<T>()));
            return _promises.back().get_future();
        }

        void complete(const T& t)
        {
            if(_isComplete)
                return;

            _isComplete = true;
            for(size_t i=0 ; i<_promises.size() ; ++i)
                _promises[i].complete(t);
        }

    private:
        vector<boost::promise<T>> _promises;
        bool _isComplete=false;
    };
}
}
#include "MemoryLoggerOff.h"

#endif // MULTIPROMISE_H
