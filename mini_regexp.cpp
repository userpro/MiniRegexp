#ifndef _MINI_REGEXP_CPP
#define _MINI_REGEXP_CPP

#include "mini_regexp.hpp"

mini_regexp::mini_regexp()
    :regexp(""),target(""),Multiline(true)
{}

bool mini_regexp::compile(const std::string& regexp_str)
{
    _reset();
    regexp = regexp_str;
    mini_lexer.lexer(regexp_str);
    mini_parser.parser(mini_lexer);
    return false;
}


bool mini_regexp::match(const std::string& match_str, std::function<void(Result&)> callback)
{
    bool flag = false;
    target = match_str;
    mini_vm.vm(match_str, mini_parser.Code, regex_result.matched);
    if (callback) callback(regex_result);
    return flag;
}

void mini_regexp::_reset()
{
    regexp = "";
    target = "";
    regex_result.matched.clear();
}

void mini_regexp::output_code()
{
    std::cout << "RegExp: " << regexp << std::endl;
    mini_parser.output_code();
}

#endif