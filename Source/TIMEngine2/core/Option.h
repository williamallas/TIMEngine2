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

        Option() { _hasValue=false; }
        Option(const T& value) { _val=value; _hasValue = true; }
        Option(const Option& option) { *this = option; }
        virtual ~Option()
        {
            if(_hasValue)
                _val.~T();
        }

        Option& operator=(const Option& option)
        {
            _hasValue = option.hasValue();
            if(_hasValue)
                _val = option.value();
            return *this;
        }

        bool hasValue() const { return _hasValue; }

        const T& value() const
        {
            if(!_hasValue)
                throw BadOptionAccess();

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
