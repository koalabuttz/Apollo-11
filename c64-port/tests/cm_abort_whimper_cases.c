/* SPDX-License-Identifier: MIT */
#include "cm_abort_whimper_cases.h"
#include "cm_vac5_store_cases.h"
void cm_abort_whimper_input(apollo_overlay_state *s,cm_native_io *io,uint8_t *in_isr,uint32_t n){cm_vac5_store_input(s,io,n);s->words[4]=(agc_word)((n/64u)%32u*02000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[5]=n&1u?07767:07766;s->words[015]=(agc_word)(n*37u&07777u);*in_isr=(uint8_t)((n>>1)&1u);}
