/* SPDX-License-Identifier: MIT */
#include "cm_tvc_daps_cases.h"
/* Every entry (cases 5 and 6 of each eight are PCOPY+1 and YCOPY+1, with a
 * caller's return in Q; case 7 is 2543 or 2745); STROKER zero, -0, small
 * and raw (the HACK calls);
 * CDU readings near their past values and raw (RLIMTEST's rate limit);
 * attitude errors, rates, sines/cosines, filter state, coefficients and
 * VARK small and raw, so ERRORLIM and ACTLIM both pass and limit, ACTLIM's
 * DOUBLE overflows and every DAPDATR1 cascade choice is taken; raw
 * T5TVCDT (DAPINIT's TIME5 overflow), A/L/Q and unowned words. Channel 14
 * starts with the bits yaAGC's gyro and CDU drive models act on (01000,
 * 070000) clear; the file only ORs in bits 11 and 12. */
static const agc_word entry[8]={02530u,02732u,03107u,02541u,02743u,02675u,03051u,02543u};
static uint32_t next(uint32_t *s){*s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;}
static agc_word classed(uint32_t *s,unsigned k){agc_word v=(agc_word)(next(s)&AGC_MASK);switch(k&7u){case 0:return 0;case 1:return AGC_MASK;case 2:return (agc_word)(v&0777u);case 3:return agc_negate((agc_word)(v&0777u));case 4:return (agc_word)(v&07777u);case 5:return agc_negate((agc_word)(v&07777u));default:return v;}}
void cm_tvc_daps_input(agc_image *m,cm_tvc_daps_io *io,uint32_t n){
    uint32_t seed=UINT32_C(0x5c1b2d73)^(n*UINT32_C(0x9e3779b9));unsigned i,e=n&7u,k;
    if(!seed)seed=1;
    for(i=0;i<2048u;++i)m->words[i]=(agc_word)(next(&seed)&AGC_MASK);
    m->words[0]=(agc_word)next(&seed);m->words[1]=(agc_word)next(&seed);
    m->words[2]=(agc_word)(next(&seed)&AGC_MASK);
    if(e==5u||e==6u)m->words[2]=(agc_word)(04000u+(next(&seed)&03777u));
    m->words[3]=03000u;m->words[4]=040000u;m->words[5]=e==7u&&(n>>15)&1u?02745u:entry[e];m->words[6]=040006u;m->words[7]=0;
    m->words[03174]=classed(&seed,n>>3);            /* STROKER */
    m->words[033]=(agc_word)(next(&seed)&AGC_MASK);  /* CDUY, CDUZ and their past values */
    m->words[034]=(agc_word)(next(&seed)&AGC_MASK);
    if((n>>6)&1u){m->words[03245]=(agc_word)((m->words[033]+(next(&seed)&0377u)-0177u)&AGC_MASK);m->words[03246]=(agc_word)((m->words[034]+(next(&seed)&0377u)-0177u)&AGC_MASK);}
    /* Attitude errors, commanded and body rates, sines and cosines. */
    k=n>>7;
    m->words[03175]=classed(&seed,k);m->words[03176]=classed(&seed,k>>1);
    m->words[03177]=classed(&seed,k);m->words[03200]=classed(&seed,k>>1);
    m->words[03127]=classed(&seed,k>>2);m->words[03130]=classed(&seed,k>>3);
    m->words[03131]=classed(&seed,k>>2);m->words[03132]=classed(&seed,k>>3);
    m->words[0742]=classed(&seed,k>>4);m->words[0746]=classed(&seed,k>>5);m->words[0750]=classed(&seed,k>>6);
    /* Filter coefficients, VARK, offsets, past commands and filter state. */
    k=n>>10;
    for(i=0;i<15u;++i)m->words[03323u+i]=classed(&seed,k+i);
    m->words[03241]=classed(&seed,k>>1);
    m->words[03205]=classed(&seed,k>>2);m->words[03206]=classed(&seed,k>>3);
    m->words[03207]=classed(&seed,k>>2);m->words[03210]=classed(&seed,k>>3);
    for(i=0;i<12u;++i){m->words[03141u+i]=classed(&seed,(k>>4)+i);m->words[03155u+i]=classed(&seed,(k>>4)+i);}
    io->channel14=(agc_word)(next(&seed)&AGC_MASK&~071000u);
}
