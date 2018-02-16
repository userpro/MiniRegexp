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
        switch (regexp[_index])
        {
            case '.':  Token.push_back(TOKEN::ANY);       _index++; break;
            case '+':  Token.push_back(TOKEN::PLUS);      _index++; break;
            case '?':  Token.push_back(TOKEN::QUESTION);  _index++; break;
            case '*':  Token.push_back(TOKEN::CLOSURE);   _index++; break;
            case '|':  Token.push_back(TOKEN::OR);        _index++; break;
            case '(':  Token.push_back(TOKEN::LBRACKET);  _index++; break;
            case ')':  Token.push_back(TOKEN::RBRACKET);  _index++; break;
            case ',':  Token.push_back(TOKEN::COMMA);     _index++; break;
            case '{':
            {
                Token.push_back(TOKEN::LBRACE);
                get_close_exp(regexp, _index, '}');
                break;
            }

            case '}':  Token.push_back(TOKEN::RBRACE);    _index++; break;

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

            case '[':  
            {
                Token.push_back(TOKEN::SQUARE_LBRACKET);
                get_close_exp(regexp, _index, ']');
                break;
            }

            case ']':  Token.push_back(TOKEN::SQUARE_RBRACKET); _index++; break;

            /* 正则规定的特殊转义字符 */
            case '\\':
            {
                switch (regexp[_index + 1])
                {
                    case 'b': lexer_special_char(" ");    break;
                    case 'B': lexer_special_char("^ ");   break;
                    case 'd': lexer_special_char("0-9");  break;
                    case 'D': lexer_special_char("^0-9"); break;
                    case 's': lexer_special_char(" \f\n\r\t\v");  break;
                    case 'S': lexer_special_char("^ \f\n\r\t\v"); break;
                    case 'w': lexer_special_char("A-Za-z0-9_");   break;
                    case 'W': lexer_special_char("^A-Za-z0-9_");  break;

                    default:
                        /* 加到string中 */
                        Text.push_back(regexp.substr(_index + 1, 1));
                        Token.push_back(TOKEN::STRING);
                        break;

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

inline void RE_Lexer::get_close_exp(const std::string& regexp, std::ptrdiff_t& _index, char _end)
{
    auto st = _index + 1;
    while (regexp[++_index] != _end);
    Text.push_back(regexp.substr(st, _index - st));
    Token.push_back(TOKEN::STRING);
}

#endif