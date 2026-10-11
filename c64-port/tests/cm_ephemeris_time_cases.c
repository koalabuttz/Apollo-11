/* SPDX-License-Identifier: MIT */
#include "cm_ephemeris_time_cases.h"
void cm_ephemeris_time_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 static const agc_word entries[4]={02110,02115,02120,02123};
 static const agc_word edge[16]={0,077777,1,077776,037777,040000,037776,040001,020000,057777,017777,060000,2,077775,012345,065432};
 unsigned i;uint32_t seed=UINT32_C(0x45504854)+(n/16u)*UINT32_C(0x9e3779b9);
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;m->words[i]=(agc_word)(seed&AGC_MASK);}
 m->words[0]=(agc_register)(n*257u);m->words[1]=(agc_register)(n*977u);m->words[2]=(agc_word)(n&AGC_MASK);
 m->words[3]=(agc_word)(((n/4u)%8u)*0400u);m->words[4]=054000;m->words[6]=(agc_word)(054000u|(m->words[3]>>8));m->words[5]=06037;m->words[7]=0;m->words[067]=0;
 m->words[0120]=(agc_word)(0401u+((n/32u)%5u)*054u);m->words[0163]=0;m->words[0164]=entries[n%4u];m->words[0165]=m->words[6];m->words[0115]=040000;m->words[023]=0;
 m->words[0154]=n<1024u?edge[(n/4u)%16u]:m->words[0154];m->words[0155]=n<1024u?edge[(n/64u)%16u]:m->words[0155];m->words[0156]=0;
 for(i=0;i<3u;++i)if(n<1024u){m->words[01706u+i]=edge[(n/16u+i*3u)%16u];m->words[02033u+i]=edge[(n/64u+i*7u)%16u];}
 m->words[m->words[0120]+052u]=(agc_word)(064000u+((n/128u)%02000u));
 io->interrupt_enabled=(agc_word)((n/32u)&1u);io->superbank=(agc_word)(0100u+((n/64u)%4u)*020u);
}
