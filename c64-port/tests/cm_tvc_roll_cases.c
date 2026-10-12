/* SPDX-License-Identifier: MIT */
#include "cm_tvc_roll_cases.h"
/* Both entries; ROLLFIRE and TEMREG zero, -0, small, raw (the DURATION and
 * jet-selection branches); OGA, OGANOW/OGAPAST, 1/CONACC small and raw so
 * every phase-plane region, the DV limits and the TS overflows are reached;
 * ROLLWORD bits 1 and 2; raw TIME6, A/L/Q, channel 6, unowned words.
 * Channel 13 starts with its trap bits (04000-020000, which yaAGC turns into
 * hidden trap latches and drops) and its device-only bits (010, 01000,
 * 02000) clear; the file only ORs in bit 15. */
static uint32_t next(uint32_t *s){*s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;}
static agc_word classed(uint32_t *s,unsigned k){agc_word v=(agc_word)(next(s)&AGC_MASK);switch(k&7u){case 0:return 0;case 1:return AGC_MASK;case 2:return (agc_word)(v&0777u);case 3:return agc_negate((agc_word)(v&0777u));case 4:return (agc_word)(v&07777u);case 5:return agc_negate((agc_word)(v&07777u));default:return v;}}
void cm_tvc_roll_input(agc_image *m,cm_tvc_roll_io *io,uint32_t n){
    uint32_t seed=UINT32_C(0x7a3d1e95)^(n*UINT32_C(0x9e3779b9));unsigned i;
    if(!seed)seed=1;
    for(i=0;i<2048u;++i)m->words[i]=(agc_word)(next(&seed)&AGC_MASK);
    m->words[0]=(agc_word)next(&seed);m->words[1]=(agc_word)next(&seed);
    m->words[2]=(agc_word)(04000u+(next(&seed)&03777u));
    m->words[3]=03000u;m->words[4]=034000u;m->words[5]=(n&15u)==15u?03676u:03272u;m->words[6]=034006u;m->words[7]=0;
    m->words[03171]=classed(&seed,n>>4);            /* ROLLFIRE */
    m->words[03173]=classed(&seed,n>>7);            /* TEMREG */
    m->words[03264]=classed(&seed,n>>10);           /* OGA */
    m->words[03262]=classed(&seed,n>>13);           /* OGANOW */
    m->words[03263]=(n>>12)&1u?m->words[03262]:classed(&seed,n>>2);   /* OGAPAST */
    m->words[03240]=classed(&seed,n>>5);            /* 1/CONACC */
    io->channel6=(agc_word)(next(&seed)&AGC_MASK);
    io->channel13=(agc_word)(next(&seed)&040765u);
}
