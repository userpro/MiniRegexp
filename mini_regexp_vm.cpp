#ifndef MINI_REGEXP_VM_CPP_
#define MINI_REGEXP_VM_CPP_

#include "mini_regexp_vm.hpp"

using namespace mini_regexp_vm;

RE_VM::RE_VM() {}

bool RE_VM::vm(const std::string& target, std::vector<ByteCode>& Code, std::vector<std::string>& regex_result, RE_Config& config)
{
    vm_init();

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
                    auto exp_t = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp1);
                    switch (exp_t)
                    {
                        case TOKEN::ANY:
                            if (config.DOTALL || target[_matched_index] != '\n')
                                goto __match_ok;
                            goto __match_no;
                            break;

                        case TOKEN::DIGIT:
                            if (target[_matched_index] >= '0' && target[_matched_index] <= '9')
                                goto __match_ok;
                            goto __backtrack;
                            break;

                        case TOKEN::SPACE:
                            if (target[_matched_index] != '\f' 
                                && target[_matched_index] != '\n' 
                                && target[_matched_index] != '\r' 
                                && target[_matched_index] != '\t' 
                                && target[_matched_index] != '\v')
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
                            goto __match_no;
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

                    __backtrack:; /* backtrack */
                    if (!Eval.empty())
                    {
                        _code_ip = Eval.top();
                        Eval.pop();
                    } else {
                        goto __match_no;
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
                        goto __match_no;
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
                    goto __match_no;
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
                regex_result.push_back(target.substr(_target_start_pos, _matched_len));
            else
                _matched_len = 1;
            _target_start_pos += _matched_len;
        }
        else
        {
            /* match error */
            __match_no:;
            _target_start_pos++;
        }
    }
    return true;
    __fail_loop:;
    return false;
}

void RE_VM::vm_init()
{
    while (!Eval.empty()) Eval.pop();
}

inline int RE_VM::is_line_break(const std::string& s, int _index)
{
    if (s.length() > _index + 2 && s[_index] == '\r' && s[_index] == '\n')
        return 2;
    else if (s[_index] == '\r' || s[_index] == '\n')
        return 1;
    else
        return 0;
}

#endif