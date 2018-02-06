#include <iostream>
#include "mini_regex.hpp"

int main()
{
    mini_regex regex;
    // regex.compile("(a|c|b)*");
    regex.compile("a*|b|c");
    regex.output_code();
    std::cout << regex.match("aabacca") << std::endl;
    return 0;
}