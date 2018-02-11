#include <iostream>
#include "mini_regexp.hpp"

void func(mini_regexp::Result& res)
{
    std::cout << "Result: " << std::endl;
    for (auto i : res.matched)
        std::cout << "  " << i;
    std::cout << std::endl;
}

void test(std::string reg, std::string tar, std::function<void (mini_regexp::Result&)> callback)
{
    static mini_regexp regex;
    regex.compile(reg);
    regex.output_code();
    std::cout << "Target: " << tar << std::endl;
    regex.match(tar,func);
    std::cout << std::endl;
}

int main()
{
    test("a(o0|c|b|\\d)*.", "daccaaafao0da ac1cgg", func);
    test("\\bworld", "world world", func);
    test("do(es)+", "doesng", func);
    test("(a|b)*abb", "abbb", func);
    test("o+?", "oooo", func);
    test("a(o*?)d", "aoood", func);
    test("^abc", "abcabc", func);
    test("abc$", "abc\nabc", func);
    test("ab{1,3}", "abcbabababab", func);
    return 0;
}