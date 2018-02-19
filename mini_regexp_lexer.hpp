#ifndef MINI_REGEXP_LEXER_H_
#define MINI_REGEXP_LEXER_H_

#include <string>
#include <vector>
#include "mini_regexp_token.hpp"
#include "mini_regexp_config.hpp"
#include "mini_regexp_keywords.hpp"
#include "mini_regexp_common.hpp"

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
        std::ptrdiff_t lexer_special_token(const std::string& regexp, std::ptrdiff_t _index, std::ptrdiff_t _len, RE_Config& config);
        
        void lexer_predefined_char(std::string s);
        std::ptrdiff_t lexer_get_digit(const std::string& regexp, std::string& num, std::ptrdiff_t _index);
        std::ptrdiff_t lexer_special_char(const std::string& regexp, std::ptrdiff_t _index);
        std::ptrdiff_t lexer_get_close_exp(const std::string& regexp, std::ptrdiff_t _index, char _end);

    };
}

#endif