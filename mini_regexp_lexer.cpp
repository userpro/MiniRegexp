#ifndef MINI_REGEXP_LEXER_CPP_
#define MINI_REGEXP_LEXER_CPP_

#include "mini_regexp_lexer.hpp"
using namespace mini_regexp_lexer;
#include <iostream>

RE_Lexer::RE_Lexer() {}

void RE_Lexer::lexer(const std::string& regexp, RE_Config& config)
{
    lexer_init();
    std::ptrdiff_t _index = 0, _len = regexp.length();
    while (_index < _len)
    {
        auto ch = regexp[_index];
        if (mini_keywords.is_normal_token(ch))
        {
            Token.push_back(mini_keywords.get_normal_token(ch));
            _index++;
        }
        else
        {
            switch (ch)
            {
                case '^': 
                    if (config.MULTILINE) lexer_special_char("\2\n\r"); 
                    else lexer_special_char("\2"); 
                    _index++;
                    break;
                
                case '$':
                    if (config.MULTILINE) lexer_special_char("\3\n\r"); 
                    else lexer_special_char("\3"); 
                    _index++;
                    break;

                case '{':
                {
                    Token.push_back(TOKEN::LBRACE);
                    _index = get_close_exp(regexp, _index, '}');
                    break;
                }

                case '[':  
                {
                    Token.push_back(TOKEN::SQUARE_LBRACKET);
                    _index = get_close_exp(regexp, _index, ']');
                    break;
                }

                /* 正则规定的特殊转义字符 */
                case '\\':
                {
                    auto spec_ch = regexp[_index + 1];
                    if (mini_keywords.is_special_char(spec_ch))
                        lexer_special_char(mini_keywords.get_special_char(spec_ch));
                    else
                    {
                        /* 加到string中 */
                        Text.push_back(regexp.substr(_index + 1, 1));
                        Token.push_back(TOKEN::STRING);
                    }
                    _index += 2;
                    break;
                }

                default:
                {
                    unsigned int start_pos = _index, end_pos = _index;
                    while (end_pos < _len && !mini_keywords.is_keyword(regexp[end_pos]))
                        end_pos++;
                    Text.push_back(regexp.substr(start_pos, end_pos - start_pos));
                    Token.push_back(TOKEN::STRING);
                    _index = end_pos;
                    break;
                }
            }
        }
    }
}

inline void RE_Lexer::lexer_init()
{
    Token.clear();
    Text.clear();
}

inline void RE_Lexer::lexer_special_char(std::string s)
{
    Token.push_back(TOKEN::SQUARE_LBRACKET); 
    Token.push_back(TOKEN::STRING);
    Token.push_back(TOKEN::SQUARE_RBRACKET);
    Text.push_back(s);
}

inline std::ptrdiff_t RE_Lexer::get_close_exp(const std::string& regexp, std::ptrdiff_t _index, char _end)
{
    auto st = _index + 1, ed = _index;
    while (regexp[++ed] != _end);
    Text.push_back(regexp.substr(st, ed - st));
    Token.push_back(TOKEN::STRING);
    return ed;
}

#endif