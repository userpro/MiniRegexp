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
            case TOKEN::STRING:
            {
                Val.push(parse_stack_t(TOKEN::STRING, 1, Code.size()));
                Code.push_back(CODE_ELM(BYTE_CODE::MATCH, const_cast<char*>(_lexer.Text.back().data()), 0));
                _lexer.Text.pop_back();
                break;
            }

            /* '.' */
            case TOKEN::ANY: 
                Val.push(parse_stack_t(TOKEN::STRING, 1, Code.size()));
                Code.push_back(CODE_ELM(BYTE_CODE::MATCH, TOKEN::ANY, 0)); /* -1 means match any */
                break;

            case TOKEN::DIGIT:
                Val.push(parse_stack_t(TOKEN::STRING, 1, Code.size()));
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
                parse_stack_t exp = Val.top();
                Val.pop();

                Code.insert(Code.begin() + exp.ip + exp.n, CODE_ELM(BYTE_CODE::SPLIT, -exp.n, 1));

                exp.n += 1;
                exp.tk = TOKEN::EXP;
                Val.push(exp);
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
                parse_stack_t exp = Val.top();
                Val.pop();

                Code.insert(Code.begin() + exp.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp.n + 1));

                exp.n += 1;
                exp.tk = TOKEN::EXP;
                Val.push(exp);
                break;
            }

            /* '[' */
            case TOKEN::SQUARE_LBRACKET:
                break;

            /* ']' */
            case TOKEN::SQUARE_RBRACKET:
                break;

            /* '{' parse to '}' */
            /*  */
            case TOKEN::LBRACE:
            {
                /*
                 * 0 repeat n=1,m=2
                 * 1 exp1
                 * 2 repend
                 * 3 (split 1,4)(if m = TOKEN::INF)
                 */
                int n,m;
                /* get 'n' */
                if (_lexer.Token[_index + 1] == TOKEN::STRING)
                {
                    n = std::stoi(_lexer.Text.back());
                    _lexer.Text.pop_back();
                    _index++;
                }
                else
                {
                    std::cout << "parser {} err" << std::endl;
                    break;
                }

                /* after 'n'(Token[_index] is 'n') */
                if (_lexer.Token[_index + 1] == TOKEN::COMMA)
                {
                    /* '{' 'n,' 'm' '}' */
                    if (_lexer.Token[_index + 2] == TOKEN::STRING)
                    {
                        m = std::stoi(_lexer.Text.back());
                        _lexer.Text.pop_back();
                        _index += 2;
                    }
                    /* '{' 'n' ',' '}' */
                    else if (_lexer.Token[_index + 1] == TOKEN::RBRACE) 
                    {
                        m = TOKEN::INF;
                        _index++;
                    }
                    /* err */
                    else std::cout << "mismatch brace!" << std::endl;
                }
                /* '{' 'n' '}' */
                else if (_lexer.Token[_index + 1] == TOKEN::RBRACE) 
                {
                    m = TOKEN::NONE;
                    _index++;
                }
                /* err */
                else std::cout << "mismatch brace!" << std::endl;

                /* '?'启用非贪婪模式 */
                if (_index + 1 < _lexer.Token.size() && _lexer.Token[_index + 1] == TOKEN::QUESTION)
                    m = TOKEN::NONE;

                parse_stack_t exp = Val.top();
                Val.pop();
                Code.insert(Code.begin() + exp.ip + exp.n, CODE_ELM(BYTE_CODE::REPEND, 0, 0));
                Code.insert(Code.begin() + exp.ip, CODE_ELM(BYTE_CODE::REPEAT, n, m));

                /* like {1, } */
                if (m == TOKEN::INF)
                {
                    Code.insert(Code.begin() + exp.ip + exp.n + 1, CODE_ELM(BYTE_CODE::SPLIT, -(exp.n + 1), 1));
                    exp.n += 1;
                }

                exp.n += 2;
                exp.tk = TOKEN::EXP;
                Val.push(exp);
                break;
            }

            /* '}' */
            case TOKEN::RBRACE: break;
            
            /* '^' */
            case TOKEN::BEGIN: _begin = true; break;
            /* '$' */
            case TOKEN::END: _end = true; break;

            /* '|' */
            case TOKEN::OR:
                /* OR 延后处理 */
                Op.push(parse_stack_t(TOKEN::OR, 2, -1));
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
                parse_stack_t exp = Val.top();
                Val.pop();

                Code.insert(Code.begin() + exp.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp.n + 1 + 1));
                Code.insert(Code.begin() + exp.ip + 1 + exp.n, CODE_ELM(BYTE_CODE::SPLIT, -exp.n, 1));

                exp.n += 2;
                exp.tk = TOKEN::EXP;
                Val.push(exp);
                break;
            }

            /* '(' */
            case TOKEN::LBRACKET:
                Op.push(parse_stack_t(TOKEN::LBRACKET, 0, -1));
                Val.push(parse_stack_t(TOKEN::LBRACKET, 0, -1));
                break;

            /* ')' */
            case TOKEN::RBRACKET:
            {
                while (Op.top().tk != TOKEN::LBRACKET)
                {
                    if (Op.top().tk == TOKEN::OR)
                        parse_or(); /* OR处理 */
                    Op.pop();
                }
                if (!Op.empty()) Op.pop();
                else std::cout << "mismatch parentheses." << std::endl;
            }

            case TOKEN::EXP: break;

            default:
                std::cout << "parse: err." << std::endl;
                return false;
        }
        ++_index;
    }

    while (!Op.empty())
    {
        if (Op.top().tk == TOKEN::OR)
            parse_or();
        Op.pop();
    }

    while (!Val.empty()) Val.pop();

    /* ^ */
    if (_begin) Code.insert(Code.begin(), CODE_ELM(BYTE_CODE::MATCH, TOKEN::BEGIN, 0));
    /* $ */
    if (_end)   Code.push_back(CODE_ELM(BYTE_CODE::MATCH, TOKEN::END, 0));
    
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
                switch (exp_t)
                {
                    case TOKEN::ANY:   std::cout << "  MATCH " << "ANY" << std::endl;   break;
                    case TOKEN::DIGIT: std::cout << "  MATCH " << "DIGIT" << std::endl; break;
                    case TOKEN::SPACE: std::cout << "  MATCH " << "SPACE" << std::endl; break;
                    case TOKEN::BEGIN: std::cout << "  MATCH " << "^" << std::endl;     break;
                    case TOKEN::END:   std::cout << "  MATCH " << "$" << std::endl;     break;
                    default:
                        std::cout << "  MATCH " << (std::string)reinterpret_cast<const char*>(exp_t) << std::endl;
                }
                
                break;
            }
            case BYTE_CODE::JMP:
                std::cout << "  JMP " << reinterpret_cast<std::ptrdiff_t>(i.exp1) << std::endl; 
                break;
            case BYTE_CODE::ACCEPT: std::cout << "  ACCEPT" << std::endl; break;
            case BYTE_CODE::HALT:   std::cout << "  HALT" << std::endl;   break;
            case BYTE_CODE::REPEND: std::cout << "  REPEND" << std::endl;  break;
            case BYTE_CODE::REPEAT: 
                std::cout << "  REPEAT " << reinterpret_cast<std::ptrdiff_t>(i.exp1) << ", " << reinterpret_cast<std::ptrdiff_t>(i.exp2) << std::endl;
                break;
            default:
                std::cout << "  unknown unknown unknown" << std::endl;
                break;
        }
    }
    std::cout << std::endl;
}

