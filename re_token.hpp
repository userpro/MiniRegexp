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

        INTERN_TOKEN = -120,

        EXP,   /* stack parse */
        ANY,   /* '.' */
        BLANK, /* \b */  NOT_BLANK, /* \B */
        DIGIT, /* \d */  NOT_DIGIT, /* \D */
        SPACE, /* \s */  NOT_SPACE, /* \S */
        WWORD, /* \w */  NOT_WWORD, /* \W */
        
        INF,   /* {n, m} -> m */
        NONE,  /* {n, m} -> m */
        GROUP, /* \1 \2 ... etc */
        /* 零宽断言 */
        FORWARD_PRE_MATCH,
        FORWARD_PRE_MATCH_NOT,
        BACKWORD_PRE_MATCH,
        BACKWORD_PRE_MATCH_NOT,
        NORMAL_PRE_MATCH,
    };
}

#endif