/* SPDX-License-Identifier: MIT */
#include "cm_entry_dap_cases.h"
/* Every entry, ATTRATES and its SPSIN/SPCOS returns weighted; A, L, Q raw
 * (overflowed A at the SPSIN/SPCOS returns takes their TS skips); the CM
 * flag word, IMODES33, the CDUs, gimbal angles, attitude and rate words,
 * SR, TIME1, PIPTIME and the channels raw; JETEM, GAMDOT, CALFA, SALFA,
 * OUTTAG and the jet timers zero, -0, small, large and raw, each class
 * drawn on its own; PREL, ROLLC and ROLL/180 a quarter of the time +0 or
 * -0 (the +0/-0 ways of CCS -VT/180 and LCX/360); P63FLAG -1, +0, -0, +1 or raw; JETAG -1..+1, CMDAPMOD -1, +0,
 * -0 or +1 and SW/NDX -13..+13 or -0 (the values the file keeps). */
static const agc_word entry[34]={02434u,02446u,02476u,02500u,02507u,02507u,02540u,02543u,02551u,02607u,02631u,02641u,
                                 02665u,02665u,02700u,02712u,02724u,02724u,03056u,03064u,03064u,03207u,03215u,03642u,03650u,03717u,
                                 03612u,03616u,03620u,03627u,03636u,03672u,03716u,03732u};
static uint32_t next(uint32_t *s){*s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;}
static agc_word classed(uint32_t *s,unsigned k){agc_word v=(agc_word)(next(s)&AGC_MASK);switch(k&7u){case 0:return 0;case 1:return AGC_MASK;case 2:return (agc_word)(v&0777u);case 3:return agc_negate((agc_word)(v&0777u));case 4:return (agc_word)(v&017777u);case 5:return agc_negate((agc_word)(v&017777u));default:return v;}}
static agc_word small(uint32_t *s,int lo,int hi){int v=lo+(int)(next(s)%(uint32_t)(hi-lo+1));return v<0?agc_negate((agc_word)-v):(agc_word)v;}
#define E6(k) (03000u+(k)-01400u)
void cm_entry_dap_input(agc_image *m,cm_entry_dap_io *io,uint32_t n){
    static const unsigned classes[]={01720u,01723u,01506u,01507u,01611u,01621u,01605u,01607u,01622u,01606u,01610u,01613u,01614u,01567u};
    uint32_t seed=UINT32_C(0x6a09e667)^(n*UINT32_C(0x9e3779b9));unsigned i;agc_word z;
    if(!seed)seed=1;
    for(i=0;i<2048u;++i)m->words[i]=(agc_word)(next(&seed)&AGC_MASK);
    z=entry[n%34u];
    m->words[0]=(agc_word)next(&seed);m->words[1]=(agc_word)next(&seed);m->words[2]=(agc_word)next(&seed);
    m->words[3]=03000u;m->words[5]=z;m->words[7]=0;
    if(n%34u<26u){m->words[4]=032000u;m->words[6]=032006u;}else{m->words[4]=040000u;m->words[6]=040006u;}
    for(i=0;i<sizeof classes/sizeof classes[0];++i)m->words[E6(classes[i])]=classed(&seed,next(&seed)>>7);
    for(i=0;i<3u;++i){static const unsigned zeroed[3]={01701u,01715u,01664u};   /* PREL, ROLLC, ROLL/180 */
        switch(next(&seed)&7u){case 0: m->words[E6(zeroed[i])]=0;break;case 1: m->words[E6(zeroed[i])]=AGC_MASK;break;default: break;}}
    switch(next(&seed)%5u){                          /* P63FLAG */
    case 0: m->words[E6(01727u)]=077776u;break;
    case 1: m->words[E6(01727u)]=0;break;
    case 2: m->words[E6(01727u)]=AGC_MASK;break;
    case 3: m->words[E6(01727u)]=1u;break;
    default: break;}
    m->words[E6(01711u)]=small(&seed,-1,1);          /* JETAG */
    {static const agc_word mode[4]={077776u,0u,077777u,1u};m->words[E6(01700u)]=mode[next(&seed)&3u];}   /* CMDAPMOD */
    m->words[0305]=(next(&seed)%28u==27u)?AGC_MASK:small(&seed,-13,13);   /* SW/NDX */
    io->channel5=(agc_word)(next(&seed)&AGC_MASK);
    io->channel6=(agc_word)(next(&seed)&AGC_MASK);
}
