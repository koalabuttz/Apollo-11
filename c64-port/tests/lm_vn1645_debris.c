/* SPDX-License-Identifier: MIT */
#include "lm_vn1645_cases.h"
#define W m->words
uint32_t lm_vn1645_random(uint32_t *seed){*seed^=*seed<<13;*seed^=*seed>>17;*seed^=*seed<<5;return *seed;}
/* NEWOPS at VN1645 (35,3606) as its callers' GOTO leaves it: INTBIT15 set
 * for bank 35, any QPRET, superbank bits clear. FLAGWRD2 (FINALFLG, bit 6),
 * FLAGWRD3 (REFSMFLG, bit 13) and MODREG cover every path: FINALFLG clear
 * (GET45 at once), REFSMFLG clear, a P7X program, a P3X program with the
 * middle gimbal angle. BOFF branches to GET45 when the flag is clear. */
void lm_vn1645_debris(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 static const agc_word modes[3]={0,1,077776},p3x[4]={34,35,36,37},p7x[4]={74,75,76,77};
 uint32_t seed=UINT32_C(0x564e3136)+(n/8u)*UINT32_C(0x9e3779b9);
 unsigned i,vac,kind=(unsigned)(n%8u);
 for(i=0;i<2048u;++i)W[i]=(agc_word)(lm_vn1645_random(&seed)&AGC_MASK);
 W[0]=(agc_register)(n*257u);W[1]=(agc_register)(n*977u);W[2]=(agc_word)(n&AGC_MASK);
 W[3]=(agc_word)(((n/32u)%8u)*0400u);W[4]=072000u;W[6]=(agc_word)(072000u|(W[3]>>8));W[0165]=W[6];
 W[5]=06070;W[0164]=03606u;W[0115]=040000u;W[023]=0;W[7]=0;W[067]=0;
 W[0163]=modes[(n/16u)%3u];
 vac=0401u+((n/256u)%5u)*054u;W[0120]=(agc_word)vac;
 W[0166]=(agc_word)(vac+((n/64u)%8u)*2u);
 /* kind 0: FINALFLG clear; 1: REFSMFLG clear; 2: P7X; 3-7: P3X with MGA
  * (6: a random MODREG). */
 if(kind==0u)W[076]&=(agc_word)~040u;else W[076]|=040u;
 if(kind==1u)W[077]&=(agc_word)~010000u;else W[077]|=010000u;
 W[01011]=kind==2u?p7x[(n/8u)%4u]:kind==6u?W[01011]:p3x[(n/8u)%4u];
 W[0121]=(n/1024u)%8u==3u?1u:(n/1024u)%8u==6u?077776u:0u;
 io->interrupt_enabled=(uint16_t)((n/2048u)&1u);
 io->superbank=(uint16_t)(((n/4096u)%4u)*020u);
}
