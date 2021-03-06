#ifndef MINI_REGEXP_PARSER_CPP_
#define MINI_REGEXP_PARSER_CPP_

#include "re_parser.hpp"

using namespace mini_regexp_parser;

RE_Parser::RE_Parser() {}

bool RE_Parser::parser(RE_Lexer& _lexer, RE_Config& config)
{
    parser_init();
    std::reverse(_lexer.Text.begin(), _lexer.Text.end());
    return parse_main(_lexer, 0, _lexer.Token.size(), config);
}

bool RE_Parser::parse_main(RE_Lexer& _lexer, 
    std::ptrdiff_t _index, std::ptrdiff_t _len, 
    RE_Config& config)
{
    while (_index < _len)
    {
        switch (_lexer.Token[_index])
        {
            case TOKEN::STRING: parse_string(_lexer); break;
            case TOKEN::ANY:    parse_any();          break; /* '.' */
            /* '+' */
            case TOKEN::PLUS: 
            {
                /* '+' '?' */
                bool is_greedy = true;
                if (_index + 1 < _len && _lexer.Token[_index + 1] == TOKEN::QUESTION)
                {
                    _index++;
                    is_greedy = false;
                }
                parse_plus(is_greedy); 
                break; 
            }
            /* '?' */
            case TOKEN::QUESTION: 
            {
                /* '?' '?' */
                bool is_greedy = true;
                if (_index + 1 < _len && _lexer.Token[_index + 1] == TOKEN::QUESTION)
                {
                    _index++;
                    is_greedy = false;
                }
                parse_question(is_greedy); 
                break; 
            }

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
                /* '{' '}' '?' */
                bool is_greedy = true;
                if (_index + 3 < _len && _lexer.Token[_index + 3] == TOKEN::QUESTION)
                {
                    _index++;
                    is_greedy = false; 
                }
                parse_brace(_lexer.Text.back(), is_greedy);
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
            case TOKEN::CLOSURE: 
            {
                /* '*' '?' */
                bool is_greedy = true;
                if (_index + 1 < _len && _lexer.Token[_index + 1] == TOKEN::QUESTION)
                {
                    _index++;
                    is_greedy = false;
                }
                parse_closure(is_greedy); 
                break; 
            }

            /* '(' */
            case TOKEN::LBRACKET:
                Parser_Stack.push_back(parse_stack_t(TOKEN::LBRACKET, 1, Code.size()));
                Code.push_back(CODE_ELM(BYTE_CODE::ENTER, 0, 0));
                break;

            /* ')' */
            case TOKEN::RBRACKET: 
            {
                parse_exp();
                Code.push_back(CODE_ELM(BYTE_CODE::LEAVE, 0, 0));
                break; 
            }

            /*--- 零宽断言 start ---*/

            case TOKEN::ZERO_WIDTH_ASSERT_LBRACKET:
                Parser_Stack.push_back(parse_stack_t(TOKEN::ZERO_WIDTH_ASSERT_LBRACKET, 
                    1, Code.size()));
                Code.push_back(CODE_ELM(BYTE_CODE::ZERO_WIDTH_ASSERT_ENTER, 
                    _lexer.Token[++_index], 0));
                ZeroWidthAssert_Parse_Stack.push_back(zero_width_assert_parse_t(_lexer.Token[_index]));
                break;

            case TOKEN::ZERO_WIDTH_ASSERT_RBRACKET:
            {
                parse_zero_width_assert();
                break;
            }

            /*--- 零宽断言 end ---*/

            case TOKEN::GROUP:
                parse_group(_lexer);
                break;

            case TOKEN::EXP: break;
            default:
                /*ERROR */
                std::cout << "ERROR: parse." << std::endl;
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
    std::cout << "Code Length: " << Code.size() << std::endl;
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
                else if (exp_t == TOKEN::GROUP)
                    std::cout << "  MATCH " << "GROUP, " << reinterpret_cast<std::ptrdiff_t>(i.exp2) << std::endl;
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
            case BYTE_CODE::ENTER:
                std::cout << "  ENTER " << std::endl;
                break;
            case BYTE_CODE::LEAVE:
                std::cout << "  LEAVE " << std::endl;
                break;
            case BYTE_CODE::ZERO_WIDTH_ASSERT_ENTER:
                std::cout << "  ZERO_WIDTH_ASSERT_ENTER ";
                switch (reinterpret_cast<std::ptrdiff_t>(i.exp1))
                {
                    case TOKEN::FORWARD_PRE_MATCH:
                        std::cout << "FORWARD_PRE_MATCH" << std::endl;
                        break;
                    case TOKEN::FORWARD_PRE_MATCH_NOT:
                        std::cout << "FORWARD_PRE_MATCH_NOT" << std::endl;
                        break;
                    case TOKEN::BACKWORD_PRE_MATCH:
                        std::cout << "BACKWORD_PRE_MATCH" << std::endl;
                        break;
                    case TOKEN::BACKWORD_PRE_MATCH_NOT:
                        std::cout << "BACKWORD_PRE_MATCH_NOT" << std::endl;
                        break;
                    case TOKEN::NORMAL_PRE_MATCH:
                        std::cout << "NORMAL_PRE_MATCH" << std::endl;
                        break;
                }
                break;
            case BYTE_CODE::ZERO_WIDTH_ASSERT_LEAVE:
                std::cout << "  ZERO_WIDTH_ASSERT_LEAVE " << std::endl;
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
    ZeroWidthAssert_Parse_Stack.clear();
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

inline void RE_Parser::parse_plus(bool greedy_mode)
{
    /*
     * 0 exp
     * 1 split -1,1
     */
    parse_stack_t exp = Parser_Stack.back();
    Parser_Stack.pop_back();

    Code.insert(Code.begin() + exp.ip + exp.n, CODE_ELM(BYTE_CODE::SPLIT, -exp.n, 1));
    if (!greedy_mode)
        std::swap(Code[exp.ip + exp.n].exp1, Code[exp.ip + exp.n].exp2);

    exp.n += 1;
    exp.tk = TOKEN::EXP;
    Parser_Stack.push_back(exp);
}

inline void RE_Parser::parse_question(bool greedy_mode)
{
    /*
     * 0 split 1,2
     * 1 exp
     * 2 ...
     */
    parse_stack_t exp = Parser_Stack.back();
    Parser_Stack.pop_back();

    Code.insert(Code.begin() + exp.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp.n + 1));
    if (!greedy_mode)
        std::swap(Code[exp.ip].exp1, Code[exp.ip].exp2);

    exp.n += 1;
    exp.tk = TOKEN::EXP;
    Parser_Stack.push_back(exp);
}

inline void RE_Parser::parse_closure(bool greedy_mode)
{
    /* 
     * 0 split 1 3 
     * 1 exp(pst)
     * 2 split -1 1
     * 3 do...
     */
    parse_stack_t exp = Parser_Stack.back(); 
    Parser_Stack.pop_back();

    Code.insert(Code.begin() + exp.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp.n + 1 + 1));
    Code.insert(Code.begin() + exp.ip + 1 + exp.n, CODE_ELM(BYTE_CODE::SPLIT, -exp.n, 1));
    if (!greedy_mode)
    {
        std::swap(Code[exp.ip].exp1, Code[exp.ip].exp2);
        std::swap(Code[exp.ip + 1 + exp.n].exp1, Code[exp.ip + 1 + exp.n].exp2);
    }

    exp.n += 2;
    exp.tk = TOKEN::EXP;
    Parser_Stack.push_back(exp);
}

bool RE_Parser::parse_exp()
{
    while (Parser_Stack.size() > 1 
        && Parser_Stack.back().tk != TOKEN::LBRACKET 
        && Parser_Stack.back().tk != TOKEN::ZERO_WIDTH_ASSERT_LBRACKET)
    {
        /* 归约解析表达式 需处理or操作 */
        bool have_or = false;
        auto l_exp = parse_stack_t(), 
             r_exp = parse_stack_t(),
             or_op = parse_stack_t();

        while (!Parser_Stack.empty() 
                && Parser_Stack.back().tk != TOKEN::OR 
                && Parser_Stack.back().tk != TOKEN::LBRACKET
                && Parser_Stack.back().tk != TOKEN::ZERO_WIDTH_ASSERT_LBRACKET)
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
                    && Parser_Stack.back().tk != TOKEN::LBRACKET
                    && Parser_Stack.back().tk != TOKEN::ZERO_WIDTH_ASSERT_LBRACKET)
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
    if (Parser_Stack.size() >= 2)
    {
        parse_stack_t exp = Parser_Stack.back(); 
        Parser_Stack.pop_back(); /* EXP */
        exp.ip = Parser_Stack.back().ip;
        Parser_Stack.pop_back(); /* ( */
        exp.n += 2; /* 加了一对 ENTER LEAVE */
        Parser_Stack.push_back(exp);
    }

    return true;
}

bool RE_Parser::parse_brace(std::string& exp, bool greedy_mode)
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
            if (!greedy_mode)
                std::swap(Code[m_ip + 1].exp1, Code[m_ip + 1].exp2);
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
            if (!greedy_mode)
            {
                std::swap(Code[m_ip].exp1, Code[m_ip].exp2);
                std::swap(Code[m_ip + exp1.n - 1].exp1, Code[m_ip + exp1.n - 1].exp2);
            }
            exp1.n *= 2;
        }
    }

    Parser_Stack.push_back(exp1);

    return true;
}

