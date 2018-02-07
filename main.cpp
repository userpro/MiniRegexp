#include <iostream>
#include "mini_regex.hpp"

void func(mini_regex::_regex_result& res)
{
    std::cout << res.matched << std::endl;
}

int main()
{
    mini_regex regex;
    regex.compile("a(a|c|b)*");
    // regex.compile("a*|b|c");
    // regex.compile("aa*");
    regex.output_code();
    regex.match("dacca",func);
    return 0;
}