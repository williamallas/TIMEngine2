#ifndef ASSETMANAGER_H_INCLUDED
#define ASSETMANAGER_H_INCLUDED

#include "core/core.h"
#include "Asset.h"
#include "Singleton.h"
#include "SpinLock.h"
#include "AddYourLoader.h"

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

namespace tim
{
    using namespace core;
namespace resource
{
    template<class T>
    class AssetManager : public Singleton<AssetManager<T>>
    {
        friend class Singleton<AssetManager<T>>;

    public:
        template <bool async, typename... Args>
        Option<T> load(Args... args)
        {
            class StaticLoader : public Loader<Args...>, public AssetLoader<T>
            {
            public:
                StaticLoader() : Loader<Args...>(), AssetLoader<T>() {}
            };

            static StaticLoader in_loader;

            auto opt = in_loader.get(args...);
            if(opt) return opt.value();
            else
            {
                auto opt_dat = in_loader.template operator()<async>(args...);
                if(opt_dat)
                    in_loader.add(args..., opt_dat.value());

                return opt_dat;
            }
        }

        virtual ~AssetManager() { clear(); }

        uint nbLoaders() const { return _loaders.size(); }

        void clear()
        {
            std::for_each(_loaders.begin(), _loaders.end(), [](GenericLoader* l) { l->clear(); });
        }

    private:
        class GenericLoader
        {
        public:
            GenericLoader() { AssetManager::instance().addLoaders(this); }
            virtual ~GenericLoader() = default;

            virtual void clear() = 0;
        };

        vector<GenericLoader*> _loaders;
        SpinLock _lock;

        void addLoaders(GenericLoader* l)
        {
            boost::lock_guard<SpinLock> guard(_lock);
            _loaders.push_back(l);
        }

        template <typename... Args>
        class Loader : GenericLoader
        {
        public:
            Loader() : GenericLoader() {}
            virtual ~Loader() { clear(); }

            Option<T> get(Args... args) const
            {
                boost::lock_guard<SpinLock> guard(_lock);
                auto it = _assets.find(boost::make_tuple(args...));
                if(it == _assets.end()) return Option<T>();
                else return Option<T>(it->second);
            }

            void add(Args... args, const T& asset)
            {
                boost::lock_guard<SpinLock> guard(_lock);
                _assets[boost::make_tuple(args...)] = asset;
            }

            void clear() override
            {
                 boost::lock_guard<SpinLock> guard(_lock);
                _assets.clear();
            }

            //typename boost::container::map<boost::tuple<Args...>, T>::const_iterator begin() const { return _assets.begin(); }
            //typename boost::container::map<boost::tuple<Args...>, T>::const_iterator end() const { return _assets.end(); }

            boost::container::map<boost::tuple<Args...>, T> _assets;
            mutable SpinLock _lock;
        };
    };


}
}

#endif // ASSETMANAGER_H_INCLUDED
