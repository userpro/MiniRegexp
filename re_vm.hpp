#ifndef MINI_REGEXP_VM_H_
#define MINI_REGEXP_VM_H_

#include <string>
#include <stack>
#include <vector>
#include <iostream>

#include "re_token.hpp"
#include "re_code.hpp"
#include "re_config.hpp"
#include "re_result.hpp"
#include "re_common.hpp"

namespace mini_regexp_vm
{
    using namespace mini_regexp_token;
    using namespace mini_regexp_code;
    using namespace mini_regexp_config;
    using namespace mini_regexp_result;
    using namespace mini_regexp_common;

    class RE_VM
    {
    public:
        RE_Result regex_result;

    private:
        struct split_stack_t
        {
            std::ptrdiff_t ip;
            std::ptrdiff_t target_start_pos;
            std::ptrdiff_t match_index, match_len;
            std::ptrdiff_t sub_match_start, sub_match_len;
            split_stack_t():ip(-1),target_start_pos(0),match_index(-1),match_len(0),sub_match_start(0), sub_match_len(0) {}
            split_stack_t(std::ptrdiff_t _ip, 
                          std::ptrdiff_t _start_pos,
                          std::ptrdiff_t _index, 
                          std::ptrdiff_t _len, 
                          std::ptrdiff_t _sub_match_start, 
                          std::ptrdiff_t _sub_match_len)
                :ip(_ip),target_start_pos(_start_pos),match_index(_index),match_len(_len),sub_match_start(_sub_match_start),sub_match_len(_sub_match_len) {}
        };

        struct repeat_stack_t
        {
            std::ptrdiff_t ip;
            std::ptrdiff_t n;
            repeat_stack_t():ip(-1),n(-1) {}
            repeat_stack_t(std::ptrdiff_t _ip, std::ptrdiff_t _n):ip(_ip),n(_n) {}
        };

        struct zero_width_assert_stack_t
        {
            TOKEN tk;
            std::ptrdiff_t _matched_index, _matched_len;
            zero_width_assert_stack_t():tk(TOKEN::ERR),_matched_index(0),_matched_len(0) {}
            zero_width_assert_stack_t(TOKEN _tk, std::ptrdiff_t minx, std::ptrdiff_t mlen):tk(_tk),_matched_index(minx),_matched_len(mlen) {}
        };

        /* Sub  Split Stack给子表达式创建独立栈空间 在离开代码段时清理 */
        std::stack<split_stack_t> Split_stack, Sub_Split_stack;
        std::stack<repeat_stack_t> Repeat_stack;
        std::stack<zero_width_assert_stack_t> ZeroWidthAssert_stack;


        std::ptrdiff_t _matched_index, _matched_len,
                       _sub_matched_start, _sub_matched_len,
                       _code_ip, _code_len,
                       _target_start_pos, _target_len;

    public:
        RE_VM();
        bool vm(const std::string& target, 
            std::vector<ByteCode>& Code, RE_Config& config);

    private:
        void vm_init(const std::string& target, std::vector<ByteCode>& Code);
        bool vm_main(const std::string& target,
            std::vector<ByteCode>& Code, RE_Config& config);
        void vm_stack_init();
        void vm_result_init();
        void vm_switch_eval_stack();

        bool vm_match(const std::string& target,
                      std::vector<ByteCode>& Code,
                      RE_Config& config);

        bool _vm_match_ok(std::ptrdiff_t n = 1);
        bool _vm_backtrack();

        void vm_enter();
        void vm_leave(const std::string& target);
        void vm_jmp(std::vector<ByteCode>& Code);

        void vm_split(std::vector<ByteCode>& Code);
        void vm_repeat(std::vector<ByteCode>& Code);
        bool vm_range(const std::string& target, std::vector<ByteCode>& Code);

        void vm_zero_width_assert(const std::string& target);
        
    };
}
#endif