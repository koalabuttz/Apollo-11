/* SPDX-License-Identifier: MIT. Shared INPUT enumeration and binary transport.
 * ARITH_* implementations are selected separately in native/reference programs. */
#include <stdio.h>
#include <stdint.h>
static void emit_word(uint16_t value) {
    fputc((int)(value & 255u),stdout); fputc((int)(value >> 8),stdout);
}
static int arithmetic_stream(void) {
    uint32_t a,b;
    static const uint16_t boundary[16]={0,1,2,077,0100,0777,01000,037776,
        037777,040000,040001,076777,077676,077775,077776,077777};
    for(a=0;a<32768u;++a) emit_word((uint16_t)ARITH_EXTEND((uint16_t)a));
    for(a=0;a<65536u;++a) emit_word((uint16_t)ARITH_CORRECT((uint16_t)a));
    for(a=0;a<32768u;++a)
        for(b=0;b<16u;++b)
            emit_word((uint16_t)ARITH_ADD(ARITH_EXTEND((uint16_t)a),ARITH_EXTEND(boundary[b])));
    return ferror(stdout) ? 1 : 0;
}
