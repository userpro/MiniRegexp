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
    mini_lexer.lexer(regexp, mini_config);
    mini_parser.parser(mini_lexer, mini_config);
    return false;
}


bool mini_regexp::match(const std::string& match_str, 
    std::function<void(std::vector<std::string>&, std::vector<std::string>&)> callback)
{
    bool flag = false;
    target = "\2" + match_str + "\3";
    mini_vm.vm(target, mini_parser.Code, mini_config);
    if (callback) callback(mini_vm.regex_result.get_matched(), mini_vm.regex_result.get_sub_matched());
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