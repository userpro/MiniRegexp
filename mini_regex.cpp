#ifndef _MINI_REGEX_CPP
#define _MINI_REGEX_CPP

#include "mini_regex.hpp"

mini_regex::mini_regex()
    :regexp(""),Code(),Token(),Text(),S1(),S2(),Eval()
{
}

bool mini_regex::compile(const std::string& regexp_str)
{
    _reset();
    regexp = regexp_str;
    lexer();
    return parse();
}


bool mini_regex::match(const std::string& match_str, std::function<void(_regex_result&)> callback)
{
    bool flag = false;
    target = match_str;
    flag = evalute();
    if (callback) callback(regex_result);
    return flag;
}

void mini_regex::_reset()
{
    regexp = "";
    target = "";
    regex_result.matched.clear();
    Code.clear();
    Token.clear();
    Text.clear();
    while (!S1.empty()) S1.pop();
    while (!S2.empty()) S2.pop();
    while (!Eval.empty()) Eval.pop();
}

void mini_regex::lexer()
{
    std::ptrdiff_t _index = 0, _len = regexp.length();
    while (_index < _len)
    {
        switch (regexp[_index])
        {
            case '.':  Token.push_back(TOKEN::ANY);       _index++; break;
            case '+':  Token.push_back(TOKEN::PLUS);      _index++; break;
            case '?':  Token.push_back(TOKEN::QUESTION);  _index++; break;
            case '^':  Token.push_back(TOKEN::BEGIN);     _index++; break;
            case '$':  Token.push_back(TOKEN::END);       _index++; break;
            case '*':  Token.push_back(TOKEN::CLOSURE);   _index++; break;
            case '|':  Token.push_back(TOKEN::OR);        _index++; break;
            case '(':  Token.push_back(TOKEN::LBRACKET);  _index++; break;
            case ')':  Token.push_back(TOKEN::RBRACKET);  _index++; break;

            case '\\':
            {
                _index++; /* skip '\\' */
                switch (regexp[_index])
                {
                    case 'b':
                        Token.push_back(TOKEN::_CHAR);
                        Text.push_back(' ');
                        break;

                    case 'd':
                        Token.push_back(TOKEN::DIGIT);
                        break;

                    default:
                        break;

                }
                _index++;
                break;
            }

            default:
            {
                unsigned int start_pos = _index, end_pos = _index;
                while (end_pos < _len && ((regexp[end_pos] >= 'a' && regexp[end_pos] <= 'z') || (regexp[end_pos] >= 'A' && regexp[end_pos] <= 'Z') || (regexp[end_pos] >= '0' && regexp[end_pos] <= '9')))
                {
                    Text.push_back(regexp[end_pos]);
                    Token.push_back(TOKEN::_CHAR);
                    end_pos++;
                }
                _index = end_pos;
                break;
            }
        }
    }
}

