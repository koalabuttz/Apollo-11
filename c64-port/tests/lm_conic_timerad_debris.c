/* SPDX-License-Identifier: MIT */
#include "lm_conic_timerad_cases.h"
#define W m->words
uint32_t lm_conic_timerad_random(uint32_t *seed){*seed^=*seed<<13;*seed^=*seed>>17;*seed^=*seed<<5;return *seed;}
void lm_conic_timerad_debris(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 uint32_t seed=UINT32_C(0x54524144)+(n/8u)*UINT32_C(0x9e3779b9);
 unsigned i,vac,apsides=(unsigned)(n&1u);
 for(i=0;i<2048u;++i)W[i]=(agc_word)(lm_conic_timerad_random(&seed)&AGC_MASK);
 W[0]=(agc_register)(n*257u);W[1]=(agc_register)(n*977u);W[2]=(agc_word)(n&AGC_MASK);
 /* The caller's STCALL: NEWOPS at 12,3547 or 12,3674 (FBANK 12, BIT15 clear). */
 W[3]=(agc_word)(((n/16u)%8u)*0400u);W[4]=024000;W[6]=(agc_word)(024000u|(W[3]>>8));W[0165]=W[6];
 W[5]=06070;W[0164]=apsides?03674u:03547u;W[0115]=0;W[023]=0;W[7]=0;W[067]=0;
 vac=0401u+((n/32u)%5u)*054u;W[0120]=(agc_word)vac;W[0166]=(agc_word)(vac+(n%6u)*2u);W[0163]=(agc_word)((n/4u)%2u?077776u:0u);
 /* X1 from RTX1 (Earth -2, Moon -10); QPRET the caller's return
  * (35,2106 after STCALL RDESIRED TIMERAD; 23,2333 after STCALL XXXALT
  * APSIDES). */
 W[vac+046u]=(n/8u)&1u?077765u:077775u;W[vac+052u]=apsides?046333u:072106u;
 /* RVSW (FLAGWRD7 BIT9): CSI sets it before TIMERAD; APSIDES leaves it. */
 if(!apsides||(n/2u)&1u)W[0103]|=0400u;else W[0103]&=(agc_word)~0400u;
 /* OVFIND as overflow leaves it: usually clear, sometimes +-1. */
 W[0121]=(n/1024u)%8u==3u?1u:(n/1024u)%8u==6u?077776u:0u;
 io->interrupt_enabled=(uint16_t)((n/64u)&1u);
 io->superbank=(uint16_t)(((n/512u)%8u)*020u);
}
