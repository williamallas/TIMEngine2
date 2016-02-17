#ifndef STRINGUTILS_H_INCLUDED
#define STRINGUTILS_H_INCLUDED

#include <string>
#include <sstream>
#include <fstream>

#include "MemoryLogger.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{
    class StringUtils
    {
    public:
        friend std::ostream& operator<< (std::ostream& stream, const StringUtils& t) { stream << t.str(); return stream;}

        static std::string readFile(const std::string&);

        static std::string str(const char*);

        StringUtils();

        template<class T>
        StringUtils(const T& val);

        int toInt() const;
        float toFloat() const;
        double toDouble() const;
        bool toBool() const;

        std::string& str();

        bool isNumber() const;

        StringUtils& toLower();
        StringUtils& toUpper();

        vector<std::string> splitWord(char splitChar = ' ') const;

        std::string str() const;
        operator std::string() const;

    private:
        std::string _str;

    };

    template<class T>
    StringUtils::StringUtils(const T& val)
    {
        std::stringstream os;
        os << val;
        _str = os.str();
    }
}
}


/** Inline implementation */
#include <locale>

using namespace tim::core;

inline StringUtils::StringUtils() { }

inline int StringUtils::toInt() const { int i=0; std::stringstream(_str)>>i; return i; }
inline float StringUtils::toFloat() const { float i=0; std::stringstream(_str)>>i; return i; }
inline double StringUtils::toDouble() const { double i=0; std::stringstream(_str)>>i; return i; }
inline std::string& StringUtils::str() {  return _str; }
inline std::string StringUtils::str() const { return _str; }
inline StringUtils::operator std::string() const { return _str; }

inline bool StringUtils::toBool() const
{
    if(_str=="true" || _str=="yes" || _str=="1")
        return true;
    else return false;
}

inline StringUtils & StringUtils::toLower()
{
    for(size_t i=0; i < _str.size(); ++i)
        _str[i]=tolower(_str[i]);
    return *this;
}

inline StringUtils & StringUtils::toUpper()
{
    for(size_t i=0; i < _str.size(); ++i)
        _str[i]=toupper(_str[i]);
    return *this;
}

inline bool StringUtils::isNumber() const
{
    std::istringstream iss(_str);
    float f;
    iss >> f;
    return iss.eof() && !iss.fail();
}

inline vector<std::string> StringUtils::splitWord(char splitChar) const
{
    std::string cpyStr = _str;
    for(size_t i=0 ; i<cpyStr.size() ; ++i)
    {
        if(cpyStr[i] == splitChar)
            cpyStr[i] = ' ';
    }

    std::istringstream iss(cpyStr);
    vector<std::string> res;
    while(!iss.eof())
    {
        std::string str;
        iss >> str;
        if(!str.empty())
            res.push_back(std::move(str));
    }
    return res;
}

inline std::string StringUtils::readFile(const std::string& file)
{
    std::ifstream ff(file);
    std::string line, total;

    while(std::getline(ff, line))
        total += line+"\n";

    return total;
}

inline std::string StringUtils::str(const char* str)
{
    if(str) return std::string(str);
    else return std::string();
}

#include "MemoryLoggerOff.h"

#endif // STRINGUTILS_H_INCLUDED
