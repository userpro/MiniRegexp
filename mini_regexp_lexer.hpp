#ifndef MINI_REGEXP_LEXER_H_
#define MINI_REGEXP_LEXER_H_

#include <string>
#include <vector>
#include "mini_regexp_token.hpp"
#include "mini_regexp_config.hpp"

namespace mini_regexp_lexer
{
    using namespace mini_regexp_token;
    using namespace mini_regexp_config;

    class RE_Lexer
    {
    public:
        std::vector<TOKEN> Token; /* lexer */
        std::vector<std::string> Text;  /* TEXT (a|b)* => store a, b */

    public:
        RE_Lexer();
        void lexer(const std::string& regexp, RE_Config& config);

    private:
        void lexer_init();
        void lexer_special_char(std::string s);

    };
}

#endif