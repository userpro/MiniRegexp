#ifndef MINI_REGEXP_PARSER_H_
#define MINI_REGEXP_PARSER_H_

#include <vector>
#include <algorithm>
#include <iostream>
#include "re_token.hpp"
#include "re_lexer.hpp"
#include "re_code.hpp"
#include "re_config.hpp"
#include "re_common.hpp"
#include "re_keywords.hpp"

#define CODE_ELM(a,b,c) ByteCode(a, reinterpret_cast<void*>(b), reinterpret_cast<void*>(c))

namespace mini_regexp_parser
{
    using namespace mini_regexp_lexer;
    using namespace mini_regexp_token;
    using namespace mini_regexp_code;
    using namespace mini_regexp_config;
    using namespace mini_regexp_common;
    using namespace mini_regexp_keywords;

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

        RE_Keywords mini_keywords;

    private:
        struct zero_width_assert_parse_t
        {
            TOKEN tk;
            zero_width_assert_parse_t(TOKEN _tk):tk(_tk) {}
        };
        std::vector<zero_width_assert_parse_t> ZeroWidthAssert_Parse_Stack;

    public:
        RE_Parser();
        bool parser(RE_Lexer& _lexer, RE_Config& config);
        void output_code();

    private:
        void parser_init();
        bool parse_main(RE_Lexer& _lexer, 
            std::ptrdiff_t _index, std::ptrdiff_t _len, 
            RE_Config& config);
        void parse_string(RE_Lexer& _lexer);
        void parse_any();
        bool parse_exp(); /* 子表达式和零宽断言 */
        void parse_plus(bool greedy_mode = true);
        void parse_question(bool greedy_mode = true);
        void parse_closure(bool greedy_mode = true);
        bool parse_brace(std::string& exp, bool greedy_mode = true);
        bool parse_square_brace(std::string& exp);
        std::ptrdiff_t parse_square_escape_char(
            std::string& s, std::ptrdiff_t _index, std::string& res);
        void parse_group(RE_Lexer& _lexer);
        void parse_zero_width_assert();
    };
}
#endif