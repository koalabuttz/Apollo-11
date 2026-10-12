/* SPDX-License-Identifier: MIT */
#include "lm_initvel_cases.h"
#define W m->words
uint32_t lm_initvel_random(uint32_t *seed){*seed^=*seed<<13;*seed^=*seed>>17;*seed^=*seed<<5;return *seed;}
/* The Luminary call returns: CALL INITVEL (35,2502, 35,3366, 27,2372) and
 * CALL HAVEGUES (27,2724). */
static const agc_word q_all[4]={072503,073367,056373,056725};
void lm_initvel_debris(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 uint32_t seed=UINT32_C(0x494e4954)+(n/8u)*UINT32_C(0x9e3779b9);
 unsigned i,vac,kind=(unsigned)(n%8u),moon=(unsigned)((n/8u)&1u),k=(unsigned)(n/16u);
 for(i=0;i<2048u;++i)W[i]=(agc_word)(lm_initvel_random(&seed)&AGC_MASK);
 W[0]=(agc_register)(n*257u);W[1]=(agc_register)(n*977u);W[2]=(agc_word)(n&AGC_MASK);
 /* The caller's CALL (or the integration's return): NEWOPS in bank 11. */
 W[3]=(agc_word)(((n/32u)%8u)*0400u);W[4]=022000;W[6]=(agc_word)(022000u|(W[3]>>8));W[0165]=W[6];
 W[5]=06070;W[0115]=0;W[023]=0;W[7]=0;W[067]=0;
 W[0164]=kind<4u?02000u:kind==4u?02002u:kind==5u?02155u:02200u;
 vac=0401u+((n/256u)%5u)*054u;W[0120]=(agc_word)vac;
 /* PUSHLOC 4D at the call; anywhere even in the VAC where the integration
  * returns. */
 W[0166]=(agc_word)(kind<5u?vac+4u:vac+((n/64u)%8u)*2u);
 /* QPRET the caller's return; NORMEX one where the integration returns. */
 if(kind<4u)W[vac+052u]=q_all[k%3u];
 else if(kind==4u)W[vac+052u]=q_all[3];
 else W[03463]=q_all[k%4u];
 /* RTX1/RTX2 (E5,1776-1777) as the callers' SETs leave them, Earth (-2, 0)
  * or Moon (-10, 2). */
 W[02776]=moon?077765u:077775u;W[02777]=moon?2u:0u;
 /* OVFIND as overflow leaves it: usually clear, sometimes +-1. */
 W[0121]=(n/1024u)%8u==3u?1u:(n/1024u)%8u==6u?077776u:0u;
 io->interrupt_enabled=(uint16_t)((n/2048u)&1u);
 io->superbank=(uint16_t)(((n/4096u)%8u)*020u);
}
