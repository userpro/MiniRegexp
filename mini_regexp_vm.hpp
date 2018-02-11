#ifndef MINI_REGEXP_VM_H_
#define MINI_REGEXP_VM_H_

#include <string>
#include <stack>
#include <vector>
#include <iostream>

#include "mini_regexp_token.hpp"
#include "mini_regexp_code.hpp"
#include "mini_regexp_config.hpp"

namespace mini_regexp_vm
{
    using namespace mini_regexp_token;
    using namespace mini_regexp_code;
    using namespace mini_regexp_config;

    class RE_VM
    {
    private:
        struct split_stack_t
        {
            std::ptrdiff_t ip;
            std::ptrdiff_t match_index, match_len;
            split_stack_t():ip(-1),match_index(-1),match_len(0) {}
            split_stack_t(std::ptrdiff_t _ip, std::ptrdiff_t _index, std::ptrdiff_t _len):ip(_ip),match_index(_index),match_len(_len) {}
        };

        struct repeat_stack_t
        {
            std::ptrdiff_t ip;
            std::ptrdiff_t n,m;
            repeat_stack_t():ip(-1),n(-1),m(-1) {}
            repeat_stack_t(std::ptrdiff_t _ip, std::ptrdiff_t _n, std::ptrdiff_t _m):ip(_ip),n(_n),m(_m) {}
        };
        std::stack<split_stack_t> Split_stack;
        std::stack<repeat_stack_t> Repeat_stack;

    public:
        RE_VM();
        bool vm(const std::string& target, std::vector<ByteCode>& Code, std::vector<std::string>& regex_result, RE_Config& config);

    private:
        inline void vm_init();
        inline int is_line_break(const std::string& s, int _index);
        
    };
}
#endif