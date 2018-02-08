#include <iostream>
#include "mini_regex.hpp"

void func(mini_regex::_regex_result& res)
{
    for (auto i : res.matched)
        std::cout << i << std::endl;
}

int main()
{
    mini_regex regex;

    regex.compile("\\ba(a|c|b|\\d)*.");
    regex.output_code();
    std::cout << "result: ";
    regex.match("daccaaafascda ac1cgg",func);
    std::cout << std::endl;

    regex.compile("do(es)?");
    regex.output_code();
    std::cout << "result: ";
    regex.match("doing",func);
    std::cout << std::endl;

    regex.compile("do(es)+");
    regex.output_code();
    std::cout << "result: ";
    regex.match("doesng",func);
    std::cout << std::endl;
    return 0;
}