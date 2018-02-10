#ifndef MINI_REGEXP_CONFIG_H_
#define MINI_REGEXP_CONFIG_H_

namespace mini_regexp_config
{
    class RE_Config
    {
    public:
        bool CASE_INSENSITIVE;  /* (?i)：忽略大小写 */
        bool COMMENTS;          /* (?x)：忽略空格字符 */
        bool DOTALL;            /* (?s)：.匹配任意字符，包括换行符 */
        bool MULTILINE;         /* (?m)：多行模式 */
        bool UNICODE_CASE;      /* (?u)：对Unicode符大小写不敏感，必须启用CASE_INSENSITIVE */
        bool UNIX_LINES;        /* (?d)：只有'\n'才被认作一行的中止 */
        RE_Config():
            CASE_INSENSITIVE(false),
            COMMENTS(false),
            DOTALL(false),
            MULTILINE(true),
            UNICODE_CASE(false),
            UNIX_LINES(true)
        {}
    };
}

#endif