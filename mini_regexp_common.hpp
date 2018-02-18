#ifndef MINI_REGEXP_COMMON_H_
#define MINI_REGEXP_COMMON_H_

#include <string>

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

    inline std::ptrdiff_t str2hex(std::string& s)
    {
        return 0;
    }

    inline std::ptrdiff_t str2oct(std::string& s)
    {
        return 0;
    }
}

#endif