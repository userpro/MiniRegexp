#ifndef MINI_REGEXP_LEXER_CPP_
#define MINI_REGEXP_LEXER_CPP_

#include "mini_regexp_lexer.hpp"
using namespace mini_regexp_lexer;

RE_Lexer::RE_Lexer() {}

void RE_Lexer::lexer(const std::string& regexp, RE_Config& config)
{
    lexer_init();
    std::ptrdiff_t _index = 0, _len = regexp.length();
    while (_index < _len)
    {
        switch (regexp[_index])
        {
            case ' ':  break;
            case '.':  Token.push_back(TOKEN::ANY);       _index++; break;
            case '+':  Token.push_back(TOKEN::PLUS);      _index++; break;
            case '?':  Token.push_back(TOKEN::QUESTION);  _index++; break;
            case '^':  Token.push_back(TOKEN::BEGIN);     _index++; break;
            case '$':  Token.push_back(TOKEN::END);       _index++; break;
            case '*':  Token.push_back(TOKEN::CLOSURE);   _index++; break;
            case '|':  Token.push_back(TOKEN::OR);        _index++; break;
            case '(':  Token.push_back(TOKEN::LBRACKET);  _index++; break;
            case ')':  Token.push_back(TOKEN::RBRACKET);  _index++; break;
            case ',':  Token.push_back(TOKEN::COMMA);     _index++; break;
            case '{':  Token.push_back(TOKEN::LBRACE);    _index++; break;
            case '}':  Token.push_back(TOKEN::RBRACE);    _index++; break;

            case '[':  
            {
                Token.push_back(TOKEN::SQUARE_LBRACKET);
                std::ptrdiff_t st = _index + 1;
                while (regexp[++_index] != ']');
                Text.push_back(regexp.substr(st, _index - st));
                Token.push_back(TOKEN::STRING);
                break;
            }

            case ']':  Token.push_back(TOKEN::SQUARE_RBRACKET); _index++; break;

            case '\\':
            {
                switch (regexp[_index + 1])
                {
                    case 'b': Token.push_back(TOKEN::STRING); Text.push_back(" ");  break;
                    case 'd': Token.push_back(TOKEN::DIGIT); break;
                    case 's': Token.push_back(TOKEN::SPACE); break;

                    default:
                        /* 加到string中 */
                        goto __lex_string;
                        break;

                }
                _index += 2;
                break;
            }

            default:
            {
                __lex_string:;
                unsigned int start_pos = _index, end_pos = _index;
                while (end_pos < _len 
                    && 
                    (is_escape_char(regexp[end_pos])
                        || is_range_in(regexp[end_pos], 'a', 'z')
                        || is_range_in(regexp[end_pos], 'A', 'Z') 
                        || is_range_in(regexp[end_pos], '0', '9') 
                        || regexp[end_pos] == '_' || regexp[end_pos] == '\\'))
                    end_pos++;
                Text.push_back(regexp.substr(start_pos, end_pos - start_pos));
                Token.push_back(TOKEN::STRING);
                _index = end_pos;
                break;
            }
        }
    }
}

void RE_Lexer::lexer_init()
{
    Token.clear();
    Text.clear();
}

#endif