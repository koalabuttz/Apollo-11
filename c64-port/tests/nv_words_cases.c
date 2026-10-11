/* SPDX-License-Identifier: MIT */
#include "nv_words_cases.h"
#include "nv_entry_cases.h"
void nv_words_input(apollo_overlay_state *s,p66_bias_io *io,uint16_t n){uint8_t i;nv_entry_input(s,io,(uint16_t)(n*2u));s->words[01012]=0;s->words[01021]&=057777u;for(i=0;i<11u;++i)s->words[01023u+i]|=040000u;(void)p66_nv_enter(s,io);}
