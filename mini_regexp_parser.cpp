#ifndef MINI_REGEXP_PARSER_CPP_
#define MINI_REGEXP_PARSER_CPP_

#include "mini_regexp_parser.hpp"

using namespace mini_regexp_parser;

RE_Parser::RE_Parser() {}

bool RE_Parser::parser(RE_Lexer& _lexer, RE_Config& config)
{
    parser_init();

    bool _begin = false, _end = false;
    std::ptrdiff_t _index = 0, _len = _lexer.Token.size();
    std::reverse(_lexer.Text.begin(), _lexer.Text.end());

    while (_index < _len)
    {
        switch (_lexer.Token[_index])
        {
            case TOKEN::STRING: parse_string(_lexer); break;
            case TOKEN::ANY:    parse_any();          break; /* '.' */
            /* '+' */
            case TOKEN::PLUS: { parse_plus(); break; }
            /* '?' */
            case TOKEN::QUESTION: { parse_question(); break; }
            /* '[' */
            case TOKEN::SQUARE_LBRACKET:
            {
                parse_square_brace(_lexer.Text.back());
                _lexer.Text.pop_back();
                _index += 2;
                break;
            }
            /* ']' */
            case TOKEN::SQUARE_RBRACKET: break;
            /* '{' 'string' '}' */
            case TOKEN::LBRACE: 
            {
                parse_brace(_lexer.Text.back()); 
                _lexer.Text.pop_back();
                _index += 2;
                break; 
            }
            /* '}' */
            case TOKEN::RBRACE: break;
            /* '|' */
            case TOKEN::OR:
                /* OR 延后处理 */
                Parser_Stack.push_back(parse_stack_t(TOKEN::OR, 0, -1));
                break;

            /* '*' */
            case TOKEN::CLOSURE: { parse_closure(); break; }

            /* '(' */
            case TOKEN::LBRACKET:
                Parser_Stack.push_back(parse_stack_t(TOKEN::LBRACKET, 0, -1));
                break;

            /* ')' */
            case TOKEN::RBRACKET: { parse_exp(); break; }
            case TOKEN::EXP: break;

            default:
                std::cout << "parse: err." << std::endl;
                return false;
        }
        ++_index;
    }

    if (!Parser_Stack.empty())
        parse_exp();
    
    Code.push_back(CODE_ELM(BYTE_CODE::ACCEPT, 0, 0));
    return true;
}

void RE_Parser::output_code()
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
                if (exp_t == TOKEN::ANY)
                    std::cout << "  MATCH " << "ANY" << std::endl;
                else
                    std::cout << "  MATCH " << (std::string)reinterpret_cast<const char*>(exp_t) << std::endl;
                
                break;
            }
            case BYTE_CODE::JMP:
                std::cout << "  JMP " << reinterpret_cast<std::ptrdiff_t>(i.exp1) << std::endl; 
                break;
            case BYTE_CODE::RANGE:
                std::cout << "  RANGE " << reinterpret_cast<std::ptrdiff_t>(i.exp1) << ", " << reinterpret_cast<std::ptrdiff_t>(i.exp2) << std::endl; 
                break;
            case BYTE_CODE::ACCEPT: std::cout << "  ACCEPT" << std::endl; break;
            case BYTE_CODE::HALT:   std::cout << "  HALT" << std::endl;   break;
            case BYTE_CODE::REPEND: std::cout << "  REPEND" << std::endl;  break;
            case BYTE_CODE::REPEAT: 
                std::cout << "  REPEAT " << reinterpret_cast<std::ptrdiff_t>(i.exp1) << std::endl;
                break;
            default:
                std::cout << "  unknown unknown unknown" << std::endl;
                break;
        }
    }
    std::cout << std::endl;
}

inline void RE_Parser::parser_init()
{
    Parser_Stack.clear();
    Code.clear();
}

inline void RE_Parser::parse_string(RE_Lexer& _lexer)
{
    Parser_Stack.push_back(parse_stack_t(TOKEN::STRING, 1, Code.size()));
    Code.push_back(CODE_ELM(BYTE_CODE::MATCH, const_cast<char*>(_lexer.Text.back().data()), 0));
    _lexer.Text.pop_back();
}

inline void RE_Parser::parse_any()
{
    Parser_Stack.push_back(parse_stack_t(TOKEN::STRING, 1, Code.size()));
    Code.push_back(CODE_ELM(BYTE_CODE::MATCH, TOKEN::ANY, 0)); /* -1 means match any */
}

