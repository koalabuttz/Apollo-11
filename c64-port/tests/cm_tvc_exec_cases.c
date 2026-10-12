/* SPDX-License-Identifier: MIT */
#include "cm_tvc_exec_cases.h"
/* Every entry; FLAGWRD6 TVC/RCS states; engine on/off (channel 11 bit 13);
 * each CCS class of VCNTR and CNTR, small and raw; REPFRAC sign; LEM on/off;
 * raw trims, masses, counters, A/L/Q, IRQ and unowned words. */
static const agc_word entry[12]={02720u,02730u,02746u,02750u,02773u,02775u,03003u,03011u,03040u,03043u,03121u,03132u};
static uint32_t next(uint32_t *s){*s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;}
static agc_word classed(uint32_t *s,unsigned k){agc_word v=(agc_word)(next(s)&AGC_MASK);switch(k&7u){case 0:return 0;case 1:return AGC_MASK;case 2:return (agc_word)(1u+next(s)%20u);case 3:return agc_negate((agc_word)(1u+next(s)%20u));case 4:return 1u;default:return v;}}
void cm_tvc_exec_input(agc_image *m,agc_word *irq,agc_word *ch11,uint32_t n){
    uint32_t seed=UINT32_C(0x2545f491)^(n*UINT32_C(0x9e3779b9));unsigned i;
    if(!seed)seed=1;
    for(i=0;i<2048u;++i)m->words[i]=(agc_word)(next(&seed)&AGC_MASK);
    m->words[0]=(agc_word)next(&seed);m->words[1]=(agc_word)next(&seed);
    m->words[2]=(agc_word)(04000u+(next(&seed)&03777u));
    m->words[3]=03000u;m->words[4]=034000u;m->words[5]=entry[n%12u];m->words[6]=034006u;m->words[7]=0;
    if((n>>4)&1u)m->words[0102]=(agc_word)((m->words[0102]&017777u)|(((n>>5)&3u)<<13));
    m->words[03243]=classed(&seed,n>>7);          /* VCNTR */
    m->words[03047]=classed(&seed,n>>10);         /* CNTR */
    if((n>>13)&1u)m->words[03242]=classed(&seed,n>>14); /* REPFRAC */
    *ch11=(agc_word)(next(&seed)&AGC_MASK);if((n>>3)&1u)*ch11^=010000u;
    *irq=(agc_word)((n>>15)&1u);
}
