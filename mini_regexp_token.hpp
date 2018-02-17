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
        INF = -2,
        NONE = -3
    };

}

#endif