/* 解析[]中的转义字符 */
std::ptrdiff_t RE_Parser::parse_square_escape_char(
    std::string& s, std::ptrdiff_t _index, std::string& res)
{
    auto spec_ch = s[++_index];
    if (mini_keywords.is_escape_char(spec_ch))
    {
        res = mini_keywords.get_escape_char(spec_ch);
        return _index + 1;
    }
    else
    {
        std::string num;
        switch (spec_ch)
        {
            /* 八进制 \0XXX */
            case '0':
            {
                std::string s;
                num = s.substr(_index + 1, 3);
                res = s + char(str2oct(num));
                _index += 4;
                break;
            }
            /* 十进制 \X... */
            case'1':case'2':case'3':case'4':case'5':
            case'6':case'7':case'8':case'9':
                _index += str_get_digit(s, _index, num);
                res = num;
                break;
            case 'x': /* 16进制: \xXX */
            {
                std::string s;
                num = s.substr(_index + 1, 2);
                res = s + char(str2hex(num));
                _index += 3;
                break;
            }

            case 'u': /* Unicode \uXXXX */
            {
                std::string s1, s2;
                num = s.substr(_index + 1, 2);
                auto num2 = s.substr(_index + 3, 2);
                res = s1 + char(str2hex(num))+ char(str2hex(num2));
                // std::cout << Text.back().length() << std::endl;
                _index += 5;
                break;
            }

            default:
                /* 加到string中 \X */
                res = s.substr(_index, 1);
                _index += 1;
                break;
        }
    }
    return _index;
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

    std::vector<std::string> sub_string; /* \w \W \d \D... */
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
            if (s[_index] == '\\')
            {
                std::string res;
                _index = parse_square_escape_char(s, _index, res);
                sub_string.push_back(res);
                // std::cout << res << " " << std::endl;
            }
            else
            {
                ins.push_back(CODE_ELM(BYTE_CODE::RANGE, s[_index], s[_index]));
                _index++;
            }
        }
    }

    /* 组合当前表达式---start */
    parse_stack_t exp;
    exp.ip = Code.size();
    exp.tk = TOKEN::EXP;
    exp.n = 0;
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
        std::cout << "parse_square_brace err." << std::endl;
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
    /* 组合当前表达式---end */

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
        Parser_Stack.back().n += 3;
    }
    
    Parser_Stack.push_back(exp);

    return true;
}

