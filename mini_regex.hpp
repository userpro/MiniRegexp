#ifndef _MINI_REGEX_H_
#define _MINI_REGEX_H_

/*
Support:
    a       Single char
    a|b     Or
    ab      Concat
    a*      Closure
    (a)     Tuple

*/

#include <stack>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <iostream>

#define CODE_TYPE std::pair<BYTE_CODE, std::pair<std::ptrdiff_t*, std::ptrdiff_t*> >
#define CODE_ELM(a,b,c) std::make_pair(a, std::make_pair(reinterpret_cast<std::ptrdiff_t*>(b), reinterpret_cast<std::ptrdiff_t*>(c)))

class mini_regex
{
public:
    struct regex_result
    {
        size_t begin, end;
        const std::string matched;
    };
    
    mini_regex();

    bool compile(const std::string& regexp_str);
    bool match(const std::string& match_str, std::function<void(regex_result&)> callback = nullptr);
    
    void output_code();

private:
    /* addr is a current ip offset */
    enum BYTE_CODE {
        HALT = 0,
        MATCH,  /* match addr */
        SPLIT,  /* split addr1, addr2  if (addr1) else (addr2)   */
        JMP,    /* jmp addr */
        ACCEPT,
    };

    enum TOKEN
    {
        ERR,        /* Error */
        TEXT,       /* String */
        CLOSURE,    /* ab* */
        OR,         /* a|b */
        LBRACKET,
        RBRACKET,
        EXP,        /* stack parse */
    };

    
    std::string regexp;
    std::string target;
    
    std::vector<TOKEN> token;       /* lexer */
    std::vector<CODE_TYPE> code;    /* codegen */
    std::vector<std::string> text;  /* TEXT (a|b)* => store a, b */

    struct parse_stack_t
    {
        TOKEN tk;
        std::ptrdiff_t n; /* 对应指令数 */
        std::ptrdiff_t ip; /* 首地址 */
        parse_stack_t():tk(TOKEN::ERR),n(0),ip(-1) {}
        parse_stack_t(TOKEN _tk, std::ptrdiff_t _n, std::ptrdiff_t _ip):tk(_tk),n(_n),ip(_ip) {}
    };
    /* S1=>op S2=>TEXT or EXP */
    std::stack<parse_stack_t> S1, S2;        /* parser */

    void _reset();

    void lexer();
    bool parse();
    bool parse_or();
    bool evalute();
    
};

#endif