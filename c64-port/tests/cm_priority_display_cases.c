/* SPDX-License-Identifier: MIT */
#include "cm_priority_display_cases.h"
#include "cm_admission_complete_cases.h"
void cm_priority_display_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){unsigned slot;static const agc_word callers[8]={0100,01776,01777,02000,03777,07775,07776,07777};(void)cm_admission_complete_input(s,io,(n%3072u)*4u+2u);s->words[0167]=(agc_word)(((n/32u)%31u+1u)*01000u+((n&1u)?0400u:0u));s->words[067]=AGC_NEG_ZERO;
 s->words[5]=02726;s->words[4]=020000;s->words[3]=(agc_word)((n%8u)*0400u);s->words[6]=(agc_word)(020000u|(s->words[3]>>8));s->words[0]=agc_extend((agc_word)((n*71u)&AGC_MASK));s->words[1]=agc_extend(s->words[1]);s->words[2]=(agc_word)(0100u+n%07677u);s->words[0133]=callers[(n/256u)%8u];s->words[0134]=(agc_word)(((n/8u)%32u)*02000u);io->interrupt_enabled=(agc_word)((n/64u)&1u);io->superbank=(agc_word)(((n/128u)%8u)*020u);
 /* Include all unavailable source +0 slots among occupied cores. */
 slot=1u+(n/1024u)%6u;if((n&3u)==0u&&s->words[0154u+slot*12u+11u]!=AGC_NEG_ZERO)s->words[0154u+slot*12u+11u]=0;
}
