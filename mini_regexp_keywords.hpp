#ifndef MINI_REGEXP_KEYWORDS_H_
#define MINI_REGEXP_KEYWORDS_H_

#include <string>

namespace mini_regexp_keywords
{
    class RE_Keywords
    {
    public:
        const std::string REGEXP_KEYWORDS = ".+?*|,(){}^$[]\\";
        bool KEYWORDS_BOOL[0xff];

    public:
        RE_Keywords()
        {
            std::fill(KEYWORDS_BOOL, KEYWORDS_BOOL + 0xff, false);
            for (int i = 0; i < REGEXP_KEYWORDS.length(); ++i)
                KEYWORDS_BOOL[REGEXP_KEYWORDS[i]] = true;
        }

        bool is_keyword(char c)
        {
            return KEYWORDS_BOOL[c];
        }
        
    };
}

#endif