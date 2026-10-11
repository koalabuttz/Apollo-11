/* SPDX-License-Identifier: MIT */
#include "cm_clock_display_busy_cases.h"
#include "cm_clock_display_mission_cases.h"
void cm_clock_display_busy_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){cm_clock_display_input(s,io,n);s->words[0100]=0;s->words[01012]=(agc_word)(1u+n%7u);s->words[0374]=051366;}
