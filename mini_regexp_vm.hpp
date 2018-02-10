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
    public:
        std::stack<std::ptrdiff_t> Eval;

    public:
        RE_VM();
        bool vm(const std::string& target, std::vector<ByteCode>& Code, std::vector<std::string>& regex_result, RE_Config& config);

    private:
        void vm_init();
        inline int is_line_break(const std::string& s, int _index);
        
    };
}
#endif