#include <iostream>
#include <vector>
#include <string>
#include <ctime>
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
    std::cout << "Target: " << std::endl << "  " << tar << std::endl;
    regex.match(tar,func);
    std::cout << std::endl;
}

int main()
{
    clock_t t;
    t = clock();

    std::string test_str[][2] = {
        {"a(c|b|o\\d)*.", "daccaaafao0da ac1cgg"},  /* () | * \d */
        {"\\bworld."    , "world world1"        },  /* \b */
        {"\\\\a"        , "wf\\asxc"            },
        {"do(es)+"      , "doesng"              },  /* () */
        {"(a|b){1,2}"   , "abbb"                },
        {"a(c|b|o\\d)*.", "daccaaafao0da ac1cgg"},
        {"\\bworld."    , "world world1"        },
        {"\\\\a"        , "wf\\asxc"            },
        {"do(es)+"      , "doesng"              },
        {"(a|b){1,2}"   , "abbb"                },
        {"o+?"          , "oooo"                }, /* ?(非)贪婪 */
        {"a(o*?)d"      , "aoood"               }, /* ?(非)贪婪 */
        {"^abc"         , "abcabc"              }, /* START */
        {"abc$"         , "abc\nabc"            }, /* MULTILINE END */
        {"a.c"          , "a\nc"                }, /* DOTALL */
        {"[abc]*"       , "aaddab"              },     /* [] */
        {"[^a-dh-mzn]"  , "axyawczmladddknwdnjkgnoiab"},   /* [] */
        {"\\d*"         , "12385567"            },
        {"a.*b"         , "aaabaab"             },
        {"^[0-9]*[1-9][0-9]*", "2sd134\n123sd2" },
        {"a{3,}b{1,2}d{1,22}e{11,12}f{1}", "aaaabbddeeeeeeeeeeeff"},
        {
            "^([a-zA-Z0-9_\\-\\.]+)@((\\[[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.)|(([a-zA-Z0-9\\-]+\\.)+))([a-zA-Z]{2,4}|[0-9]{1,3})(\\]?)$", 
            "john@johnmaddock.co.uk"
        },
        {"a{2,}?"       , "aaaaaaa"             }, /* {}?懒惰模式 */
        {"a*?"          , "aaba"                }, /* *?懒惰模式 */
        {"a+?"          , "aaaaa"               }, /* +?懒惰模式 */
        {"a??"          , "aaaaa"               }, /* ??懒惰模式 */    
        {"(ab)*cd"      , "aaabcd"              },
        {"(2)(2)\\1\\2" , "22222"               },
        {"\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}", "192.168.0.255"},
        {"\\u00a9\\x41\\010111", "©AA11"        },
        {"(?:as|bd)vv"  , "asvv"                },
        {"(?<=95|98|NT|2000)Windows.", "2000Windowsa 3.1Windowsb"},
        {"(?<!95|98|NT|2000)Windows.", "3.1Windowsc2000Windowsa 3.1Windowsb"},
        {"industr(?:y|ies)", "industries"       },
        // {"^([a-z0-9_\\.-]+)@([\\da-z\\.-]+)\\.([a-z\\.]{2,6})$"}

        /* ERROR empty loop! dead loop! */
        // {"(a*)*"        , "aaaaaaaaaaaaaaaaaaaaaaaaaaaab"},
    };

    for (int i = 0; i < sizeof(test_str)/sizeof(test_str[0]); i++)
        test(test_str[i][0], test_str[i][1], func);


    t = clock() - t;
    std::cout << "It cost " << t << " clicks (" << ((float)t)*1000/CLOCKS_PER_SEC << " ms)." << std::endl;
    return 0;
}