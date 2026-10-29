/* SPDX-License-Identifier: MIT */
#include "cm_super_dispatch_cases.h"
void cm_super_dispatch_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){unsigned i;agc_word loc=(agc_word)(0100u+(n/2u)%07700u),bank=(agc_word)((((n/2u)%32u)*02000u)|((n/64u)%8u)|(((n/512u)%8u)*020u));
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)((n*313u+i*217u+012345u)&AGC_MASK);
 for(i=0;i<13u;++i)((agc_word *)io)[i]=(agc_word)((n*17u+i*113u)&AGC_MASK);
 io->superbank=(agc_word)(((n/3u)%8u)*020u);io->interrupt_enabled=(agc_word)((n/2048u)%2u);
 s->words[0]=agc_extend(n&1u?bank:loc);s->words[1]=agc_extend(n&1u?loc:bank);s->words[2]=(agc_word)((n*713u)&0177777u);s->words[3]=(agc_word)(((n/11u)%8u)*0400u);s->words[4]=(agc_word)(((n/7u)%32u)*02000u);s->words[5]=n>=28672u?04622:n&1u?05230:05227;s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[7]=0;
}
