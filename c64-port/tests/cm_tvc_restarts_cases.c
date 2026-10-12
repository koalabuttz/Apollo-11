/* SPDX-License-Identifier: MIT */
#include "cm_tvc_restarts_cases.h"
/* Every entry; each TVCPHASE restart value (-2..3, -0) and raw ones the
 * admission accepts; TVCEXPHS 1..6 for EXRSTRT, every CCS class elsewhere;
 * STROKER classes; raw commands, trims, A/L/Q, channels and unowned words.
 * Channel 14 starts with its gyro and IMU CDU drive bits (01000, 070000)
 * clear: yaAGC's device model acts on those by itself, and this file only
 * ORs in bits 11 and 12. */
static const agc_word entry[6]={03140u,03174u,03205u,03232u,03243u,03253u};
static const agc_word phases[8]={077775u,077776u,077777u,0u,1u,2u,3u,5u};
static uint32_t next(uint32_t *s){*s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;}
static agc_word classed(uint32_t *s,unsigned k){agc_word v=(agc_word)(next(s)&AGC_MASK);switch(k&7u){case 0:return 0;case 1:return AGC_MASK;case 2:return (agc_word)(1u+next(s)%6u);case 3:return agc_negate((agc_word)(1u+next(s)%20u));case 4:return 1u;default:return v;}}
void cm_tvc_restarts_input(agc_image *m,cm_tvc_restarts_io *io,uint32_t n){
    uint32_t seed=UINT32_C(0x6c8e9cf5)^(n*UINT32_C(0x9e3779b9));unsigned i,e=n%6u;agc_word phase;
    if(!seed)seed=1;
    for(i=0;i<2048u;++i)m->words[i]=(agc_word)(next(&seed)&AGC_MASK);
    m->words[0]=(agc_word)next(&seed);m->words[1]=(agc_word)next(&seed);
    m->words[2]=(agc_word)(04000u+(next(&seed)&03777u));
    m->words[3]=03000u;m->words[4]=034000u;m->words[5]=entry[e];m->words[6]=034006u;m->words[7]=0;
    phase=phases[(n>>3)&7u];
    if(((n>>6)&3u)==3u){phase=(agc_word)(next(&seed)&AGC_MASK);if((phase&040001u)==1u)phase=(agc_word)(1u+2u*(next(&seed)%5u));}
    m->words[03244]=phase;                                    /* TVCPHASE */
    m->words[03251]=e==5u?(agc_word)(1u+next(&seed)%6u):classed(&seed,n>>8);  /* TVCEXPHS */
    m->words[03174]=classed(&seed,n>>11);                     /* STROKER */
    if((n>>14)&1u)m->words[03223]=classed(&seed,n>>15);       /* PCMD */
    io->channel12=(agc_word)(next(&seed)&AGC_MASK);
    io->channel14=(agc_word)(next(&seed)&AGC_MASK&~071000u);
}
