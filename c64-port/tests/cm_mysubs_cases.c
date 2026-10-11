/* SPDX-License-Identifier: MIT */
#include "cm_mysubs_cases.h"
void cm_mysubs_input(apollo_overlay_state *m,agc_word *irq,uint32_t n,uint8_t operation){static const agc_word edge[16]={0,077777,1,077776,037777,040000,037776,040001,020000,057777,017777,060000,2,077775,012345,065432};uint32_t seed=UINT32_C(0x20422054)+(n>>8)*UINT32_C(0x9e3779b9)+operation;unsigned i;for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;m->words[i]=(agc_word)(seed&AGC_MASK);}
 m->words[0]=(agc_word)n;m->words[1]=(agc_word)(n*37u+operation*8191u);m->words[2]=(agc_word)(02000u+n%3072u);m->words[3]=03000;m->words[4]=042000;m->words[5]=operation?02054:02042;m->words[6]=042006;m->words[7]=0;m->words[020]=0;m->words[023]=0;m->words[067]=0;m->words[0163]=(agc_word)(n&AGC_MASK);m->words[03110]=(agc_word)((n*127u+331u)&AGC_MASK);m->words[03111]=(agc_word)((n*257u+237u)&AGC_MASK);
 if(n>=65536u){uint32_t x=n-65536u;m->words[0]=agc_extend(edge[(x>>0)&15u]);m->words[1]=agc_extend(edge[(x>>4)&15u]);m->words[03110]=edge[(x>>8)&15u];m->words[03111]=edge[(x>>1)&15u];if(x&4096u){m->words[0]=(agc_word)(m->words[0]^0100000u);m->words[1]=(agc_word)(m->words[1]^040000u);}}
 *irq=(agc_word)(n&1u);
}
