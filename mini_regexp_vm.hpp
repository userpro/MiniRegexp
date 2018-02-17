#ifndef MINI_REGEXP_VM_H_
#define MINI_REGEXP_VM_H_

#include <string>
#include <stack>
#include <vector>
#include <iostream>

#include "mini_regexp_token.hpp"
#include "mini_regexp_code.hpp"
#include "mini_regexp_config.hpp"
#include "mini_regexp_result.hpp"

namespace mini_regexp_vm
{
    using namespace mini_regexp_token;
    using namespace mini_regexp_code;
    using namespace mini_regexp_config;
    using namespace mini_regexp_result;

    class RE_VM
    {
    public:
        RE_Result regex_result;

    private:
        struct split_stack_t
        {
            std::ptrdiff_t ip;
            std::ptrdiff_t match_index, match_len;
            std::ptrdiff_t sub_match_start, sub_match_len;
            split_stack_t():ip(-1),match_index(-1),match_len(0),sub_match_start(0), sub_match_len(0) {}
            split_stack_t(std::ptrdiff_t _ip, std::ptrdiff_t _index, std::ptrdiff_t _len, std::ptrdiff_t _sub_match_start, std::ptrdiff_t _sub_match_len)
                :ip(_ip),match_index(_index),match_len(_len),sub_match_start(_sub_match_start),sub_match_len(_sub_match_len) {}
        };

        struct repeat_stack_t
        {
            std::ptrdiff_t ip;
            std::ptrdiff_t n;
            repeat_stack_t():ip(-1),n(-1) {}
            repeat_stack_t(std::ptrdiff_t _ip, std::ptrdiff_t _n):ip(_ip),n(_n) {}
        };
        std::stack<split_stack_t> Split_stack;
        std::stack<repeat_stack_t> Repeat_stack;

    public:
        RE_VM();
        bool vm(const std::string& target, std::vector<ByteCode>& Code, RE_Config& config);

    private:
        void vm_stack_init();
        void vm_result_init();
        
    };
}
#endif