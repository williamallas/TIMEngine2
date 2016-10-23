#ifndef OPTION_H_INCLUDED
#define OPTION_H_INCLUDED

#include "Exception.h"

#include "MemoryLoggerOn.h"

namespace tim
{
namespace core
{

    template <class T>
    class Option
    {
    public:

        Option() : _hasValue(false) {}
        Option(const T& value) : _val(value), _hasValue(true) {}
        Option(const Option& option) : _hasValue(false) { *this = option; }
        ~Option()
        {
            if(_hasValue)
                _val.~T();
        }

#include "MemoryLoggerOff.h"
        Option& operator=(const Option& option)
        {
            if(_hasValue)
                _val.~T();

            _hasValue = option.hasValue();
            if(_hasValue)
                new (&_val) T(option.value());
            return *this;
        }
#include "MemoryLoggerOn.h"

        bool hasValue() const { return _hasValue; }

        operator bool() const { return _hasValue; }

        const T& value() const
        {
            if(!_hasValue)
                throw BadOptionAccess(typeid(T).name());

             return _val;
        }

    private:
        union { T _val; };
        bool _hasValue;
    };

}
}
#include "MemoryLoggerOff.h"

#endif // OPTION_H_INCLUDED
