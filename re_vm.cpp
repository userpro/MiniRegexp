#ifndef MINI_REGEXP_VM_CPP_
#define MINI_REGEXP_VM_CPP_

#include "re_vm.hpp"
using namespace mini_regexp_vm;

RE_VM::RE_VM() {}

bool RE_VM::vm(const std::string& target, std::vector<ByteCode>& Code, RE_Config& config)
{
    vm_init(target, Code);
    vm_result_init();
    return vm_main(target, Code, config);
}

inline void RE_VM::vm_init(const std::string& target, std::vector<ByteCode>& Code)
{
    _code_ip = 0; 
    _code_len = Code.size();
    _target_start_pos = 0; 
    _target_len = target.length();
    _matched_index = 0; 
    _matched_len = 0;
    _sub_matched_start = 0; 
    _sub_matched_len = 0;
}

bool RE_VM::vm_main(const std::string& target,
            std::vector<ByteCode>& Code, RE_Config& config)
{
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
                    if (!vm_match(target, Code, config))
                        goto __next_loop;
                    break;
                }

                case BYTE_CODE::SPLIT:  { vm_split(Code);  break; }
                case BYTE_CODE::REPEAT: { vm_repeat(Code); break; }

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

                case BYTE_CODE::ENTER: { vm_enter();       break; }
                case BYTE_CODE::LEAVE: { vm_leave(target); break; }
                case BYTE_CODE::JMP:   { vm_jmp(Code);     break; }
                
                case BYTE_CODE::RANGE:
                {
                    if (!vm_range(target, Code))
                        goto __next_loop;
                    break;
                }

                /*--- 零宽断言 start ---*/

                case BYTE_CODE::ZERO_WIDTH_ASSERT_ENTER:
                    ZeroWidthAssert_stack.push(
                        zero_width_assert_stack_t(
                            (TOKEN)reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp1),
                            _matched_index, _matched_len));
                    vm_switch_eval_stack();
                    _code_ip++;
                    break;

                case BYTE_CODE::ZERO_WIDTH_ASSERT_LEAVE:
                {
                    vm_zero_width_assert(target);
                    vm_switch_eval_stack();
                    break;
                }

                /*--- 零宽断言 end   ---*/

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
}

inline void RE_VM::vm_stack_init()
{
    while (!Split_stack.empty()) Split_stack.pop();
    while (!Repeat_stack.empty()) Repeat_stack.pop();
    while (!ZeroWidthAssert_stack.empty()) ZeroWidthAssert_stack.pop();
    while (!Sub_Split_stack.empty()) Sub_Split_stack.pop();
}

inline void RE_VM::vm_result_init()
{
    regex_result.count = 0;
    regex_result.sub_matched.clear();
    regex_result.matched.clear();
}

inline void RE_VM::vm_switch_eval_stack()
{
    /* 清理子栈环境 */
    while (!Sub_Split_stack.empty()) Sub_Split_stack.pop();
    /* 恢复栈 */
    std::swap(Split_stack, Sub_Split_stack);
}

inline void RE_VM::vm_split(std::vector<ByteCode>& Code)
{
    auto exp1 = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp1),
         exp2 = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp2);
    // if (exp1 == _code_ip + exp1 && !Split_stack.empty() && exp2 == Split_stack.top().ip) /* 死循环 */
    // {
    //     std::cout << " infinite loop." << std::endl;
    //     goto __next_loop;
    // }
    /* 默认选exp1分支 执行失败则进入exp2分支 */
    Split_stack.push(split_stack_t(_code_ip + exp2,
                                   _target_start_pos,
                                   _matched_index, _matched_len,
                                   _sub_matched_start, _sub_matched_len));
    _code_ip += exp1;
}

