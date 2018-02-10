#ifndef MINI_REGEXP_PARSER_H_
#define MINI_REGEXP_PARSER_H_

#include <stack>
#include <vector>
#include <algorithm>
#include <iostream>
#include "mini_regexp_token.hpp"
#include "mini_regexp_lexer.hpp"
#include "mini_regexp_code.hpp"
#include "mini_regexp_config.hpp"

#define CODE_ELM(a,b,c) ByteCode(a, reinterpret_cast<void*>(b), reinterpret_cast<void*>(c))

namespace mini_regexp_parser
{
    using namespace mini_regexp_lexer;
    using namespace mini_regexp_token;
    using namespace mini_regexp_code;
    using namespace mini_regexp_config;

    class RE_Parser
    {
    public:
        struct parse_stack_t
        {
            TOKEN tk;
            std::ptrdiff_t n;  /* 对应指令数 */
            std::ptrdiff_t ip; /* 首地址 */
            parse_stack_t():tk(TOKEN::ERR),n(0),ip(-1) {}
            parse_stack_t(TOKEN _tk, std::ptrdiff_t _n, std::ptrdiff_t _ip):tk(_tk),n(_n),ip(_ip) {}
        };
        /* 
         * Op => 操作栈
         * Val => MATCH栈 
         * '(', ')' 两个栈都进入
         */
        std::stack<parse_stack_t> Op, Val;

        std::vector<ByteCode> Code; /* codegen */

    public:
        RE_Parser();
        bool parser(RE_Lexer& _lexer, RE_Config& config);
        void output_code();

    private:
        void parser_init();
        bool parse_or();

    };
}
#endif