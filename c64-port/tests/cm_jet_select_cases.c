/* SPDX-License-Identifier: MIT */
#include "cm_jet_select_cases.h"
/* JETSLECT in three cases of four, T6START in the fourth. XTRANS -4..+4;
 * TAU, TAU1, TAU2 zero, -0, small (the 14 ms and 0.1 s blast limits), and
 * raw; RACFAIL, RBDFAIL and ACORBD zero, -0, small either sign and raw (the
 * CCS four ways of every failure branch, each class drawn on its own); channel 31's translation bits
 * (07700) raw, with a quarter of the cases all released (no translation
 * command); DAPDATR1, RCSFLAGS, T5TIME (TIME5's overflow) and A/L/Q raw.
 * For T6START: TIME6 and BLAST, BLAST1, BLAST2 classed, the three +1
 * words each 0, 4 or 11, and the jet words and channels 5 and 6 raw.
 * Channel 13 starts with its trap bits and device-only bits clear (as for
 * cm-rcs-dap); the file only ORs in BIT15. */
static uint32_t next(uint32_t *s){*s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;}
static agc_word classed(uint32_t *s,unsigned k){agc_word v=(agc_word)(next(s)&AGC_MASK);switch(k&7u){case 0:return 0;case 1:return AGC_MASK;case 2:return (agc_word)(v&0777u);case 3:return agc_negate((agc_word)(v&0777u));case 4:return (agc_word)(v&07777u);case 5:return agc_negate((agc_word)(v&07777u));default:return v;}}
static agc_word small(uint32_t *s,int lo,int hi){int v=lo+(int)(next(s)%(uint32_t)(hi-lo+1));return v<0?agc_negate((agc_word)-v):(agc_word)v;}
void cm_jet_select_input(agc_image *m,cm_jet_select_io *io,uint32_t n){
    static const agc_word offsets[3]={0u,4u,013u};
    uint32_t seed=UINT32_C(0x3c6ef372)^(n*UINT32_C(0x9e3779b9)),k=n>>2;unsigned i;
    if(!seed)seed=1;
    for(i=0;i<2048u;++i)m->words[i]=(agc_word)(next(&seed)&AGC_MASK);
    m->words[0]=(agc_word)next(&seed);m->words[1]=(agc_word)next(&seed);m->words[2]=(agc_word)next(&seed);
    m->words[3]=03000u;m->words[4]=036000u;m->words[5]=(n&3u)==3u?03617u:02552u;m->words[6]=036006u;m->words[7]=0;
    m->words[03231]=small(&seed,-4,4);               /* XTRANS */
    for(i=0;i<3u;++i)m->words[03161u+i]=classed(&seed,k+3u*i);        /* TAU, TAU1, TAU2 */
    for(i=0;i<3u;++i)m->words[03226u+i]=classed(&seed,next(&seed)>>7);   /* RACFAIL, RBDFAIL, ACORBD */
    m->words[031]=classed(&seed,k>>1);               /* TIME6 */
    for(i=0;i<3u;++i){m->words[03057u+2u*i]=classed(&seed,next(&seed)>>7);  /* BLAST, BLAST1, BLAST2 */
        m->words[03060u+2u*i]=offsets[next(&seed)%3u];}                /* and their +1 words */
    io->channel31=(agc_word)(next(&seed)&AGC_MASK);
    if(((k>>9)&3u)==0u)io->channel31|=07700u;
    io->channel5=(agc_word)(next(&seed)&AGC_MASK);
    io->channel6=(agc_word)(next(&seed)&AGC_MASK);
    io->channel13=(agc_word)(next(&seed)&040765u);
}
