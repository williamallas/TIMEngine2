#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

#include "boost/thread/mutex.hpp"
#include "core/Rand.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{
    template <class Data, class Key, class LockPolicy, uint NB_SLOTS=16>
    class LinearCache
    {
    public:
        LinearCache()
        {
            for(int i=0 ; i<NB_SLOTS ; ++i)
                std::get<2>(_data[i]) = 0;
        }

        /* Lock unlock function */

        void lock(uint index) { std::get<3>(_data[index]).lock(); }

        bool searchLock(const Key& k, uint& index) const // return true if the key is found
        {
            for(uint i=0 ; i<NB_SLOTS ; ++i)
            {
                std::get<3>(_data[i]).lock();
                if(k == std::get<0>(_data[i]))
                {
                    index=i;
                    return true;
                }
                std::get<3>(_data[i]).unlock();
            }
            return false;
        }

        bool searchAvailableLock(uint& index) // if return false, the cache must be flush
        {
            for(uint i=0 ; i<NB_SLOTS ; ++i)
            {
                std::get<3>(_data[i]).lock();
                if(std::get<2>(_data[i]) == 0)
                {
                    index=i;
                    return true;
                }
                std::get<3>(_data[i]).unlock();
            }

            vector<uint> available;
            for(uint i=0 ; i<NB_SLOTS ; ++i)
            {
                std::get<3>(_data[i]).lock();
                if(std::get<2>(_data[i]) == 1)
                    available.push_back(i);
                else
                    std::get<3>(_data[i]).unlock();
            }

            if(available.empty())
                return false; // the cache need to be flush

            index=Rand::rand()%available.size();
            for(size_t i=0 ; i<available.size() ; ++i)
            {
                if(i != index)
                    std::get<3>(_data[available[i]]).unlock();
            }

            index = available[index];
            return true;
        }

        void unlock(uint index) const
        {
            std::get<3>(_data[index]).unlock();
        }

        /* if you have the lock you can use: */

        const Data& get(int i) const { return std::get<1>(_data[i]); }
        Data& get(int i) { return std::get<1>(_data[i]); }

        void upToDate(uint index) { std::get<2>(_data[index]) = 1; }
        void changed(uint index) { std::get<2>(_data[index]) = 2; }
        void setKey(uint index, const Key& k) { std::get<0>(_data[index]) = k; }

        const Key& getKey(uint index) const { return std::get<0>(_data[index]); }
        int getState(uint index) const { return std::get<2>(_data[index]); }

    private:
        mutable std::tuple<Key, Data, int, boost::mutex> _data[NB_SLOTS]; // key, data, state { 0:free 1:uptodate 2:changed }, lock
    };
}
}
#include "MemoryLoggerOff.h"

#endif // CACHE_H_INCLUDED
