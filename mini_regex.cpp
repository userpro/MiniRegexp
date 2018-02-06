#include "mini_regex.hpp"

mini_regex::mini_regex()
    :regexp(""),code(),token()
{
}

bool mini_regex::compile(const std::string& regexp_str)
{
    _reset();
    regexp = regexp_str;
    lexer();
    return parse();
}


bool mini_regex::match(const std::string& match_str, std::function<void(regex_result&)> callback)
{
    target = match_str;
    return evalute();
    // return false;
}

void mini_regex::_reset()
{
    regexp = "";
    target = "";
    for (std::vector<CODE_TYPE>::iterator i = code.begin(); i != code.end(); ++i)
    {
        if (i->first == MATCH)
            delete(reinterpret_cast<std::string*>(i->second.first));
    }
    code.clear();
    token.clear();
    text.clear();
    while (!S1.empty()) S1.pop();
    while (!S2.empty()) S2.pop();
}

void mini_regex::lexer()
{
    std::ptrdiff_t _index = 0, _len = regexp.length();
    while (_index < _len)
    {
        switch (regexp[_index])
        {
            case '*':
                token.push_back(TOKEN::CLOSURE);
                _index++;
                break;
            case '|':
                token.push_back(TOKEN::OR);
                _index++;
                break;
            case '(':
                token.push_back(TOKEN::LBRACKET);
                _index++;
                break;
            case ')':
                token.push_back(TOKEN::RBRACKET);
                _index++;
                break;
            default:
            {
                unsigned int start_pos = _index, end_pos = _index;
                while (end_pos < _len && ((regexp[end_pos] >= 'a' && regexp[end_pos] <= 'z') || (regexp[end_pos] >= 'A' && regexp[end_pos] <= 'Z') || (regexp[end_pos] >= '0' && regexp[end_pos] <= '9')))
                    end_pos++;
                text.push_back(regexp.substr(start_pos, end_pos - start_pos));
                token.push_back(TEXT);
                _index = end_pos;
                break;
            }
        }
    }
}

/* Priority[from low -> high]: 
 *      '(', '|', ')', '*'
 */
bool mini_regex::parse()
{
    std::ptrdiff_t _index = 0, _len = token.size();
    std::reverse(text.begin(), text.end());

    while (_index < _len)
    {
        switch (token[_index])
        {
            case TOKEN::TEXT: 
                S2.push(parse_stack_t(TOKEN::TEXT,1,code.size()));
                code.push_back(CODE_ELM(BYTE_CODE::MATCH,const_cast<char*>(text.back().data()),0));
                text.pop_back();
                break;

            case TOKEN::OR:
                /* OR 延后处理 */
                S1.push(parse_stack_t(TOKEN::OR,2,-1));
                break;

            case TOKEN::CLOSURE:
            {
                /* 
                 * 0 split 1 3 
                 * 1 exp(pst)
                 * 2 split 1 3
                 * 3 do...
                 */
                parse_stack_t exp = S2.top();
                S2.pop();

                code.insert(code.begin() + exp.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp.n + 1 + 1));
                code.insert(code.begin() + exp.ip + 1 + exp.n, CODE_ELM(BYTE_CODE::SPLIT, -exp.n, 1));

                exp.n += 2;
                S2.push(exp);
                break;
            }

            case TOKEN::LBRACKET:
                S1.push(parse_stack_t(TOKEN::LBRACKET,0,-1));
                break;

            case TOKEN::RBRACKET:
            {
                while (S1.top().tk != TOKEN::LBRACKET)
                {
                    if (S1.top().tk == TOKEN::OR)
                        parse_or(); /* OR处理 */
                    S1.pop();
                }
                S1.pop();
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

    code.push_back(CODE_ELM(ACCEPT,0,0));
    return true;
}
/* 
 * OR 处理:
 *      0 split 1 4 
 *      1 exp1
 *      2 jmp 4
 *      3 exp2
 *      4 do...
 */
bool mini_regex::parse_or()
{
    parse_stack_t s1_pst, exp1, exp2;
    s1_pst = S1.top();

    if (S2.size() < 2)
    {
        std::cout << "parse: '|' not enough text." << std::endl;
        return false;
    }
    /* 根据栈的特性 先出的ip大于后出的 */
    exp2 = S2.top(); S2.pop();
    exp1 = S2.top(); S2.pop();
    if (exp1.ip > exp2.ip)
        std::swap(exp1, exp2);
    s1_pst.ip = exp1.ip; /* update ip */

    // std::cout << "exp1.ip: " << exp1.ip << " exp1.n: " << exp1.n << std::endl;
    // std::cout << "exp2.ip: " << exp2.ip << " exp2.n: " << exp2.n << std::endl;

    /* insert JMP */
    exp2.ip++;
    code.insert(code.begin() + exp1.ip + exp1.n, CODE_ELM(BYTE_CODE::JMP, exp2.n + 1, 0));

    /*  insert SPLIT (+1) */
    exp1.ip++; exp2.ip++;
    code.insert(code.begin() + s1_pst.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp2.ip - s1_pst.ip));

    s1_pst.tk = TOKEN::EXP;
    s1_pst.n += (exp1.n + exp2.n);
    S2.push(s1_pst);

    return true;
}

bool mini_regex::evalute()
{
    return false;
}

void mini_regex::output_code()
{
    std::cout << "Regexp: " << regexp << std::endl;
    std::cout << "Generate Code: " << std::endl;
    for (std::vector<CODE_TYPE>::iterator i = code.begin(); i != code.end(); ++i)
    {
        switch (i->first)
        {
            case BYTE_CODE::SPLIT:
                std::cout << "  SPLIT " << reinterpret_cast<std::ptrdiff_t>(i->second.first)\
                        << ", " << reinterpret_cast<std::ptrdiff_t>(i->second.second) << std::endl;
                break;
            case BYTE_CODE::MATCH:
            {
                std::string s = reinterpret_cast<const char*>(i->second.first);
                std::cout << "  MATCH " << s << std::endl;
                break;
            }
            case BYTE_CODE::JMP:
                std::cout << "  JMP " << reinterpret_cast<std::ptrdiff_t>(i->second.first) << std::endl;
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
}