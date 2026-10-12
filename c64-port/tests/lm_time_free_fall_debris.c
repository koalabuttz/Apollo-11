/* SPDX-License-Identifier: MIT */
#include "lm_time_free_fall_cases.h"
#define W m->words
uint32_t lm_time_free_fall_random(uint32_t *seed){*seed^=*seed<<13;*seed^=*seed>>17;*seed^=*seed<<5;return *seed;}
void lm_time_free_fall_debris(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 uint32_t seed=UINT32_C(0x54464631)+(n/8u)*UINT32_C(0x9e3779b9);
 unsigned i,vac;
 for(i=0;i<2048u;++i)W[i]=(agc_word)(lm_time_free_fall_random(&seed)&AGC_MASK);
 W[0]=(agc_register)(n*257u);W[1]=(agc_register)(n*977u);W[2]=(agc_word)(n&AGC_MASK);
 W[3]=(agc_word)(((n/16u)%8u)*0400u);
 vac=0401u+((n/32u)%5u)*054u;W[0120]=(agc_word)vac;
 /* SR30.1 SETPD 0. */
 W[0166]=(agc_word)vac;W[0163]=0;W[0156]=0;
 /* OVFIND as overflow leaves it: usually clear, sometimes +-1. */
 W[0121]=(n/1024u)%8u==3u?1u:(n/1024u)%8u==6u?077776u:0u;
 io->interrupt_enabled=(uint16_t)((n/64u)&1u);
 io->superbank=(uint16_t)(((n/512u)%8u)*020u);
}
