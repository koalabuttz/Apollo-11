/* SPDX-License-Identifier: MIT */
#include "cm_interbank_extended_cases.h"
#include "cm_fixed_compat_cases.h"
agc_word cm_interbank_extended_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){uint16_t local=(uint16_t)n;unsigned kind=(unsigned)(n/65536u);agc_word literal=(agc_word)(((uint32_t)local*977u)&AGC_MASK);static const agc_word pc[14]={04662,04666,04675,04701,04704,04711,04715,04740,04744,04753,04757,04773,04707,04710};cm_fixed_compat_input(s,io,n%40960u);
 s->words[0]=local;s->words[1]=(agc_register)((uint32_t)local*313u);s->words[2]=0705;s->words[3]=(agc_word)((local&7u)*0400u);s->words[4]=(agc_word)(((local>>3)&31u)*02000u);s->words[6]=(agc_word)(s->words[4]|(local&7u));s->words[5]=pc[kind];s->words[0705]=literal;
 s->words[0133]=s->words[072]=0710;s->words[0134]=s->words[073]=(agc_word)(((uint32_t)local*997u)&AGC_MASK);io->superbank=(agc_word)(((local>>8)&7u)*020u);return literal;
}
