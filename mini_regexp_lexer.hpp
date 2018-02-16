#ifndef MINI_REGEXP_LEXER_H_
#define MINI_REGEXP_LEXER_H_

#include <string>
#include <vector>
#include "mini_regexp_token.hpp"
#include "mini_regexp_config.hpp"
#include "mini_regexp_keywords.hpp"

namespace mini_regexp_lexer
{
    using namespace mini_regexp_token;
    using namespace mini_regexp_config;
    using namespace mini_regexp_keywords;

    class RE_Lexer
    {
    public:
        std::vector<TOKEN> Token; /* lexer */
        std::vector<std::string> Text;  /* TEXT (a|b)* => store a, b */

    public:
        RE_Lexer();
        void lexer(const std::string& regexp, RE_Config& config);

    private:
        RE_Keywords mini_keywords;

        void lexer_init();
        void lexer_special_char(std::string s);

        inline void get_close_exp(const std::string& regexp, std::ptrdiff_t& _index, char _end);

    };
}

#endif