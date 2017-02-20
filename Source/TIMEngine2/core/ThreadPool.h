#ifndef THREADPOOL_H_INCLUDED
#define THREADPOOL_H_INCLUDED

#include <boost/shared_ptr.hpp>
#include <boost/threadpool.hpp>
#include "Singleton.h"
#include <future>

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{
    class ThreadPool : boost::noncopyable
    {
    public:
        ThreadPool() { _pool = new boost::threadpool::pool(boost::thread::hardware_concurrency()); }
        ThreadPool(size_t poolSize) { _pool = new boost::threadpool::pool(poolSize); }
        ~ThreadPool() { delete _pool; }

        template <class T>
        ThreadPool& schedule(const T& task)
        {
            boost::mutex::scoped_lock(_mutex);
            _pool->schedule(task);
            return *this;
        }

        template <class TaskType>
        std::future<decltype((*((TaskType*)nullptr))())> schedule_trace(const TaskType& task)
        {
            boost::mutex::scoped_lock(_mutex);
            boost::shared_ptr<std::promise<decltype((*((TaskType*)nullptr))())>> prom(new std::promise<decltype((*((TaskType*)nullptr))())>());
            _pool->schedule([=](){prom->set_value(task());});
            return prom->get_future();

        }

        bool empty() const;
        size_t pending() const;
        size_t active() const;

        void wait(size_t threshold=0) const;

    private:
        boost::threadpool::pool* _pool;
        mutable boost::mutex _mutex;
    };

    inline bool ThreadPool::empty() const { boost::mutex::scoped_lock(_mutex); return _pool->empty(); }
    inline size_t ThreadPool::pending() const { boost::mutex::scoped_lock(_mutex); return _pool->pending(); }
    inline size_t ThreadPool::active() const { boost::mutex::scoped_lock(_mutex); return _pool->active(); }

    inline void ThreadPool::wait(size_t threshold) const { boost::mutex::scoped_lock(_mutex); _pool->wait(threshold); }
}
}
#include "MemoryLoggerOff.h"

#endif // THREADPOOL_H_INCLUDED
