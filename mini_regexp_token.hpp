#ifndef MINI_REGEXP_TOKEN_H_
#define MINI_REGEXP_TOKEN_H_

namespace mini_regexp_token
{

    enum TOKEN
    {
        ERR,        /* error */
        STRING,     /* string */
        COMMA,      /* , */
        CLOSURE,    /* * */
        OR,         /* '|' */
        PLUS,       /* '+' */
        QUESTION,   /* '?' */
        LBRACKET,   /* '(' */
        RBRACKET,   /* ')' */
        SQUARE_LBRACKET,   /* '[' */
        SQUARE_RBRACKET,   /* ']' */
        LBRACE,     /* '{' */
        RBRACE,     /* '}' */
        EXP,        /* stack parse */
        ANY = -1,   /* '.' */
        BEGIN = -4,      /* '^' */
        END = -5,        /* '$' */
        INF = -6,
        NONE = -7
    };

    inline int is_ANY(char c, bool dotall = false)
    {
        return dotall ? 1 : !!(c != '\n');
    }

    inline int is_line_break(const std::string& s, int _index)
    {
        if (s.length() > _index + 2 && s[_index] == '\r' && s[_index] == '\n')
            return 2;
        else if (s[_index] == '\r' || s[_index] == '\n')
            return 1;
        else
            return 0;
    }

    inline int is_range_in(char c, char start, char end)
    {
        return (c - start >= 0 && end - c >= 0);
    }

    inline int is_escape_char(char c)
    {
        return (c == '\n'
            || c == '\r'
            || c == '\t'
            || c == '\f'
            || c == '\v'
            || c == '\b'
            || c == '\a'
            );
    }
}

#endif