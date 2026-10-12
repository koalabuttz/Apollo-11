/* SPDX-License-Identifier: MIT */
#include "lm_conic_kepler_cases.h"
#define W m->words
uint32_t lm_conic_kepler_random(uint32_t *seed){*seed^=*seed<<13;*seed^=*seed>>17;*seed^=*seed<<5;return *seed;}
void lm_conic_kepler_debris(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 uint32_t seed=UINT32_C(0x4b45504c)+(n/8u)*UINT32_C(0x9e3779b9);
 unsigned i,vac;
 for(i=0;i<2048u;++i)W[i]=(agc_word)(lm_conic_kepler_random(&seed)&AGC_MASK);
 W[0]=(agc_register)(n*257u);W[1]=(agc_register)(n*977u);W[2]=(agc_word)(n&AGC_MASK);
 /* KEPPREP's GOTO KEPLERN: NEWOPS at 12,2023 (FBANK 12, BIT15 clear). */
 W[3]=(agc_word)(((n/16u)%8u)*0400u);W[4]=024000;W[6]=(agc_word)(024000u|(W[3]>>8));W[0165]=W[6];
 W[5]=06070;W[0164]=02023;W[0115]=0;W[023]=0;W[7]=0;W[067]=0;
 vac=0401u+((n/32u)%5u)*054u;W[0120]=(agc_word)vac;W[0166]=(agc_word)(vac+(n%6u)*2u);W[0163]=(agc_word)((n/4u)%2u?077776u:0u);
 /* AXC,1 2 (Earth) or 10 (Moon, MOONFLAG); STQ KEPRTN, the return of one of
  * KEPPREP's callers (13,3252, 11,2516, 11,3012). */
 W[vac+046u]=(n/8u)&1u?077765u:077775u;W[02112]=(n/7u)%3u==0u?027252u:(n/7u)%3u==1u?022516u:023012u;
 /* OVFIND as overflow leaves it: usually clear, sometimes +-1. */
 W[0121]=(n/1024u)%8u==3u?1u:(n/1024u)%8u==6u?077776u:0u;
 io->interrupt_enabled=(uint16_t)((n/64u)&1u);
 io->superbank=(uint16_t)(((n/512u)%8u)*020u);
}
