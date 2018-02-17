#ifndef MINI_REGEXP_COMMON_H_
#define MINI_REGEXP_COMMON_H_

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
}

#endif