inline void RE_Parser::parse_plus()
{
    /*
     * 0 exp
     * 1 split -1,1
     */
    parse_stack_t exp = Parser_Stack.back();
    Parser_Stack.pop_back();

    Code.insert(Code.begin() + exp.ip + exp.n, CODE_ELM(BYTE_CODE::SPLIT, -exp.n, 1));

    exp.n += 1;
    exp.tk = TOKEN::EXP;
    Parser_Stack.push_back(exp);
}

inline void RE_Parser::parse_question()
{
    /*
     * 0 split 1,2
     * 1 exp
     * 2 ...
     */
    parse_stack_t exp = Parser_Stack.back();
    Parser_Stack.pop_back();

    Code.insert(Code.begin() + exp.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp.n + 1));

    exp.n += 1;
    exp.tk = TOKEN::EXP;
    Parser_Stack.push_back(exp);
}

inline void RE_Parser::parse_closure()
{
    /* 
     * 0 split 1 3 
     * 1 exp(pst)
     * 2 split -1 3
     * 3 do...
     */
    parse_stack_t exp = Parser_Stack.back(); 
    Parser_Stack.pop_back();

    Code.insert(Code.begin() + exp.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp.n + 1 + 1));
    Code.insert(Code.begin() + exp.ip + 1 + exp.n, CODE_ELM(BYTE_CODE::SPLIT, -exp.n, 1));

    exp.n += 2;
    exp.tk = TOKEN::EXP;
    Parser_Stack.push_back(exp);
}

bool RE_Parser::parse_exp()
{
    while (Parser_Stack.size() > 1 && Parser_Stack.back().tk != TOKEN::LBRACKET)
    {
        /* 归约解析表达式 需处理or操作 */
        bool have_or = false;
        auto l_exp = parse_stack_t(), 
             r_exp = parse_stack_t(),
             or_op = parse_stack_t();

        while (!Parser_Stack.empty() 
                && Parser_Stack.back().tk != TOKEN::OR 
                && Parser_Stack.back().tk != TOKEN::LBRACKET)
        {
            l_exp.n += Parser_Stack.back().n;
            l_exp.ip = Parser_Stack.back().ip;
            Parser_Stack.pop_back();
        }
        l_exp.tk = TOKEN::EXP;
        
        if (!Parser_Stack.empty() && Parser_Stack.back().tk == TOKEN::OR)
        {
            or_op = Parser_Stack.back();
            Parser_Stack.pop_back();
            have_or = true;

            while (!Parser_Stack.empty() 
                    && Parser_Stack.back().tk != TOKEN::OR 
                    && Parser_Stack.back().tk != TOKEN::LBRACKET)
            {
                r_exp.n += Parser_Stack.back().n;
                r_exp.ip = Parser_Stack.back().ip;
                Parser_Stack.pop_back();
            }
            r_exp.tk = TOKEN::EXP;
        }

        /* 生成Code */
        if (!have_or)
        {
            Parser_Stack.push_back(l_exp);
            break; /* 没有or 不可进一步规约 */
        }
        else 
        { 
            /* OR:
             * 0 split 1 4 
             * 1 exp1
             * 2 jmp 4
             * 3 exp2
             * 4 do...
             */            
            if (l_exp.ip > r_exp.ip)
                std::swap(l_exp, r_exp);
            or_op.ip = l_exp.ip; /* update ip */

            /* insert JMP */
            r_exp.ip++;
            Code.insert(Code.begin() + l_exp.ip + l_exp.n, CODE_ELM(BYTE_CODE::JMP, r_exp.n + 1, 0));

            /*  insert SPLIT (+1) */
            l_exp.ip++; r_exp.ip++;
            Code.insert(Code.begin() + or_op.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, r_exp.ip - or_op.ip));

            or_op.tk = TOKEN::EXP;
            or_op.n += (l_exp.n + r_exp.n + 2);
            Parser_Stack.push_back(or_op);            
        }
    }
    /* ( EXP */
    if (Parser_Stack.size() > 2)
    {
        parse_stack_t exp = Parser_Stack.back(); 
        Parser_Stack.pop_back(); /* EXP */
        Parser_Stack.pop_back(); /* ( */
        Parser_Stack.push_back(exp);
    }

    return true;
}

