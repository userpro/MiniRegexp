#ifndef _MINI_REGEX_H_
#define _MINI_REGEX_H_

#include <stack>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <iostream>

#define CODE_ELM(a,b,c) ByteCode(a, reinterpret_cast<void*>(b), reinterpret_cast<void*>(c))

class mini_regex
{
public:
    struct _regex_result {
        size_t begin, end;
        std::string matched;
    } regex_result;
    
    mini_regex();

    bool compile(const std::string& regexp_str);
    bool match(const std::string& match_str, std::function<void(_regex_result&)> callback = nullptr);
    
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
        _CHAR,      /* Char */
        CLOSURE,    /* ab* */
        OR,         /* a|b */
        LBRACKET,
        RBRACKET,
        EXP,        /* stack parse */
    };

    struct ByteCode
    {
        BYTE_CODE op;
        void *exp1, *exp2;
        ByteCode():op(BYTE_CODE::HALT),exp1(0),exp2(0) {}
        ByteCode(BYTE_CODE _op, void *_exp1, void *_exp2):exp1(_exp1),exp2(_exp2),op(_op) {}
    };

    
    std::string regexp;
    std::string target;
    
    std::vector<TOKEN> Token;       /* lexer */
    std::vector<ByteCode> Code;     /* codegen */
    std::vector<char> Text;  /* TEXT (a|b)* => store a, b */

    struct parse_stack_t
    {
        TOKEN tk;
        std::ptrdiff_t n;  /* 对应指令数 */
        std::ptrdiff_t ip; /* 首地址 */
        parse_stack_t():tk(TOKEN::ERR),n(0),ip(-1) {}
        parse_stack_t(TOKEN _tk, std::ptrdiff_t _n, std::ptrdiff_t _ip):tk(_tk),n(_n),ip(_ip) {}
    };
    /* S1 => op 
     * S2 => (TEXT or EXP) 
     */
    std::stack<parse_stack_t> S1, S2;        /* parser */

    std::stack<std::ptrdiff_t> Eval;

    void _reset();

    void lexer();
    bool parse();
    bool parse_or();
    bool evalute();
    
};

#endif