/* SPDX-License-Identifier: MIT */
#include "cm_tvc_init_cases.h"
/* Every entry; CSM/LM (DAPDATR1 bit 14) and attitude-error init (bit 13)
 * set and clear; engine on/off (channel 11 bit 13); ERRBTMP small, zero
 * and raw around the 1/ATTLIM limit; IXX, KTLX/I, IAVG/TLX small or raw so
 * the gain TS words overflow sometimes; raw EMDOT, CSMMASS, trims, A/L/Q,
 * interrupt enable and unowned words. S40.15 gets a fixed-fixed Q. */
static const agc_word entry[9]={02147u,02152u,02171u,02176u,02324u,02343u,02351u,02361u,02462u};
static uint32_t next(uint32_t *s){*s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;}
static agc_word smallish(uint32_t *s,unsigned k){agc_word v=(agc_word)(next(s)&AGC_MASK);switch(k&3u){case 0:return (agc_word)(v&0777u);case 1:return agc_negate((agc_word)(v&0777u));case 2:return 0;default:return v;}}
void cm_tvc_init_input(agc_image *m,cm_tvc_init_io *io,uint32_t n){
    uint32_t seed=UINT32_C(0x51ed2701)^(n*UINT32_C(0x9e3779b9));unsigned i;
    if(!seed)seed=1;
    for(i=0;i<2048u;++i)m->words[i]=(agc_word)(next(&seed)&AGC_MASK);
    m->words[0]=(agc_word)next(&seed);m->words[1]=(agc_word)next(&seed);
    m->words[2]=(agc_word)(04000u+(next(&seed)&03777u));
    m->words[3]=03000u;m->words[4]=036000u;m->words[5]=entry[n%9u];m->words[6]=036006u;m->words[7]=0;
    m->words[03066]=(agc_word)((m->words[03066]&047777u)|((((n/9u)>>0)&3u)<<12));   /* DAPDATR1 bits 13, 14 */
    m->words[03302]=smallish(&seed,(n/9u)>>2);                                     /* ERRBTMP */
    m->words[03303]=smallish(&seed,(n/9u)>>4);                                     /* ERRBTMP +1 */
    m->words[03070]=smallish(&seed,(n/9u)>>6);                                     /* IXX */
    m->words[03236]=smallish(&seed,(n/9u)>>8);                                     /* KTLX/I */
    m->words[03072]=smallish(&seed,(n/9u)>>10);                                    /* IAVG/TLX */
    io->channel11=(agc_word)(next(&seed)&AGC_MASK);if(((n/9u)>>12)&1u)io->channel11^=010000u;
    io->interrupt_enabled=(agc_word)(((n/9u)>>13)&1u);
}
