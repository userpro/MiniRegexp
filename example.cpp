#include <iostream>
#include "mini_regex.hpp"

void func(mini_regex::Result& res)
{
    for (auto i : res.matched)
        std::cout << i << std::endl;
}

int main()
{
    mini_regex regex;
    regex.compile("a|food");
    regex.match("food", func);
    return 0;
}