void RE_Parser::parser_init()
{
    Code.clear();
    while (!Op.empty()) Op.pop();
    while (!Val.empty()) Val.pop();
}

/* 
 * OR 处理:
 *      0 split 1 4 
 *      1 exp1
 *      2 jmp 4
 *      3 exp2
 *      4 do...
 */
bool RE_Parser::parse_or()
{
    parse_stack_t Op_pst, exp1, exp2;
    Op_pst = Op.top();

    if (Val.size() < 2)
    {
        std::cout << "parse: '|' not enough Text." << std::endl;
        return false;
    }
    exp2 = Val.top(); Val.pop();
    exp1 = Val.top(); Val.pop();
    if (exp1.ip > exp2.ip)
        std::swap(exp1, exp2);
    Op_pst.ip = exp1.ip; /* update ip */

    /* insert JMP */
    exp2.ip++;
    Code.insert(Code.begin() + exp1.ip + exp1.n, CODE_ELM(BYTE_CODE::JMP, exp2.n + 1, 0));

    /*  insert SPLIT (+1) */
    exp1.ip++; exp2.ip++;
    Code.insert(Code.begin() + Op_pst.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp2.ip - Op_pst.ip));

    Op_pst.tk = TOKEN::EXP;
    Op_pst.n += (exp1.n + exp2.n);
    Val.push(Op_pst);

    return true;
}

#endif