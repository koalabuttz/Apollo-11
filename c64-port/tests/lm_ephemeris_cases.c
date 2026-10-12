/* SPDX-License-Identifier: MIT */
#include "lm_ephemeris_cases.h"
void lm_ephemeris_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 /* Actual LSPOS/LUNPOS QPRET values: ORBITAL_INTEGRATION 11,2562/3272/3372,
  * P51-P53 14,2511 and S-band 42,3626 (bank slot 32 under SUPER 0100). */
 static const agc_word returns[5]={022563,023273,023373,030512,065627};
 static const agc_word edge[16]={0,077777,1,077776,037777,040000,037776,040001,020000,057777,017777,060000,2,077775,012345,065432};
 unsigned i,ret;uint32_t seed=UINT32_C(0x4c535053)+(n/16u)*UINT32_C(0x9e3779b9);
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;m->words[i]=(agc_word)(seed&AGC_MASK);}
 m->words[0]=(agc_register)(n*257u);m->words[1]=(agc_register)(n*977u);m->words[2]=(agc_word)(n&AGC_MASK);
 m->words[3]=(agc_word)(((n/4u)%8u)*0400u);m->words[4]=032000;m->words[6]=(agc_word)(032000u|(m->words[3]>>8));m->words[5]=06070;m->words[7]=0;m->words[067]=0;
 m->words[0120]=(agc_word)(0401u+((n/32u)%5u)*054u);m->words[0163]=0;m->words[0165]=m->words[6];m->words[0115]=0;m->words[023]=0;
 if(n%64u==63u){m->words[0164]=03775;m->words[m->words[0120]+052u]=023417;}
 else{ret=(n/2u)%5u;m->words[0164]=03664;m->words[m->words[0120]+052u]=returns[ret];}
 if(n<4096u){m->words[0154]=edge[(n/4u)%16u];m->words[0155]=edge[(n/64u)%16u];for(i=0;i<3u;++i)m->words[01706u+i]=edge[(n/16u+i*5u)%16u];}
 else{m->words[0154]&=037777u;m->words[01706]&=0177u;}
 io->interrupt_enabled=(uint16_t)((n/32u)&1u);
 io->superbank=(uint16_t)(m->words[m->words[0120]+052u]==065627u?0100u:((n/128u)%8u)*020u);
}
