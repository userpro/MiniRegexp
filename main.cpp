#include <iostream>
#include <vector>
#include <string>
#include "mini_regexp.hpp"

void func(std::vector<std::string>& res)
{
    std::cout << "Result: " << std::endl;
    for (auto i : res)
        std::cout << "  " << i;
    std::cout << std::endl;
}

void test(std::string reg, std::string tar, std::function<void (std::vector<std::string>&)> callback)
{
    std::cout << "=======================" << std::endl;
    static mini_regexp regex;
    regex.compile(reg);
    regex.output_code();
    std::cout << "Target: " << tar << std::endl;
    regex.match(tar,func);
    std::cout << std::endl;
}

int main()
{
    test("a(c|b|o\\d)*.", "daccaaafao0da ac1cgg", func); /* () | * \d */
    test("\\bworld", "world world", func); /* \b */
    test("\\a", "dwf\\asxc", func);
    test("do(es)+", "doesng", func); /* () */
    test("(a|b){1,2}", "abbb", func);
    test("o+?", "oooo", func); /* ?(非)贪婪 */
    test("a(o*?)d", "aoood", func); /* ?(非)贪婪 */
    test("^abc", "abcabc", func); /* START */
    test("abc$", "abc\nabc", func); /* MULTILINE END */
    test("a.c", "a\nc", func); /* DOTALL */
    test("[abc]*", "aaddab", func);     /* [] */
    test("[^a-dh-mzn]", "axyawczmladddknwdnjkgnoiab", func);   /* [] */
    test("\\d*", "12385567", func);
    test("a.*b", "aaabaab", func);
    test("^[0-9]*[1-9][0-9]*", "2sd134\n123sd2", func);
    test("a{3,}b{1,2}d{1,22}e{11,12}f{1}", "aaaabbddeeeeeeeeeeeff", func);
    return 0;
}