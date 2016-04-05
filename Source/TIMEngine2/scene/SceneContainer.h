#ifndef SCENECONT_H_INCLUDED
#define SCENECONT_H_INCLUDED

#include <typeindex>
#include <boost/unordered_map.hpp>
#include <algorithm>
#include <iterator>

#include "core.h"

#include "MemoryLoggerOn.h"
namespace tim
{
    using namespace core;
namespace scene
{
    template <class Contained>
    class SceneContainer
    {
    public:
        SceneContainer() = default;

        ~SceneContainer()
        {
            std::for_each(std::begin(_container), std::end(_container), [](Contained* c) { delete c; });
        }

        template<class SuperType, class... Args>
        SuperType& add(Args... args)
        {
            SuperType* obj = new SuperType(args...);
            _container.push_back(obj);

            vector<Contained*>& cvec = _typeContainer[std::type_index(typeid(SuperType))];
            cvec.push_back(obj);

            _container.back()->_containerInfo = {this, _container.size()-1, cvec.size()-1 };

            return *obj;
        }

        void remove(Contained& obj)
        {
            if(obj._containerInfo.container != this)
                return;

            TransformableInfo& info = obj._containerInfo;

            if(info.indexInContainer+1 != _container.size())
            {
                _container[info.indexInContainer] = _container.back();
                _container[info.indexInContainer]->_containerInfo.indexInContainer = info.indexInContainer;
            }
            _container.pop_back();

            vector<Contained*>& typedVec = _typeContainer[std::type_index(typeid(obj))];
            if(info.indexInTypedVector+1 != typedVec.size())
            {
                typedVec[info.indexInTypedVector] = typedVec.back();
                typedVec[info.indexInTypedVector]->_containerInfo.indexInTypedVector = info.indexInTypedVector;
            }
            typedVec.pop_back();

            delete &obj;
        }

        template<class Type, class Predicat, class Collector>
        typename std::enable_if<!std::is_same<Type, Contained>::value>::type
            query(Predicat f, Collector collector)
        {
            auto it = _typeContainer.find(std::type_index(typeid(Type)));

            if(it == std::end(_typeContainer)) return;

            for(size_t i=0 ; i<it->second.size() ; ++i)
            {
                if(f(static_cast<const Type&>(*(it->second[i]))))
                    collector(static_cast<Type&>(*(it->second[i])));
            }
        }

        template<class Type, class Predicat, class Collector>
        typename std::enable_if<std::is_same<Type, Contained>::value>::type
            query(Predicat f, Collector collector)
        {
            std::copy_if(std::begin(_container), std::end(_container), collector, f);
        }

        struct TransformableInfo
        {
            SceneContainer* container;
            uint indexInContainer;
            uint indexInTypedVector;
        };

    protected:
        vector<Contained*> _container;
        boost::unordered_map<std::type_index, vector<Contained*>> _typeContainer;
    };

}
}
#include "MemoryLoggerOff.h"


#endif // SCENECONT_H_INCLUDED
