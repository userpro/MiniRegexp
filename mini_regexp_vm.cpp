#ifndef MINI_REGEXP_VM_CPP_
#define MINI_REGEXP_VM_CPP_

#include "mini_regexp_vm.hpp"
using namespace mini_regexp_vm;

RE_VM::RE_VM() {}

bool RE_VM::vm(const std::string& target, std::vector<ByteCode>& Code, RE_Config& config)
{
    vm_result_init();
    std::ptrdiff_t _code_ip = 0, _code_len = Code.size();
    std::ptrdiff_t _target_start_pos = 0, _target_len = target.length();
    std::ptrdiff_t _matched_index = 0, _matched_len = 0,
                   _sub_matched_start = 0, _sub_matched_len = 0;
    bool is_accept = false;

    while (_target_start_pos < _target_len)
    {
        vm_stack_init();
        _code_ip = 0;
        _matched_index = _target_start_pos;
        _matched_len = 0;
        _sub_matched_start = _sub_matched_len = 0;
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
                    if (exp_t == TOKEN::ANY)
                    {
                        if (is_ANY(target[_matched_index], config.DOTALL))
                            goto __match_ok;
                        goto __backtrack;
                    }
                    else
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
                    }
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
                    Split_stack.push(split_stack_t(_code_ip + exp2,
                                                   _matched_index, _matched_len,
                                                   _sub_matched_start, _sub_matched_len));
                    _code_ip += exp1;
                    break;
                }

                case BYTE_CODE::REPEAT:
                    Repeat_stack.push(repeat_stack_t(_code_ip, reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp1)));
                    _code_ip++;
                    break;

                case BYTE_CODE::REPEND:
                {
                    if (!Repeat_stack.empty())
                    {
                        repeat_stack_t& rs = Repeat_stack.top();
                        if (--rs.n > 0) 
                        {
                            _code_ip = rs.ip + 1;
                            goto __repeat;
                        }
                        Repeat_stack.pop();
                    }
                    _code_ip++; 
                    break;
                }

                case BYTE_CODE::ENTER:
                    _code_ip++;
                    break;

                case BYTE_CODE::LEAVE:
                    _code_ip++;
                    break;

                case BYTE_CODE::RANGE:
                {
                    auto a = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp1),
                         b = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp2);

                    if (is_range_in(target[_matched_index], a, b))
                        goto __match_ok;
                    else
                        goto __backtrack;
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

            goto __protect_match_ok;
            __match_ok:; /* match ok */
            _matched_index += 1;
            _matched_len += 1;
            _code_ip++;
            __protect_match_ok:;

            goto __protect_backtrack;
            __backtrack:; /* backtrack */
            if (!Split_stack.empty())
            {
                _code_ip = Split_stack.top().ip;
                _matched_index = Split_stack.top().match_index;
                _matched_len   = Split_stack.top().match_len;
                _sub_matched_start = Split_stack.top().sub_match_start;
                _sub_matched_len = Split_stack.top().sub_match_len;
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
            {
                regex_result.count++;
                regex_result.matched.push_back(target.substr(_target_start_pos, _matched_len));
            }
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

inline void RE_VM::vm_stack_init()
{
    while (!Split_stack.empty()) Split_stack.pop();
    while (!Repeat_stack.empty()) Repeat_stack.pop();
}

inline void RE_VM::vm_result_init()
{
    regex_result.count = 0;
    regex_result.matched.clear();
}

#endif