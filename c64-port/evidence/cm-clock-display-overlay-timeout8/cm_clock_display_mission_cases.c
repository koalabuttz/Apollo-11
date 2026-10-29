/* SPDX-License-Identifier: MIT */
#include "cm_clock_display_mission_cases.h"
#include "cm_clock_mission_cases.h"
void cm_clock_display_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){cm_clock_mission_input(s,io,(uint16_t)(128u+n));s->words[0100]=n>=32u?1u:0;s->words[01012]=n>=16u&&n<32u?1u:0;s->words[01042]=0;s->words[01043]=0;s->words[0374]=0;io->channel11=(agc_word)(n*371u);}
