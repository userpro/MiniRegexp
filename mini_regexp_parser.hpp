#ifndef MINI_REGEXP_PARSER_H_
#define MINI_REGEXP_PARSER_H_

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
         * Or_Stack => or栈 专门处理 ||
         * Val => MATCH栈(exp)
         * '(', ')' 两个栈都进入
         */
        std::vector<parse_stack_t> Parser_Stack;

        std::vector<ByteCode> Code; /* codegen */

    public:
        RE_Parser();
        bool parser(RE_Lexer& _lexer, RE_Config& config);
        void output_code();

    private:
        void parser_init();
        void parse_string(RE_Lexer& _lexer);
        void parse_any();
        void parse_plus();
        bool parse_exp();
        void parse_question();
        void parse_closure();
        
        int  parse_brace(RE_Lexer& _lexer, std::ptrdiff_t _index);
        bool parse_square_brace(std::string& exp);
        inline void val_push_string_tok(RE_Lexer& _lexer, std::ptrdiff_t _index);

    };
}
#endif