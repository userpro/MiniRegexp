#ifndef MINI_REGEXP_LEXER_H_
#define MINI_REGEXP_LEXER_H_

#include <string>
#include <vector>
#include "re_token.hpp"
#include "re_config.hpp"
#include "re_keywords.hpp"
#include "re_common.hpp"

namespace mini_regexp_lexer
{
    using namespace mini_regexp_token;
    using namespace mini_regexp_config;
    using namespace mini_regexp_keywords;
    using namespace mini_regexp_common;

    class RE_Lexer
    {
    public:
        std::vector<TOKEN> Token; /* lexer */
        std::vector<std::string> Text;  /* TEXT (a|b)* => store a, b */

    private:
        RE_Keywords mini_keywords;

    public:
        RE_Lexer();
        void lexer(const std::string& regexp, RE_Config& config);

    private:
        void lexer_init();
        void lexer_main(const std::string& s, std::ptrdiff_t _start, std::ptrdiff_t _end, RE_Config& config);
        std::ptrdiff_t lexer_special_token(const std::string& regexp, std::ptrdiff_t _index, std::ptrdiff_t _len, RE_Config& config);
        
        void lexer_predefined_char(std::string s);
        std::ptrdiff_t lexer_get_digit(const std::string& regexp, std::string& num, std::ptrdiff_t _index);
        std::ptrdiff_t lexer_escape_char(const std::string& regexp, std::ptrdiff_t _index);
        std::ptrdiff_t lexer_get_close_exp(const std::string& regexp, std::ptrdiff_t _index, char _end);
        std::ptrdiff_t lexer_zero_width_assert(const std::string& regexp, std::ptrdiff_t _index, RE_Config& config);

    };
}

#endif