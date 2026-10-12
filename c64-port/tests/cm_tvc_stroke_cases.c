/* SPDX-License-Identifier: MIT */
#include "cm_tvc_stroke_cases.h"
/* Every entry; CSM/LM bit 14 of DAPDATR1 set and clear; CARD -4..-1; each
 * CCS class of REVS; CADDY near the -1 to +0 step, small and raw; N small
 * and raw (DOUBLE overflow); raw STROKER, ESTROKER, TVCPITCH, A/L/Q,
 * channel 14, interrupt enable and unowned words. Channel 14 starts with
 * its gyro and IMU CDU drive bits (01000, 070000) clear: yaAGC's device
 * model acts on those by itself, and this file only ORs in bit 11. */
static const agc_word entry[4]={03501u,03533u,03541u,03542u};
static uint32_t next(uint32_t *s){*s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;}
static agc_word classed(uint32_t *s,unsigned k){agc_word v=(agc_word)(next(s)&AGC_MASK);switch(k&7u){case 0:return 0;case 1:return AGC_MASK;case 2:return (agc_word)(1u+next(s)%10u);case 3:return agc_negate((agc_word)(1u+next(s)%10u));case 4:return agc_negate(1u);default:return v;}}
void cm_tvc_stroke_input(agc_image *m,cm_tvc_stroke_io *io,uint32_t n){
    uint32_t seed=UINT32_C(0x3b1f7c2d)^(n*UINT32_C(0x9e3779b9));unsigned i;
    if(!seed)seed=1;
    for(i=0;i<2048u;++i)m->words[i]=(agc_word)(next(&seed)&AGC_MASK);
    m->words[0]=(agc_word)next(&seed);m->words[1]=(agc_word)next(&seed);
    m->words[2]=(agc_word)(04000u+(next(&seed)&03777u));
    m->words[3]=03000u;m->words[4]=040000u;m->words[5]=entry[n%4u];m->words[6]=040006u;m->words[7]=0;
    m->words[03066]=(agc_word)((m->words[03066]&057777u)|(((n>>2)&1u)<<13));   /* DAPDATR1 bit 14 */
    m->words[03261]=agc_negate((agc_word)(1u+((n>>3)&3u)));                     /* CARD */
    m->words[03260]=classed(&seed,n>>5);                                        /* REVS */
    m->words[03255]=classed(&seed,n>>8);                                        /* CADDY */
    if((n>>11)&1u)m->words[03256]=classed(&seed,n>>12);                         /* N */
    io->channel14=(agc_word)(next(&seed)&AGC_MASK&~071000u);
    io->interrupt_enabled=(agc_word)((n>>15)&1u);
}
