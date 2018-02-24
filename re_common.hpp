#ifndef MINI_REGEXP_COMMON_H_
#define MINI_REGEXP_COMMON_H_

#include <string>
#include <algorithm>

namespace mini_regexp_common
{
    inline int is_ANY(char c, bool dotall = false)
    {
        return dotall ? 1 : (c != '\n');
    }

    inline int is_range_in(char c, char start, char end)
    {
        return (c - start >= 0 && end - c >= 0);
    }

    inline std::ptrdiff_t str2int(std::string& s)
    {
        return std::stoll(s);
    }

    inline std::ptrdiff_t __str2hex(char c)
    {
        if (is_range_in(c, 'a', 'z'))
            return c - 'a' + 10;
        else
            return c - '0';
    }

    inline std::ptrdiff_t str2hex(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        std::ptrdiff_t num = 0;
        for (int i = 0; i < s.length(); ++i)
            num = num * 16 + __str2hex(s[i]);
        return num;
    }

    inline std::ptrdiff_t str2oct(std::string& s)
    {
        std::ptrdiff_t num = 0;
        for (int i = 0; i < s.length(); i++)
            num = num * 8 + s[i] - '0';
        return num;
    }

    inline std::ptrdiff_t str_get_digit(const std::string& s, std::ptrdiff_t _index, std::string& num)
    {
        auto _start = _index;
        while (is_range_in(s[_index++], '0', '9'))
            ;
        num = s.substr(_start, _index - _start);
        return _index - _start;
    }
}

#endif