inline void RE_Parser::parse_group(RE_Lexer& _lexer)
{
    Parser_Stack.push_back(parse_stack_t(TOKEN::STRING, 1, Code.size()));
    Code.push_back(CODE_ELM(BYTE_CODE::MATCH, TOKEN::GROUP, str2int(_lexer.Text.back())));
    _lexer.Text.pop_back();
}

inline void RE_Parser::parse_zero_width_assert()
{
    parse_exp();
    /* LEAVE */
    Parser_Stack.back().n++;
    Code.push_back(CODE_ELM(BYTE_CODE::ZERO_WIDTH_ASSERT_LEAVE, 1, 0));
    /* NOT */
    auto exp1 = ZeroWidthAssert_Parse_Stack.back().tk;
    ZeroWidthAssert_Parse_Stack.pop_back();
    if (exp1 == TOKEN::FORWARD_PRE_MATCH_NOT 
        || exp1 == TOKEN::BACKWORD_PRE_MATCH_NOT)
    {
        auto pst = Parser_Stack.back();
        Parser_Stack.pop_back();
        if (pst.ip + pst.n >= Code.size())
            Code.push_back(CODE_ELM(BYTE_CODE::HALT, 0, 0));
        else
            Code.insert(Code.begin() + pst.ip + pst.n, CODE_ELM(BYTE_CODE::HALT, 0, 0));
        Code.insert(Code.begin() + pst.ip + 1, CODE_ELM(BYTE_CODE::SPLIT, 1, pst.n));
        pst.tk = TOKEN::EXP;
        pst.n += 2;
        Parser_Stack.push_back(pst);
    }
}

#endif