#ifndef MINI_REGEXP_VM_CPP_
#define MINI_REGEXP_VM_CPP_

#include "mini_regexp_vm.hpp"

using namespace mini_regexp_vm;

RE_VM::RE_VM() {}

bool RE_VM::vm(const std::string& target, std::vector<ByteCode>& Code, RE_Config& config)
{
    std::ptrdiff_t _code_ip = 0, _code_len = Code.size();
    std::ptrdiff_t _target_start_pos = 0, _target_len = target.length();
    std::ptrdiff_t _matched_index = 0, _matched_len = 0;
    bool is_accept = false;

    while (_target_start_pos < _target_len)
    {
        vm_init();
        _code_ip = 0;
        _matched_index = _target_start_pos;
        _matched_len = 0;
        is_accept = false;

        __repeat:;

        while (!is_accept && _code_ip < _code_len)
        {
            switch (Code[_code_ip].op)
            {
                case BYTE_CODE::MATCH:
                {
                    /* 匹配超出目标串长度 */
                    if (_matched_index >= _target_len) goto __backtrack;
                    auto exp_t = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp1);
                    switch (exp_t)
                    {
                        case TOKEN::ANY:
                            if (is_ANY(target[_matched_index], config.DOTALL))
                                goto __match_ok;
                            goto __backtrack;
                            break;

                        case TOKEN::DIGIT:
                            if (is_range_in(target[_matched_index], '0', '9'))
                                goto __match_ok;
                            goto __backtrack;
                            break;

                        case TOKEN::SPACE:
                            if (is_SPACE(target[_matched_index]))
                                goto __match_ok;
                            goto __backtrack;
                            break;

                        case TOKEN::BEGIN:
                        {
                            if (_matched_index == 0)
                            {
                                _code_ip++;
                                break;
                            }

                            int is_lb = is_line_break(target, _matched_index);
                            if (config.MULTILINE && is_lb)
                            {
                                _code_ip++;
                                _matched_index += is_lb;
                                break;
                            }
                            goto __fail_loop;
                            break;
                        }

                        case TOKEN::END:
                        {
                            if (_matched_index == _target_len)
                            {
                                _code_ip++;
                                break;
                            }
                            int is_lb = is_line_break(target, _matched_index);
                            if (config.MULTILINE && is_lb)
                            {
                                _code_ip++;
                                _matched_index += is_lb;
                                break;
                            }
                            goto __next_loop;
                            break;
                        }

                        default:
                        {
                            std::string s = reinterpret_cast<const char*>(exp_t);
                            if (target.compare(_matched_index, s.length(), s) == 0)
                            {
                                _matched_index += s.length();
                                _matched_len += s.length();
                                _code_ip++;
                                goto __out;
                            }
                            goto __backtrack;
                            break;
                        }
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
                    if (exp1 == _code_ip + exp1 && !Split_stack.empty() && exp2 == Split_stack.top().ip) /* 死循环 */
                    {
                        std::cout << " infinite loop." << std::endl;
                        goto __next_loop;
                    }
                    /* 默认选exp1分支 执行失败则进入exp2分支 */
                    Split_stack.push(split_stack_t(_code_ip + exp2, _matched_index, _matched_len));
                    _code_ip += exp1;
                    break;
                }

                case BYTE_CODE::REPEAT:
                {
                    auto n = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp1),
                         m = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp2);
                    Repeat_stack.push(repeat_stack_t(_code_ip, n, m));
                    _code_ip++;
                    break;
                }

                case BYTE_CODE::REPEND:
                {
                    if (!Repeat_stack.empty())
                    {
                        repeat_stack_t& rs = Repeat_stack.top();
                        /* {n, m} like {n} {n, } */
                        if (rs.m == TOKEN::NONE || rs.m == TOKEN::INF)
                        {
                            rs.n--;
                            if (rs.n > 0)
                            {
                                _code_ip = rs.ip + 1;
                                goto __repeat;
                            }
                        }
                        else
                        {
                            if (rs.n > rs.m) std::swap(rs.n, rs.m);
                            /* 至少重复n次 */
                            rs.n--; rs.m--;
                            if (rs.n > 0)
                            {
                                _code_ip = rs.ip + 1;
                                goto __repeat;
                            }
                            else
                            {
                                /* 最多重复m次 */
                                if (rs.m > 0)
                                {
                                    Split_stack.push(split_stack_t(_code_ip + 1, _matched_index, _matched_len)); /* REPEND的下一条指令 */
                                    _code_ip = rs.ip + 1;
                                    goto __repeat;
                                }
                            }
                        }
                        Repeat_stack.pop();
                    }
                    _code_ip++;
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
                    goto __next_loop;
                    break;

                default:
                    _code_ip++;
                    break;

            }

            goto __protect_backtrack;
            __backtrack:; /* backtrack */
            if (!Split_stack.empty())
            {
                _code_ip = Split_stack.top().ip;
                _matched_index = Split_stack.top().match_index;
                _matched_len   = Split_stack.top().match_len;
                Split_stack.pop();
            } else {
                goto __next_loop;
            }
            __protect_backtrack:;
            
        }

        if (is_accept)
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
            __next_loop:;
            _target_start_pos++;
        }
    }
    return true;
    __fail_loop:;
    return false;
}

inline void RE_VM::vm_init()
{
    while (!Split_stack.empty()) Split_stack.pop();
    while (!Repeat_stack.empty()) Repeat_stack.pop();
    regex_result.matched.clear();
}

#endif