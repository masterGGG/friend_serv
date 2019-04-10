#ifndef STRING_UTILS_H_
#define STRING_UTILS_H_

#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::istringstream;
using std::stringstream;

namespace Common
{
    // 根据delim分解字符串
    void split(const std::string& s, char delim, std::vector<std::string>& elems);
    // 去除字符串s头尾的charlist字符
    void trim(std::string& s, const std::string& charlist);
    // 判断给定的字符串是否utf8编码
    bool is_utf8(const std::string& s);
    // 判断字符串是否全是数字
    bool is_all_digit(const std::string& s);
    // 字符串转换成数字类型，整型，浮点型等。
    template<typename T> void strtodigit(const string& str, T& out);

    // 只做简单转换，调用时要确保str的值。
    template<typename T> void strtodigit(const string& str, T& out)
    {
        stringstream ss;

        ss.clear();

        ss << str;
        ss >> out;
    }
}

#endif // LIBANT_STRING_UTILS_HPP_
