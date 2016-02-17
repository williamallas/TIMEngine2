#ifndef EXCEPTION_H_INCLUDED
#define EXCEPTION_H_INCLUDED

#include <string>
#include <exception>

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{
    class Exception : public std::exception
    {
    public:
        Exception(const std::string& s) { _msg = s; }
        virtual ~Exception() throw() {}

        virtual const char* what() const throw()
        {
            return _msg.c_str();
        }

    private:
        std::string _msg;
    };

    class BadDealloc : public Exception
    {
    public:
        BadDealloc(const std::string& s) : Exception(s){}
    };

    class BadOptionAccess : public Exception
    {
    public:
        BadOptionAccess() : Exception("Bad option access, option doesn't have a value."){}
    };

    class BadRefCounter : public Exception
    {
    public:
        BadRefCounter() : Exception("Bad Ref counter decreasment. (<0)"){}
    };

    class BadResourceDelete : public Exception
    {
    public:
        BadResourceDelete() : Exception("Delete resource with counter > 0"){}
    };

    class AllocationFailedException : public Exception
    {
    public:
        AllocationFailedException() : Exception("Failed to alloc the requested memory, probably not enough space.") {}
    };
}
}
#include "MemoryLoggerOff.h"

#endif // EXCEPTION_H_INCLUDED
