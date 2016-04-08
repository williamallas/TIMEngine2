#include "StringUtils.h"

#include "MemoryLoggerOn.h"
namespace tim
{
namespace core
{

bool StringUtils::toBool() const
{
    if(_str=="true" || _str=="yes" || _str=="1")
        return true;
    else return false;
}

StringUtils & StringUtils::toLower()
{
    for(size_t i=0; i < _str.size(); ++i)
        _str[i]=tolower(_str[i]);
    return *this;
}

StringUtils & StringUtils::toUpper()
{
    for(size_t i=0; i < _str.size(); ++i)
        _str[i]=toupper(_str[i]);
    return *this;
}

bool StringUtils::isNumber() const
{
    std::istringstream iss(_str);
    float f;
    iss >> f;
    return iss.eof() && !iss.fail();
}

vector<std::string> StringUtils::splitWord(char splitChar) const
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

std::string StringUtils::readFile(const std::string& file)
{
    std::ifstream ff(file);
    std::string line, total;

    while(std::getline(ff, line))
        total += line+"\n";

    return total;
}

std::string StringUtils::str(const char* str)
{
    if(str) return std::string(str);
    else return std::string();
}

std::string StringUtils::extension() const
{
    if(_str.empty())
        return "";

    std::string buf;
    for(size_t i=_str.size() ; i>0 ; --i)
    {
        if(_str[i-1] == '.')
            return std::string(buf.rbegin(), buf.rend());

        buf += char(tolower(_str[i-1]));
    }
    return "";
}

}
}
