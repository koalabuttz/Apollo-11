/* SPDX-License-Identifier: MIT */
#include "cm_tvc_massprop_cases.h"
/* Both entries; every DAPDATR1 CCS class with LEM on/off (BIT13) and
 * descent-stage (BIT14) combinations; physical and raw masses, so the
 * overflow detours and their non-returning loops are reached too. */
static uint32_t next(uint32_t *s){*s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;}
void cm_tvc_massprop_input(agc_image *m,agc_word *irq,uint32_t n){
    uint32_t seed=UINT32_C(0x5bd1e995)^(n*UINT32_C(0x9e3779b9));unsigned i,mode=(unsigned)(n>>1)&7u;agc_word d;
    if(!seed)seed=1;
    for(i=0;i<2048u;++i)m->words[i]=(agc_word)(next(&seed)&AGC_MASK);
    m->words[0]=(agc_word)next(&seed);m->words[1]=(agc_word)next(&seed);
    m->words[2]=(agc_word)(n&0x200u?04000u+(next(&seed)&03777u):02000u+(next(&seed)&01365u));
    m->words[3]=03000u;m->words[4]=012000u;m->words[5]=(n&1u)?03422u:03366u;m->words[6]=012006u;m->words[7]=0;
    d=(agc_word)(next(&seed)&AGC_MASK);
    if(mode==0u)d=0;else if(mode==1u)d=AGC_MASK;else{d=(agc_word)(d&~(agc_word)070000u);if(mode&4u)d|=040000u;if(n&0x10u)d|=010000u;if(n&0x20u)d|=020000u;}
    m->words[03066]=d;
    if((n>>6)&3u){m->words[03073]=(agc_word)(next(&seed)%4000u);m->words[03074]=(agc_word)(1500u+next(&seed)%7000u);}
    if(((n>>6)&3u)==1u){m->words[03073]=(agc_word)(next(&seed)&037777u);}
    *irq=(agc_word)((n>>8)&1u);
}
