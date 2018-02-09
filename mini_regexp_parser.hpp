#ifndef MINI_REGEXP_PARSER_H_
#define MINI_REGEXP_PARSER_H_

#include <stack>
#include <vector>
#include <algorithm>
#include <iostream>
#include "mini_regexp_token.hpp"
#include "mini_regexp_lexer.hpp"
#include "mini_regexp_code.hpp"

#define CODE_ELM(a,b,c) ByteCode(a, reinterpret_cast<void*>(b), reinterpret_cast<void*>(c))

namespace mini_regexp_parser
{
    using namespace mini_regexp_lexer;
    using namespace mini_regexp_token;
    using namespace mini_regexp_code;

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
         * S1 => 操作栈
         * S2 => MATCH栈 
         * '(', ')' 两个栈都进入
         */
        std::stack<parse_stack_t> S1, S2; /* parser */

        std::vector<ByteCode> Code; /* codegen */

    public:
        RE_Parser() {}
        bool parser(RE_Lexer& _lexer)
        {
            parser_init();
            std::ptrdiff_t _index = 0, _len = _lexer.Token.size();
            std::reverse(_lexer.Text.begin(), _lexer.Text.end());

            while (_index < _len)
            {
                switch (_lexer.Token[_index])
                {
                    case TOKEN::STRING:
                    {
                        S2.push(parse_stack_t(TOKEN::STRING, 1, Code.size()));
                        Code.push_back(CODE_ELM(BYTE_CODE::MATCH, const_cast<char*>(_lexer.Text.back().data()), 0));
                        _lexer.Text.pop_back();
                        break;
                    }

                    /* '.' */
                    case TOKEN::ANY: 
                        S2.push(parse_stack_t(TOKEN::STRING, 1, Code.size()));
                        Code.push_back(CODE_ELM(BYTE_CODE::MATCH, TOKEN::ANY, 0)); /* -1 means match any */
                        break;

                    case TOKEN::DIGIT:
                        S2.push(parse_stack_t(TOKEN::STRING, 1, Code.size()));
                        Code.push_back(CODE_ELM(BYTE_CODE::MATCH, TOKEN::DIGIT, 0));
                        break;
                    
                    /* '+' */
                    case TOKEN::PLUS:
                    {
                        /* 后一位是'?' 开启非贪婪模式 */
                        if (_index + 1 < _len && _lexer.Token[_index + 1] == TOKEN::QUESTION) 
                        {
                            _index++;
                            break;
                        }
                        /*
                         * 0 exp
                         * 1 split -1,1
                         */
                        parse_stack_t exp = S2.top();
                        S2.pop();

                        Code.insert(Code.begin() + exp.ip + exp.n, CODE_ELM(BYTE_CODE::SPLIT, -exp.n, 1));

                        exp.n += 1;
                        exp.tk = TOKEN::EXP;
                        S2.push(exp);
                        break;
                    }
                    
                    /* '?' */
                    case TOKEN::QUESTION: 
                    {
                        /*
                         * 0 split 1,2
                         * 1 exp
                         * 2 ...
                         */
                        parse_stack_t exp = S2.top();
                        S2.pop();

                        Code.insert(Code.begin() + exp.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp.n + 1));

                        exp.n += 1;
                        exp.tk = TOKEN::EXP;
                        S2.push(exp);
                        break;
                    }

                    /* '[' */
                    case TOKEN::SQUARE_LBRACKET:
                        break;

                    /* ']' */
                    case TOKEN::SQUARE_RBRACKET:
                        break;

                    /* '{' */
                    case TOKEN::LBRACE:
                        break;

                    /* '}' */
                    case TOKEN::RBRACE:
                        break;
                    
                    /* '^' */
                    case TOKEN::BEGIN: 
                        break;
                    
                    /* '$' */
                    case TOKEN::END: 
                        break;

                    /* '|' */
                    case TOKEN::OR:
                        /* OR 延后处理 */
                        S1.push(parse_stack_t(TOKEN::OR, 2, -1));
                        break;

                    /* '*' */
                    case TOKEN::CLOSURE:
                    {
                        /* 后面是'?' 开启非贪婪模式 */
                        if (_index + 1 < _len && _lexer.Token[_index + 1] == TOKEN::QUESTION) break;
                        /* 
                         * 0 split 1 3 
                         * 1 exp(pst)
                         * 2 split -1 3
                         * 3 do...
                         */
                        parse_stack_t exp = S2.top();
                        S2.pop();

                        Code.insert(Code.begin() + exp.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp.n + 1 + 1));
                        Code.insert(Code.begin() + exp.ip + 1 + exp.n, CODE_ELM(BYTE_CODE::SPLIT, -exp.n, 1));

                        exp.n += 2;
                        exp.tk = TOKEN::EXP;
                        S2.push(exp);
                        break;
                    }

                    /* '(' */
                    case TOKEN::LBRACKET:
                        S1.push(parse_stack_t(TOKEN::LBRACKET, 0, -1));
                        S2.push(parse_stack_t(TOKEN::LBRACKET, 0, -1));
                        break;

                    /* ')' */
                    case TOKEN::RBRACKET:
                    {
                        while (S1.top().tk != TOKEN::LBRACKET)
                        {
                            if (S1.top().tk == TOKEN::OR)
                                parse_or(); /* OR处理 */
                            S1.pop();
                        }
                        if (!S1.empty()) S1.pop();
                        else std::cout << "mismatch parentheses." << std::endl;

                    }

                    case TOKEN::EXP: break;

                    default:
                        std::cout << "parse: err." << std::endl;
                        return false;
                }
                ++_index;
            }

            while (!S1.empty())
            {
                if (S1.top().tk == TOKEN::OR)
                    parse_or();
                S1.pop();
            }

            while (!S2.empty()) S2.pop();

            Code.push_back(CODE_ELM(BYTE_CODE::ACCEPT, 0, 0));
            return true;
        }

        void output_code()
        {
            std::cout << "Generate Code: " << std::endl;
            for (auto i : Code)
            {
                switch (i.op)
                {
                    case BYTE_CODE::SPLIT:
                        std::cout << "  SPLIT " << reinterpret_cast<std::ptrdiff_t>(i.exp1)\
                                << ", " << reinterpret_cast<std::ptrdiff_t>(i.exp2) << std::endl;
                        break;
                    case BYTE_CODE::MATCH:
                    {
                        auto exp_t = reinterpret_cast<std::ptrdiff_t>(i.exp1);
                        switch (exp_t)
                        {
                            case TOKEN::ANY:
                                std::cout << "  MATCH " << "ANY" << std::endl;
                                break;
                            case TOKEN::DIGIT:
                                std::cout << "  MATCH " << "DIGIT" << std::endl;
                                break;
                            case TOKEN::SPACE:
                                std::cout << "  MATCH " << "SPACE" << std::endl;
                                break;
                            default:
                                std::cout << "  MATCH " << (std::string)reinterpret_cast<const char*>(exp_t) << std::endl;
                        }
                        
                        break;
                    }
                    case BYTE_CODE::JMP:
                        std::cout << "  JMP " << reinterpret_cast<std::ptrdiff_t>(i.exp1) << std::endl;
                        break;
                    case BYTE_CODE::ACCEPT:
                        std::cout << "  ACCEPT" << std::endl;
                        break;
                    case BYTE_CODE::HALT:
                        std::cout << "  HALT" << std::endl;
                        break;
                    default:
                        std::cout << "  unknown unknown unknown" << std::endl;
                        break;
                }
            }
            std::cout << std::endl;
        }

    private:

        void parser_init()
        {
            Code.clear();
            while (!S1.empty()) S1.pop();
            while (!S2.empty()) S2.pop();
        }

        /* 
         * OR 处理:
         *      0 split 1 4 
         *      1 exp1
         *      2 jmp 4
         *      3 exp2
         *      4 do...
         */
        bool parse_or()
        {
            parse_stack_t s1_pst, exp1, exp2;
            s1_pst = S1.top();

            if (S2.size() < 2)
            {
                std::cout << "parse: '|' not enough Text." << std::endl;
                return false;
            }
            exp2 = S2.top(); S2.pop();
            exp1 = S2.top(); S2.pop();
            if (exp1.ip > exp2.ip)
                std::swap(exp1, exp2);
            s1_pst.ip = exp1.ip; /* update ip */

            /* insert JMP */
            exp2.ip++;
            Code.insert(Code.begin() + exp1.ip + exp1.n, CODE_ELM(BYTE_CODE::JMP, exp2.n + 1, 0));

            /*  insert SPLIT (+1) */
            exp1.ip++; exp2.ip++;
            Code.insert(Code.begin() + s1_pst.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp2.ip - s1_pst.ip));

            s1_pst.tk = TOKEN::EXP;
            s1_pst.n += (exp1.n + exp2.n);
            S2.push(s1_pst);

            return true;
        }

    };
}
#endif