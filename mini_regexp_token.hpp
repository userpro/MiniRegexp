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
        BEGIN,      /* '^' */
        END,        /* '$' */
        LBRACKET,   /* '(' */
        RBRACKET,   /* ')' */
        SQUARE_LBRACKET,   /* '[' */
        SQUARE_RBRACKET,   /* ']' */
        LBRACE,     /* '{' */
        RBRACE,     /* '}' */
        EXP,        /* stack parse */
        ANY = -1,   /* '.' */
        DIGIT = -2, /* \d <=> [0-9] */
        SPACE = -3, /* \s <=> [ \f\n\r\t\v] */
    };
}

#endif