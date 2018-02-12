#ifndef _MINI_REGEXP_CPP_
#define _MINI_REGEXP_CPP_

#include "mini_regexp.hpp"

mini_regexp::mini_regexp()
    :regexp(""),target("")
{}

bool mini_regexp::compile(const std::string& regexp_str)
{
    _reset();
    regexp = regexp_str;
    mini_lexer.lexer(regexp_str, mini_config);
    mini_parser.parser(mini_lexer, mini_config);
    return false;
}


bool mini_regexp::match(const std::string& match_str, std::function<void(std::vector<std::string>&)> callback)
{
    bool flag = false;
    target = match_str;
    mini_vm.vm(match_str, mini_parser.Code, mini_config);
    if (callback) callback(mini_vm.regex_result.matched);
    return flag;
}

void mini_regexp::_reset()
{
    regexp = "";
    target = "";
}

void mini_regexp::output_code()
{
    std::cout << "RegExp: " << regexp << std::endl;
    mini_parser.output_code();
}

#endif