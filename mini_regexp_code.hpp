#ifndef MINI_REGEXP_CODE_H_
#define MINI_REGEXP_CODE_H_

namespace mini_regexp_code
{
    /* addr is a current ip offset */
    enum BYTE_CODE {
        HALT = 0,
        MATCH,  /* match addr */
        SPLIT,  /* split addr1, addr2  if (addr1) else (addr2)   */
        JMP,    /* jmp addr */
        REPEAT, /* repeat n, m (m>=s>=n) */
        REPEND, /* repend */
        RANGE,  /* range _start, _end(_start < c < _end) */
        ACCEPT,
    };

    struct ByteCode
    {
        BYTE_CODE op;
        void *exp1, *exp2;
        ByteCode():op(BYTE_CODE::HALT),exp1(0),exp2(0) {}
        ByteCode(BYTE_CODE _op, void *_exp1, void *_exp2):exp1(_exp1),exp2(_exp2),op(_op) {}
    };
}
#endif