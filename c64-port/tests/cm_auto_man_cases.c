/* SPDX-License-Identifier: MIT */
#include "cm_auto_man_cases.h"
/* Every entry (AHFNOROT and JETS weighted); channel 31 hold/free bits and
 * RCSFLAGS raw; HOLDFLAG zero, -0, small and raw; ADB small and raw (JETS'
 * TS T5TEMP overflow when ADB is near POSMAX); ADOT, WBODY, the attitude
 * errors, AERR, SLOPE and J/M zero, -0, small, medium and raw, so every
 * phase-plane region (J7, J8, J18-J24, JN/JP) and the TAU limits occur;
 * KMPAC zero, -0, small and raw at the SMALLMP returns; a sixteenth of
 * the cases put the rates on the WLH/WLMH boundaries; raw A/L/Q and unowned words. */
static const agc_word entry[8]={03250u,03250u,03250u,03441u,03441u,03304u,03732u,03734u};
static uint32_t next(uint32_t *s){*s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;}
static agc_word classed(uint32_t *s,unsigned k){agc_word v=(agc_word)(next(s)&AGC_MASK);switch(k&7u){case 0:return 0;case 1:return AGC_MASK;case 2:return (agc_word)(v&0777u);case 3:return agc_negate((agc_word)(v&0777u));case 4:return (agc_word)(v&07777u);case 5:return agc_negate((agc_word)(v&07777u));default:return v;}}
void cm_auto_man_input(agc_image *m,cm_auto_man_io *io,uint32_t n){
    uint32_t seed=UINT32_C(0x3c6ef372)^(n*UINT32_C(0x9e3779b9)),k=n>>3;unsigned i;
    if(!seed)seed=1;
    for(i=0;i<2048u;++i)m->words[i]=(agc_word)(next(&seed)&AGC_MASK);
    m->words[0]=(agc_word)next(&seed);m->words[1]=(agc_word)next(&seed);m->words[2]=(agc_word)next(&seed);
    m->words[3]=03000u;m->words[4]=042000u;m->words[5]=(n&15u)==15u?03757u:entry[n&7u];m->words[6]=042006u;m->words[7]=0;
    m->words[01332]=classed(&seed,k);                /* HOLDFLAG */
    m->words[03255]=(k>>3)&1u?(agc_word)(037770u+(next(&seed)&7u)):classed(&seed,k>>4);   /* ADB */
    for(i=0;i<3u;++i)m->words[03256u+i]=(agc_word)(next(&seed)&3u);   /* RMANNDX, PMANNDX, YMANNDX */
    m->words[03106]=(agc_word)(next(&seed)%3u);      /* SPNDX */
    for(i=0;i<6u;++i){m->words[03133u+i]=classed(&seed,(k>>2)+i);m->words[03125u+i]=classed(&seed,(k>>5)+i);}   /* ADOT, WBODY */
    for(i=0;i<3u;++i){m->words[03167u+i]=classed(&seed,(k>>6)+i);m->words[03164u+i]=classed(&seed,(k>>7)+i);m->words[03223u+i]=classed(&seed,(k>>8)+i);}   /* ERRORX-Z, BIAS, J/M */
    m->words[03254]=classed(&seed,k>>9);             /* SLOPE */
    m->words[03110]=classed(&seed,k>>10);            /* KMPAC (TAUNORM at the SMALLMP return) */
    if(((k>>10)&3u)==1u){                            /* rates on the WLH/WLMH boundaries (SIGNCK2/3) */
        m->words[01332]=0;
        for(i=0;i<3u;++i){agc_word h=(next(&seed)&1u)?022u:012u;m->words[03133u+2u*i]=(next(&seed)&1u)?agc_negate(h):h;}
    }
    io->channel31=(agc_word)(next(&seed)&AGC_MASK);
    io->channel32=(agc_word)(next(&seed)&AGC_MASK);
}