bool RE_Parser::parse_brace(std::string& exp)
{
    /*
     * 0 repeat n
     * 1 exp1
     * 2 repend
     * 3 (split 1,4)
     * 4 ...
     */
    int n, m;
    auto domma = exp.find(",");
    
    auto exp1 = Parser_Stack.back();
    Parser_Stack.pop_back();
    
    /* {n,m} 处理n */
    n = std::stoi(exp.substr(0, domma));
    Code.insert(Code.begin() + exp1.ip + exp1.n, CODE_ELM(BYTE_CODE::REPEND, 0, 0));
    Code.insert(Code.begin() + exp1.ip, CODE_ELM(BYTE_CODE::REPEAT, n, 0));
    n = std::stoi(exp.substr(0, domma));
    exp1.n += 2;
    exp1.tk = TOKEN::EXP;

    if (domma != std::string::npos)
    {
        int m_ip = exp1.ip + exp1.n;
        std::vector<ByteCode> tmp(exp1.n);
        std::copy(Code.begin() + exp1.ip, Code.begin() + exp1.ip + exp1.n, tmp.begin());
        Code.insert(Code.begin() + m_ip, tmp.begin(), tmp.end());
        /* {n,m}
         *
         * 0 repeat m
         * 1 split 1, 4
         * 2 exp
         * 3 repend
         * 4 ...
         */
        if (exp.length() - domma - 1 > 0) 
        {
            m = std::stoi(exp.substr(domma + 1, exp.length() - domma - 1));
            Code[m_ip].exp1 = reinterpret_cast<void*>(m - n);
            Code.insert(Code.begin() + m_ip + 1, CODE_ELM(BYTE_CODE::SPLIT, 1, exp1.n));
            exp1.n += (exp1.n + 1);
        }
        /* {n,}
         * 
         * 0 split 1, 3
         * 1 exp
         * 2 split 1, 3
         * 3 ...
         */
        else
        {
            Code[m_ip] = CODE_ELM(BYTE_CODE::SPLIT, 1, exp1.n);
            Code[m_ip + exp1.n - 1] = CODE_ELM(BYTE_CODE::SPLIT, -(exp1.n - 2), 1);
            exp1.n *= 2;
        }
    }

    Parser_Stack.push_back(exp1);

    return true;
}

bool RE_Parser::parse_square_brace(std::string& s)
{
    bool _not = false;
    std::vector<ByteCode> ins;
    std::ptrdiff_t _index;
    if (s[0] == '^') 
    {
        _index = 1;
        _not = true;
    }
    else
    {
        _index = 0;
        _not = false;
    }

    while (_index < s.length())
    {
        /* 向后看2个 如果存在a-z这样的表达式 */
        if (_index + 2 < s.length() && s[_index + 1] == '-')
        {
            ins.push_back(CODE_ELM(BYTE_CODE::RANGE, s[_index], s[_index + 2]));
            _index += 3;
        }
        /* 普通匹配一个字符 这里使用RANGE特殊情况 */
        else
        {
            ins.push_back(CODE_ELM(BYTE_CODE::RANGE, s[_index], s[_index]));
            _index++;
        }
    }

    parse_stack_t exp;
    exp.ip = Code.size();
    exp.tk = TOKEN::EXP;
    ByteCode bc1, bc2;
    /* 预组合 */
    if (ins.size() == 1)
    {
        Code.push_back(ins.back()); ins.pop_back();
        exp.n = 1;
    }
    else if (ins.size() >= 2)
    {
        bc1 = ins.back(); ins.pop_back();
        bc2 = ins.back(); ins.pop_back();
        Code.push_back(CODE_ELM(BYTE_CODE::SPLIT, 1, 3));
        Code.push_back(bc1);
        Code.push_back(CODE_ELM(BYTE_CODE::JMP, 2, 0));
        Code.push_back(bc2);
        exp.n = 4;
    }
    else
    {
        std::cout << "empty []." << std::endl;
        return false;
    }

    /* 指令用split组合 */
    while (!ins.empty())
    {
        bc1 = ins.back(); ins.pop_back();
        Code.insert(Code.begin() + exp.ip + exp.n, CODE_ELM(BYTE_CODE::JMP, 2, 0));
        Code.insert(Code.begin() + exp.ip + exp.n + 1, bc1);
        Code.insert(Code.begin() + exp.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp.n + 2));
        exp.n += 3;
    }

    if (_not)
    {
        /*
         * 0 split 1, 3
         * 1 exp1
         * 2 halt
         * 3 match any
         * 4 ...
         */
        Code.insert(Code.begin() + exp.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp.n + 2));
        Code.insert(Code.begin() + exp.ip + exp.n + 1, CODE_ELM(BYTE_CODE::HALT, 0, 0));
        Code.insert(Code.begin() + exp.ip + exp.n + 2, CODE_ELM(BYTE_CODE::MATCH, TOKEN::ANY, 0));
        exp.n += 3;
    }

    Parser_Stack.push_back(exp);
    return true;
}

#endif