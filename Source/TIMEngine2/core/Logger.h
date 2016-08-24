#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include <fstream>
#include <string>
#include <mutex>
#include "Exception.h"
#include "Singleton.h"

namespace tim
{
namespace core
{
    class Logger : Singleton<Logger>
    {
        friend class Singleton<Logger>;

    public:
        Logger(const std::string& name = "Log.txt") : _file(name, std::ios_base::out | std::ios_base::trunc)
        {
            if(!_file.is_open()) throw Exception("Unuable to open log file: "+name);
        }

        ~Logger() = default;

        template< class...Args >
        void log(Args... args)
        {
            inner_log(args...);
            if(_endl_mode) _file << std::endl;
            _mutex.unlock();
        }

        template< class...Args >
        void log2(Args... args)
        {
            _file << _src_file << "(" << _line << "): ";
            inner_log(args...);
            if(_endl_mode) _file << std::endl;
            _mutex.unlock();
        }

        Logger& setEndlMode(bool b) { _endl_mode = b; return *this; }
        Logger& setLoggerInfo(const char* str, int line)
        {
            _mutex.lock();
            _src_file = str;
            _line = line;
            return *this;
        }

    private:
            std::ofstream _file;
            bool _endl_mode = true;
            std::string _src_file;
            int _line;
            std::mutex _mutex;

            template< class Arg , class...Args >
            void inner_log(Arg a, Args... as)
            {
                _file << a;
                inner_log(as...);
            }


            void inner_log(){}
    };

    static Logger& logger = Singleton<Logger>::instance();

    #define LOG tim::core::logger.setLoggerInfo(__FILE__,__LINE__).setEndlMode(true).log
    #define LOG_EXT tim::core::logger.setLoggerInfo(__FILE__,__LINE__).setEndlMode(true).log2
    #define LOG_L tim::core::logger.setLoggerInfo(__FILE__,__LINE__).setEndlMode(false).log

#ifdef TIM_DEBUG
    #define DLOG tim::core::logger.setLoggerInfo(__FILE__,__LINE__).setEndlMode(true).log
    #define DLOG_EXT tim::core::logger.setLoggerInfo(__FILE__,__LINE__).setEndlMode(true).log2
    #define DLOG_L tim::core::logger.setLoggerInfo(__FILE__,__LINE__).setEndlMode(false).log
#else
    #define DLOG(...)
    #define DLOG_EXT(...)
    #define DLOG_L(...)
#endif


}
}

#endif // LOGGER_H_INCLUDED
