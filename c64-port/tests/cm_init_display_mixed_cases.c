/* SPDX-License-Identifier: MIT */
#include "cm_init_display_mixed_cases.h"
#include "cm_priority_complete_cases.h"
void cm_init_display_mixed_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){unsigned c=n/14u;agc_word priority;
 cm_priority_complete_input(s,io,n);priority=s->words[0];s->words[4]=020000;s->words[6]=(agc_word)(020000u|(s->words[3]>>8));s->words[5]=03261;s->words[0366]=(agc_word)(priority|(((c*5u)%8u)*020u));s->words[01071]=(agc_word)((s->words[01071]&074377u)|(((c*3u)%8u)*0400u));s->words[0374]=(agc_word)(((c*7u)%32u)*02000u+01403u+c%0370u);
}
