#include <iostream>
#include "mini_regexp.hpp"

void func(mini_regexp::Result& res)
{
    for (auto i : res.matched)
        std::cout << i << std::endl;
}

int main()
{
    mini_regexp regex;
    regex.compile("a|food");
    regex.match("food", func);
    return 0;
}