#ifndef MINI_REGEXP_KEYWORDS_H_
#define MINI_REGEXP_KEYWORDS_H_

#include <string>
#include <algorithm>
#include "re_token.hpp"
#include "re_config.hpp"

namespace mini_regexp_keywords
{
    using namespace mini_regexp_token;
    using namespace mini_regexp_config;

    class RE_Keywords
    {
    private:
        /* 正则关键词 */
        const std::string REGEXP_KEYWORDS = ".+?*|,(){}^$[]\\";
        bool KEYWORDS_BOOL[0xff];
        /* 正则(简单操作)关键词到TOKEN转换数组 */
        TOKEN KEYWORDS_NORMAL_TOKEN[0xff];
        /* 正则预定义转义字符 */
        std::string REGEXP_ESCAPE_CHAR[0xff];

    public:
        RE_Keywords()
        {
            std::fill(KEYWORDS_BOOL, KEYWORDS_BOOL + 0xff, false);
            for (int i = 0; i < REGEXP_KEYWORDS.length(); ++i)
                KEYWORDS_BOOL[REGEXP_KEYWORDS[i]] = true;

            std::fill(KEYWORDS_NORMAL_TOKEN, KEYWORDS_NORMAL_TOKEN + 0xff, TOKEN::ERR);
            KEYWORDS_NORMAL_TOKEN['.'] = TOKEN::ANY;
            KEYWORDS_NORMAL_TOKEN['+'] = TOKEN::PLUS;
            KEYWORDS_NORMAL_TOKEN['?'] = TOKEN::QUESTION;
            KEYWORDS_NORMAL_TOKEN['*'] = TOKEN::CLOSURE;
            KEYWORDS_NORMAL_TOKEN['|'] = TOKEN::OR;
            KEYWORDS_NORMAL_TOKEN[')'] = TOKEN::RBRACKET;
            KEYWORDS_NORMAL_TOKEN[','] = TOKEN::COMMA;
            KEYWORDS_NORMAL_TOKEN['}'] = TOKEN::RBRACE;
            KEYWORDS_NORMAL_TOKEN[']'] = TOKEN::SQUARE_RBRACKET;

            /* 默认'[' ']' */
            REGEXP_ESCAPE_CHAR['b'] = " ";
            REGEXP_ESCAPE_CHAR['B'] = "^ ";
            REGEXP_ESCAPE_CHAR['d'] = "0-9";
            REGEXP_ESCAPE_CHAR['D'] = "^0-9";
            REGEXP_ESCAPE_CHAR['s'] = " \f\n\r\t\v";
            REGEXP_ESCAPE_CHAR['S'] = "^ \f\n\r\t\v";
            REGEXP_ESCAPE_CHAR['w'] = "A-Za-z0-9_";
            REGEXP_ESCAPE_CHAR['W'] = "^A-Za-z0-9_";
        }

        bool is_keyword(char c)
        {
            return KEYWORDS_BOOL[c];
        }

        bool is_normal_token(char c)
        {
            return KEYWORDS_NORMAL_TOKEN[c] != TOKEN::ERR;
        }

        TOKEN get_normal_token(char c)
        {
            return KEYWORDS_NORMAL_TOKEN[c];
        }

        bool is_escape_char(char c)
        {
            return REGEXP_ESCAPE_CHAR[c].length() != 0;
        }

        std::string& get_escape_char(char c)
        {
            return REGEXP_ESCAPE_CHAR[c];
        }
        
    };
}

#endif