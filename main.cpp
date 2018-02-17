#include <iostream>
#include <vector>
#include <string>
#include "mini_regexp.hpp"

void func(std::vector<std::string>& matched, std::vector<std::string>& sub_matched)
{
    std::cout << "Matched: " << std::endl;
    for (auto i : matched)
        std::cout << "  " << i;
    std::cout << std::endl;
    
    std::cout << "Sub Matched: " << std::endl;
    for (auto i : sub_matched)
        std::cout << "  " << i;
    std::cout << std::endl;
}

void test(std::string reg, std::string tar, std::function<void (std::vector<std::string>&, std::vector<std::string>&)> callback)
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
    // test("\\bworld.", "world world1", func); /* \b */
    // test("\\\\a", "wf\\asxc", func);
    test("do(es)+", "doesng", func); /* () */
    test("(a|b){1,2}", "abbb", func);
    // test("o+?", "oooo", func); /* ?(非)贪婪 */
    // test("a(o*?)d", "aoood", func); /* ?(非)贪婪 */
    // test("^abc", "abcabc", func); /* START */
    // test("abc$", "abc\nabc", func); /* MULTILINE END */
    // test("a.c", "a\nc", func); /* DOTALL */
    // test("[abc]*", "aaddab", func);     /* [] */
    // test("[^a-dh-mzn]", "axyawczmladddknwdnjkgnoiab", func);   /* [] */
    // test("\\d*", "12385567", func);
    // test("a.*b", "aaabaab", func);
    // test("^[0-9]*[1-9][0-9]*", "2sd134\n123sd2", func);
    // test("a{3,}b{1,2}d{1,22}e{11,12}f{1}", "aaaabbddeeeeeeeeeeeff", func);
    test("^([a-zA-Z0-9_\\-\\.]+)@((\\[[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.)|(([a-zA-Z0-9\\-]+\\.)+))([a-zA-Z]{2,4}|[0-9]{1,3})(\\]?)$", "john@johnmaddock.co.uk", func);
    // test("a{2,}?", "aaaaaaa", func); /* {}?懒惰模式 */
    // test("a*?", "aaba", func); /* *?懒惰模式 */
    // test("a+?", "aaaaa", func); /* +?懒惰模式 */
    // test("a??", "aaaaa", func); /* ??懒惰模式 */    
    test("(ab)*cd", "aaabcd", func);
    return 0;
}