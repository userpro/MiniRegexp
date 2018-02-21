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
        ZERO_WIDTH_ASSERT_LBRACKET, /* (? */
        ZERO_WIDTH_ASSERT_RBRACKET, /* ) */
        EXP,        /* stack parse */
        ANY = -1,   /* '.' */
        INF = -2,   /* {n, m} -> m */
        NONE = -3,  /* {n, m} -> m */
        GROUP = -4, /* \1 \2 ... etc */
        /* 零宽断言 */
        FORWARD_PRE_MATCH = -5,
        FORWARD_PRE_MATCH_NOT = -6,
        BACKWORD_PRE_MATCH = -7,
        BACKWORD_PRE_MATCH_NOT = -8,
        NORMAL_PRE_MATCH = -9,
    };
}

#endif