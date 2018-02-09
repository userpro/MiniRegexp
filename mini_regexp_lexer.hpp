#ifndef MINI_REGEXP_LEXER_H_
#define MINI_REGEXP_LEXER_H_

#include <string>
#include <vector>
#include "mini_regexp_token.hpp"

namespace mini_regexp_lexer
{
    using namespace mini_regexp_token;

    class RE_Lexer
    {
    public:
        std::vector<TOKEN> Token; /* lexer */
        std::vector<std::string> Text;  /* TEXT (a|b)* => store a, b */

    public:
        RE_Lexer() {}
        void lexer(const std::string& regexp)
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
                    case '^':  Token.push_back(TOKEN::BEGIN);     _index++; break;
                    case '$':  Token.push_back(TOKEN::END);       _index++; break;
                    case '*':  Token.push_back(TOKEN::CLOSURE);   _index++; break;
                    case '|':  Token.push_back(TOKEN::OR);        _index++; break;
                    case '(':  Token.push_back(TOKEN::LBRACKET);  _index++; break;
                    case ')':  Token.push_back(TOKEN::RBRACKET);  _index++; break;
                    case '[':  Token.push_back(TOKEN::SQUARE_LBRACKET); _index++; break;
                    case ']':  Token.push_back(TOKEN::SQUARE_RBRACKET); _index++; break;
                    case ',':  Token.push_back(TOKEN::COMMA);     _index++; break;
                    case '{':  Token.push_back(TOKEN::LBRACE);    _index++; break;
                    case '}':  Token.push_back(TOKEN::RBRACE);    _index++; break;

                    case '\\':
                    {
                        _index++; /* skip '\\' */
                        switch (regexp[_index])
                        {
                            case 'b': Token.push_back(TOKEN::STRING); Text.push_back(" ");  break;
                            case 'n': Token.push_back(TOKEN::STRING); Text.push_back("\n"); break;
                            case 't': Token.push_back(TOKEN::STRING); Text.push_back("\t"); break;
                            case 'r': Token.push_back(TOKEN::STRING); Text.push_back("\r"); break;
                            case 'f': Token.push_back(TOKEN::STRING); Text.push_back("\f"); break;
                            case 'v': Token.push_back(TOKEN::STRING); Text.push_back("\v"); break;

                            case 'd': Token.push_back(TOKEN::DIGIT); break;
                            case 's': Token.push_back(TOKEN::SPACE); break;

                            default:
                                Token.push_back(TOKEN::STRING);
                                Text.push_back(regexp.substr(_index, 1));
                                break;

                        }
                        _index++;
                        break;
                    }

                    default:
                    {
                        unsigned int start_pos = _index, end_pos = _index;
                        while (end_pos < _len 
                            && ((regexp[end_pos] >= 'a' && regexp[end_pos] <= 'z') 
                                || (regexp[end_pos] >= 'A' && regexp[end_pos] <= 'Z') 
                                || (regexp[end_pos] >= '0' && regexp[end_pos] <= '9') 
                                || regexp[end_pos] == '_'))
                            end_pos++;
                        Text.push_back(regexp.substr(start_pos, end_pos - start_pos));
                        Token.push_back(TOKEN::STRING);
                        _index = end_pos;
                        break;
                    }
                }
            }
        }

    private:
        void lexer_init()
        {
            Token.clear();
            Text.clear();
        }

    };
}

#endif