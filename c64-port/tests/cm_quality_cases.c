/* SPDX-License-Identifier: MIT */
#include "cm_quality_cases.h"
void cm_quality_input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n)
{
 static const agc_word edge[12]={0,077777,1,077776,020000,057777,017777,060000,037777,040000,012345,065432};
 uint32_t seed=UINT32_C(0x434d5155)+(n/16u)*UINT32_C(0x9e3779b9);unsigned i,base;
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;m->words[i]=(agc_word)(seed&AGC_MASK);}
 m->words[0]=(agc_register)(n*977u);m->words[1]=(agc_register)(n*257u);m->words[2]=(agc_word)(n&AGC_MASK);
 m->words[3]=(agc_word)((n%8u)*0400u);m->words[4]=026000;m->words[6]=(agc_word)(026000u|(m->words[3]>>8));m->words[5]=06037;m->words[7]=0;
 m->words[067]=0;m->words[0115]=0;m->words[0120]=(agc_word)(0401u+(n%5u)*054u);base=m->words[0120];
 m->words[0164]=03770;m->words[0165]=m->words[6];m->words[0166]=(agc_word)(base+2u);m->words[0163]=077776;m->words[023]=0;
 m->words[base+052u]=023146;m->words[074]=(agc_word)((m->words[074]&073777u)|((n&1u)?04000u:0u));
 for(i=0;i<6u;++i)m->words[base+016u+i]=(agc_word)(n<256u?edge[(n+i*3u)%12u]:(n*977u+i*1237u)&AGC_MASK);
 m->words[01776]=(agc_word)((n*47u)&AGC_MASK);
 m->words[035]=m->words[036]=m->words[047]=0;for(i=050;i<=054;++i)m->words[i]=0;
 io->interrupt_enabled=(agc_word)((n>>1)&1u);io->superbank=(agc_word)(((n/8u)%8u)*020u);
}
