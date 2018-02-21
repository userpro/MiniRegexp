#ifndef _MINI_REGEXP_H_
#define _MINI_REGEXP_H_

#include <stack>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include "re_token.hpp"
#include "re_code.hpp"
#include "re_lexer.hpp"
#include "re_parser.hpp"
#include "re_vm.hpp"
#include "re_config.hpp"
#include "re_result.hpp"

using namespace mini_regexp_code;
using namespace mini_regexp_token;
using namespace mini_regexp_lexer;
using namespace mini_regexp_parser;
using namespace mini_regexp_vm;
using namespace mini_regexp_config;
using namespace mini_regexp_result;

class mini_regexp
{
public:
    mini_regexp();
    bool compile(const std::string& regexp_str);
    bool match(const std::string& match_str, 
        std::function<void(std::vector<std::string>&, std::vector<std::string>&)> callback = nullptr);
    void output_code();

private:
    RE_Lexer mini_lexer;
    RE_Parser mini_parser;
    RE_VM mini_vm;
    RE_Config mini_config;

    std::string regexp;
    std::string target;

    void _reset();
};

#endif