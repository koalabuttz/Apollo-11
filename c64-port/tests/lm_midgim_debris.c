/* SPDX-License-Identifier: MIT */
#include "lm_midgim_cases.h"
#define W m->words
uint32_t lm_midgim_random(uint32_t *seed){*seed^=*seed<<13;*seed^=*seed>>17;*seed^=*seed<<5;return *seed;}
/* Each routine's call returns: GET+MGA (35,3632), GET.LVC (35,2510,
 * 27,2332, 27,2401) and SELECTMU's callers (35,2127, 35,2231, 35,2572,
 * 35,2737; its P39/P79 switch branches leave QPRET as it was). */
static const agc_word q_getmga[1]={073634},q_getlvc[3]={072512,056334,056403},q_selectmu[4]={072131,072233,072574,072741};
void lm_midgim_debris(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 static const agc_word modes[3]={0,1,077776};
 uint32_t seed=UINT32_C(0x4d494447)+(n/8u)*UINT32_C(0x9e3779b9);
 unsigned i,vac,kind=(unsigned)(n%8u),k=(unsigned)(n/8u);agc_word fb;
 for(i=0;i<2048u;++i)W[i]=(agc_word)(lm_midgim_random(&seed)&AGC_MASK);
 W[0]=(agc_register)(n*257u);W[1]=(agc_register)(n*977u);W[2]=(agc_word)(n&AGC_MASK);
 fb=kind<6u?014000u:020000u;
 W[3]=(agc_word)(((n/32u)%8u)*0400u);W[4]=fb;W[6]=(agc_word)(fb|(W[3]>>8));W[0165]=W[6];
 W[5]=06070;W[0164]=kind<3u?03717u:kind<6u?03733u:02000u;W[0115]=0;W[023]=0;W[7]=0;W[067]=0;
 W[0163]=modes[(n/16u)%3u];
 vac=0401u+((n/256u)%5u)*054u;W[0120]=(agc_word)vac;
 /* PUSHLOC: above GET+MGA's pushed velocity; anywhere in the VAC else. */
 W[0166]=(agc_word)(vac+(kind<3u?6u:0u)+((n/64u)%8u)*2u);
 W[vac+052u]=kind<3u?q_getmga[0]:kind<6u?q_getlvc[k%3u]:(n/2048u)%4u==3u?W[vac+052u]:q_selectmu[k%4u];
 /* OVFIND as overflow leaves it: usually clear, sometimes +-1. */
 W[0121]=(n/1024u)%8u==3u?1u:(n/1024u)%8u==6u?077776u:0u;
 io->interrupt_enabled=(uint16_t)((n/2048u)&1u);
 io->superbank=(uint16_t)(((n/4096u)%8u)*020u);
}
