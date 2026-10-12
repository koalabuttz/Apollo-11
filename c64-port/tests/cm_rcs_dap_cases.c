/* SPDX-License-Identifier: MIT */
#include "cm_rcs_dap_cases.h"
/* Every entry, RCSATT weighted; T5PHASE, HOLDFLAG, ATTSEC and the attitude
 * error inputs zero, -0, small and raw; ATTKALMN -1..11 and RATEINDX 0..6
 * (the table indices the entries admit); channel 31's DAP-enable bits and
 * hand-controller bits against CH31TEMP (equal, all released, raw);
 * RCSFLAGS, IMODES33, FLAGWRD0/9 and channel 12 raw, so every needles,
 * free-drift, rate-filter, Kalman and manual-rate branch is reached; AK
 * small and raw (the DAC limit and its TS overflow); raw T5TIME (KALUPDT's
 * TIME5 overflow), A/L and unowned words. A caller's return in Q for the
 * TC Q entries; raw Q elsewhere. A quarter of the RCSATT cases each are
 * steered into T5PHASE2 with the DAP enabled (hand controller, manual rate
 * and free-drift paths) and into the rate filter. Channel 13 starts with its trap bits and
 * device-only bits clear (as for cm-tvc-roll) and channel 14 with the gyro,
 * optics and IMU CDU drive bits clear (01000-070000): the file only ORs in
 * 13,14,15 and BIT15 of channel 13. */
static const agc_word entry[20]={02122u,02122u,02122u,02122u,02105u,02236u,02401u,02360u,02616u,02550u,
                                 02552u,02534u,03067u,02420u,02420u,02443u,02476u,02476u,02632u,03130u};
static uint32_t next(uint32_t *s){*s^=*s<<13;*s^=*s>>17;*s^=*s<<5;return *s;}
static agc_word classed(uint32_t *s,unsigned k){agc_word v=(agc_word)(next(s)&AGC_MASK);switch(k&7u){case 0:return 0;case 1:return AGC_MASK;case 2:return (agc_word)(v&0777u);case 3:return agc_negate((agc_word)(v&0777u));case 4:return (agc_word)(v&07777u);case 5:return agc_negate((agc_word)(v&07777u));default:return v;}}
static agc_word small(uint32_t *s,int lo,int hi){int v=lo+(int)(next(s)%(uint32_t)(hi-lo+1));return v<0?agc_negate((agc_word)-v):(agc_word)v;}
void cm_rcs_dap_input(agc_image *m,cm_rcs_dap_io *io,uint32_t n){
    uint32_t seed=UINT32_C(0x2b7e1516)^(n*UINT32_C(0x9e3779b9)),k=n/20u;unsigned i,e=n%20u,sp;agc_word z;
    if(!seed)seed=1;
    for(i=0;i<2048u;++i)m->words[i]=(agc_word)(next(&seed)&AGC_MASK);
    z=entry[e];
    m->words[0]=(agc_word)next(&seed);m->words[1]=(agc_word)next(&seed);m->words[2]=(agc_word)next(&seed);
    if(z==02420u||z==02443u||z==02476u||z==02632u||z==03130u)
        m->words[2]=(agc_word)((k&1u)?03250u+next(&seed)%0515u:04000u+(next(&seed)&03777u));
    m->words[3]=03000u;m->words[4]=042000u;m->words[5]=z;m->words[6]=042006u;m->words[7]=0;
    m->words[03065]=classed(&seed,k);                /* T5PHASE */
    m->words[03217]=small(&seed,-1,11);              /* ATTKALMN */
    m->words[01130]=small(&seed,0,6);                /* RATEINDX */
    m->words[01332]=classed(&seed,k>>3);             /* HOLDFLAG */
    m->words[03160]=classed(&seed,k>>5);             /* ATTSEC */
    sp=(unsigned)(next(&seed)%3u);m->words[03106]=(agc_word)sp;m->words[03107]=(agc_word)(2u*sp);   /* SPNDX, DPNDX */
    for(i=0;i<3u;++i)m->words[03076u+i]=classed(&seed,(k>>2)+i);   /* AK, AK1, AK2 */
    for(i=0;i<3u;++i)m->words[03103u+i]=classed(&seed,(k>>4)+i);   /* EDRIVEX-Z */
    io->channel31=(agc_word)(next(&seed)&AGC_MASK);
    switch((k>>6)&3u){                               /* CH31TEMP against channel 31 */
    case 0: m->words[03232]=(agc_word)((m->words[03232]&~077u)|(io->channel31&077u));break;
    case 1: m->words[03232]|=077u;break;
    default: break;}
    if((k>>8)&1u)io->channel31|=077u;
    if(e<4u)switch((k>>9)&3u){                       /* RCSATT: steer into the DAP phases */
    case 1: case 2:                                  /* T5PHASE2: hand controller, manual rates, free drift */
        m->words[03065]=0;m->words[03217]=small(&seed,-1,0);io->channel31&=(agc_word)~040000u;
        if(((k>>9)&3u)==1u)io->channel31|=020000u;else io->channel31&=(agc_word)~020000u;
        if((m->words[03232]&077u)==077u)m->words[03232]^=(agc_word)(1u<<(next(&seed)%6u));
        break;
    case 3:                                          /* -0: rate filter */
        m->words[03065]=AGC_MASK;io->channel31&=(agc_word)~040000u;m->words[01321]&=(agc_word)~040u;m->words[03101]&=(agc_word)~020000u;
        break;
    default: break;}
    io->channel12=(agc_word)(next(&seed)&AGC_MASK);
    io->channel13=(agc_word)(next(&seed)&040765u);
    io->channel14=(agc_word)(next(&seed)&AGC_MASK&~077000u);
}