bool mini_regex::parse()
{
    std::ptrdiff_t _index = 0, _len = Token.size();
    std::reverse(Text.begin(), Text.end());

    while (_index < _len)
    {
        switch (Token[_index])
        {
            case TOKEN::_CHAR:
            {
                S2.push(parse_stack_t(TOKEN::_CHAR, 1, Code.size()));
                Code.push_back(CODE_ELM(BYTE_CODE::MATCH, Text.back(), 0));
                Text.pop_back();
                break;
            }

            case TOKEN::ANY: 
                S2.push(parse_stack_t(TOKEN::_CHAR, 1, Code.size()));
                Code.push_back(CODE_ELM(BYTE_CODE::MATCH, TOKEN::ANY, 0)); /* -1 means match any */
                break;

            case TOKEN::DIGIT:
                S2.push(parse_stack_t(TOKEN::_CHAR, 1, Code.size()));
                Code.push_back(CODE_ELM(BYTE_CODE::MATCH, TOKEN::DIGIT, 0));
                break;
            
            case TOKEN::PLUS:
            {
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
            
            case TOKEN::BEGIN: break;
            case TOKEN::END: break;

            case TOKEN::OR:
                /* OR 延后处理 */
                S1.push(parse_stack_t(TOKEN::OR, 2, -1));
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

                Code.insert(Code.begin() + exp.ip, CODE_ELM(BYTE_CODE::SPLIT, 1, exp.n + 1 + 1));
                Code.insert(Code.begin() + exp.ip + 1 + exp.n, CODE_ELM(BYTE_CODE::SPLIT, -exp.n, 1));

                exp.n += 2;
                exp.tk = TOKEN::EXP;
                S2.push(exp);
                break;
            }

            case TOKEN::LBRACKET:
                S1.push(parse_stack_t(TOKEN::LBRACKET, 0, -1));
                S2.push(parse_stack_t(TOKEN::LBRACKET, 0, -1));
                break;

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

                /* (_CHAR,_CHAR,EXP,...) => EXP */
                parse_stack_t exp;
                while (S2.top().tk != TOKEN::LBRACKET)
                {
                    exp.n += S2.top().n;
                    exp.ip = S2.top().ip;
                    S2.pop();
                }
                S2.pop();
                exp.tk = TOKEN::EXP;
                S2.push(exp);
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

bool mini_regex::evalute()
{
    std::ptrdiff_t _code_ip = 0, _code_len = Code.size();
    std::ptrdiff_t _target_start_pos = 0, _target_len = target.length();
    std::ptrdiff_t _matched_index = 0, _matched_len = 0;
    bool is_accept = false;

    while (_target_start_pos < _target_len)
    {
        _code_ip = 0;
        _matched_index = _target_start_pos;
        _matched_len = 0;
        is_accept = false;
        while (!Eval.empty()) Eval.pop(); /* 清空分支栈 */

        while (_matched_index < _target_len && !is_accept && _code_ip < _code_len)
        {
            switch (Code[_code_ip].op)
            {
                case BYTE_CODE::MATCH:
                {
                    int exp_t = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp1);
                    switch (exp_t)
                    {
                        case TOKEN::ANY:
                            goto __match_ok;
                            break;

                        case TOKEN::DIGIT:
                            if (target[_matched_index] >= '0' && target[_matched_index] <= '9')
                                goto __match_ok;
                            else
                                goto __backtrack;
                            break;

                        default:
                            if (target[_matched_index] == (char)exp_t)
                                goto __match_ok;
                            else
                                goto __backtrack;
                            break;
                    }

                    goto __out; /* 防止直接执行 */

                    __backtrack:; /* backtrack */
                    if (!Eval.empty())
                    {
                        _code_ip = Eval.top();
                        Eval.pop();
                    } else {
                        goto __fail_loop;
                    }
                    goto __out; /* 防止直接执行 */

                    __match_ok:; /* match ok */
                    _matched_index += 1;
                    _matched_len += 1;
                    _code_ip++;

                    __out:;
                    break;
                }

                case BYTE_CODE::SPLIT:
                {
                    auto exp1 = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp1),
                         exp2 = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp2);
                    if (exp1 == _code_ip + exp1 && !Eval.empty() && exp2 == Eval.top()) /* 死循环 */
                    {
                        std::cout << " infinite loop." << std::endl;
                        goto __fail_loop;
                    }
                    /* 默认选exp1分支 执行失败则进入exp2分支 */
                    Eval.push(_code_ip + exp2);
                    _code_ip += exp1;
                    break;
                }

                case BYTE_CODE::JMP:
                    _code_ip += reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp1);
                    break;

                case BYTE_CODE::ACCEPT:
                    _code_ip++;
                    is_accept = true;
                    break;

                case BYTE_CODE::HALT:
                    goto __fail_loop;
                    break;

                default:
                    _code_ip++;
                    break;
            }
        }

        if (_matched_index >= _target_len || is_accept)
        {
            /* match success */
            if (_matched_len > 0)
                regex_result.matched.push_back(target.substr(_target_start_pos, _matched_len));
            else
                _matched_len = 1;
            _target_start_pos += _matched_len;
        }
        else
        {
            /* match error */
            __fail_loop:;
            _target_start_pos++;
        }
    }
    return false;
}

void mini_regex::output_code()
{
    std::cout << "Regexp: " << regexp << std::endl;
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
                int exp_t = reinterpret_cast<std::ptrdiff_t>(i.exp1);
                // ... wait for implement
                std::cout << "  MATCH " << (char)exp_t << std::endl;
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
    std::cout << std::endl << std::endl;

}


#endif