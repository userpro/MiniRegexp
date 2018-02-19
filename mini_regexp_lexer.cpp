#ifndef MINI_REGEXP_LEXER_CPP_
#define MINI_REGEXP_LEXER_CPP_
#include <iostream>
#include "mini_regexp_lexer.hpp"
using namespace mini_regexp_lexer;

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
            _index = lexer_special_token(regexp, _index, _len, config);
        }
    }
}

inline void RE_Lexer::lexer_init()
{
    Token.clear();
    Text.clear();
}

inline void RE_Lexer::lexer_predefined_char(std::string s)
{
    Token.push_back(TOKEN::SQUARE_LBRACKET); 
    Token.push_back(TOKEN::STRING);
    Token.push_back(TOKEN::SQUARE_RBRACKET);
    Text.push_back(s);
}

inline std::ptrdiff_t RE_Lexer::lexer_special_token(const std::string& regexp, std::ptrdiff_t _index, std::ptrdiff_t _len, RE_Config& config)
{
    auto ch = regexp[_index];
    switch (ch)
    {
        case '(':
            // ... 
            /* 零宽断言 zero width assert (?:) (?=) (?!) (?<=) (?<!) */
            Token.push_back(TOKEN::LBRACKET);
            _index++;
            break;

        case '^': 
            if (config.MULTILINE) lexer_predefined_char("\2\n\r"); 
            else lexer_predefined_char("\2"); 
            _index++;
            break;
        
        case '$':
            if (config.MULTILINE) lexer_predefined_char("\3\n\r"); 
            else lexer_predefined_char("\3"); 
            _index++;
            break;

        case '{':
        {
            Token.push_back(TOKEN::LBRACE);
            _index = lexer_get_close_exp(regexp, _index, '}');
            break;
        }

        case '[':  
        {
            Token.push_back(TOKEN::SQUARE_LBRACKET);
            _index = lexer_get_close_exp(regexp, _index, ']');
            break;
        }

        /* 正则规定的特殊转义字符 */
        case '\\':
        {
            _index = lexer_special_char(regexp, _index);
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
    return _index;
}


std::ptrdiff_t RE_Lexer::lexer_special_char(const std::string& regexp, std::ptrdiff_t _index)
{
    auto spec_ch = regexp[++_index];
    if (mini_keywords.is_special_char(spec_ch))
    {
        lexer_predefined_char(mini_keywords.get_special_char(spec_ch));
        return _index + 1;
    }
    else
    {
        std::string num;
        switch (spec_ch)
        {
            /* 八进制 \0XXX */
            case '0':
            {
                std::string s;
                num = regexp.substr(_index + 1, 3);
                Text.push_back(s + char(str2oct(num)));

                std::cout << str2oct(num) << std::endl;
                Token.push_back(TOKEN::STRING);
                _index += 4;
                break;
            }
            /* 十进制 \X... */
            case'1':case'2':case'3':case'4':case'5':
            case'6':case'7':case'8':case'9':
                _index = lexer_get_digit(regexp, num, _index);
                Text.push_back(num);
                Token.push_back(TOKEN::GROUP);
                break;
            case 'x': /* 16进制: \xXX */
            {
                std::string s;
                num = regexp.substr(_index + 1, 2);
                Text.push_back(s + char(str2hex(num)));
                Token.push_back(TOKEN::STRING);
                _index += 3;
                break;
            }

            case 'u': /* Unicode \uXXXX 拆成2字节匹配 */
            {
                std::string s1, s2;
                num = regexp.substr(_index + 1, 2);
                auto num2 = regexp.substr(_index + 3, 2);
                Text.push_back(s1 + char(str2hex(num)));
                Token.push_back(TOKEN::STRING);
                Text.push_back(s2 + char(str2hex(num2)));
                Token.push_back(TOKEN::STRING);
                _index += 5;
                break;
            }

            default:
                /* 加到string中 \X */
                Text.push_back(regexp.substr(_index, 1));
                Token.push_back(TOKEN::STRING);
                _index += 1;
                break;
        }
    }
    return _index;
}


inline std::ptrdiff_t RE_Lexer::lexer_get_digit(const std::string& regexp, std::string& num, std::ptrdiff_t _index)
{
    auto _start = _index;
    while (is_range_in(regexp[_index++], '0', '9'))
        ;
    num = regexp.substr(_start, _index - _start);
    return _index;
}

inline std::ptrdiff_t RE_Lexer::lexer_get_close_exp(const std::string& regexp, std::ptrdiff_t _index, char _end)
{
    auto st = _index + 1, ed = _index;
    while (regexp[++ed] != _end);
    Text.push_back(regexp.substr(st, ed - st));
    Token.push_back(TOKEN::STRING);
    return ed;
}

#endif