inline bool RE_VM::vm_match(const std::string& target,
                            std::vector<ByteCode>& Code,
                            RE_Config& config)
{
    /* 匹配超出目标串长度 */
    if (_matched_index >= _target_len) 
        return _vm_backtrack();

    auto exp_t = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp1);
    /* ANY */
    if (exp_t == TOKEN::ANY)
    {
        if (is_ANY(target[_matched_index], config.DOTALL))
            return _vm_match_ok();
        else
            return _vm_backtrack();
    }
    /* GROUP */
    else if (exp_t == TOKEN::GROUP)
    {
        auto group_number = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp2);
        /* Group */
        if (regex_result.sub_matched.size() > group_number - 1)
        {
            std::string& tmp = regex_result.sub_matched[group_number - 1];
            if (target.compare(_matched_index, tmp.length(), tmp) == 0)
                return _vm_match_ok(tmp.length());
            return _vm_backtrack();
        }
        /* 八进制转义字符 */
        else
        {
            /* 不打算做了 真的烦 哼(ノ=Д=)ノ┻━┻ */
            /* \n \nm \nml */
            auto n = std::to_string(group_number);
            std::cout << "Not implement!" << std::endl;
        }
    }
    /* normal string */
    else
    {
        std::string s = reinterpret_cast<const char*>(exp_t);
        // std::cout << "match: " << target.substr(_matched_index, s.length())
        //     << " " << s << std::endl;
        if (target.compare(_matched_index, s.length(), s) == 0)
            return _vm_match_ok(s.length());
        return _vm_backtrack();
    }
    return true;
}

inline bool RE_VM::_vm_match_ok(std::ptrdiff_t n)
{
    _matched_index += n;
    _matched_len += n;
    _code_ip++;
    return true;
}

inline bool RE_VM::_vm_backtrack()
{
    if (!Split_stack.empty())
    {
        _code_ip = Split_stack.top().ip;
        _target_start_pos = Split_stack.top().target_start_pos;
        _matched_index = Split_stack.top().match_index;
        _matched_len   = Split_stack.top().match_len;
        _sub_matched_start = Split_stack.top().sub_match_start;
        _sub_matched_len = Split_stack.top().sub_match_len;
        Split_stack.pop();
        return true;
    }
    return false;
}

inline void RE_VM::vm_enter()
{
    vm_switch_eval_stack();
    _sub_matched_start = _matched_index;
    _code_ip++;
}

inline void RE_VM::vm_leave(const std::string& target)
{
    vm_switch_eval_stack();
    _sub_matched_len = _matched_index - _sub_matched_start;
    regex_result.sub_matched.push_back(target.substr(_sub_matched_start, _sub_matched_len));
    _sub_matched_start = _sub_matched_len = 0;
    _code_ip++;
}

inline void RE_VM::vm_jmp(std::vector<ByteCode>& Code)
{
    _code_ip += reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp1);
}

inline void RE_VM::vm_repeat(std::vector<ByteCode>& Code)
{
    Repeat_stack.push(repeat_stack_t(_code_ip, reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp1)));
    _code_ip++;
}

inline bool RE_VM::vm_range(const std::string& target, std::vector<ByteCode>& Code)
{
    auto a = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp1),
         b = reinterpret_cast<std::ptrdiff_t>(Code[_code_ip].exp2);

    // std::cout << "range: " << target[_matched_index] << std::endl;
    if (is_range_in(target[_matched_index], a, b))
        return _vm_match_ok();
    else
        return _vm_backtrack();
}

inline void RE_VM::vm_zero_width_assert(const std::string& target)
{
    switch (ZeroWidthAssert_stack.top().tk)
    {
        case TOKEN::FORWARD_PRE_MATCH: case TOKEN::FORWARD_PRE_MATCH_NOT:
            _matched_index = ZeroWidthAssert_stack.top()._matched_index;
            _matched_len = ZeroWidthAssert_stack.top()._matched_len;
            break;
        case TOKEN::BACKWORD_PRE_MATCH: case TOKEN::BACKWORD_PRE_MATCH_NOT:
            _target_start_pos += _matched_len - ZeroWidthAssert_stack.top()._matched_len;
            _matched_len -= _matched_len - ZeroWidthAssert_stack.top()._matched_len;
            break;
        case TOKEN::NORMAL_PRE_MATCH: break;
        default:
            /* ERROR 不应该出现其他类型 */
            break;
    }
    ZeroWidthAssert_stack.pop();
    _code_ip++;
}

#endif