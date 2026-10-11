/* SPDX-License-Identifier: MIT */
#include "cm_makeplay_sleep_cases.h"
#include "cm_display_control_cases.h"
void cm_makeplay_sleep_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){uint16_t k=(uint16_t)(n%2048u);static const agc_word pc[3]={03176,03212,03037};cm_display_control_input(s,io,k);s->words[5]=pc[n/2048u];s->words[0164]=2;s->words[0154]=1;s->words[064]=(agc_word)(k%8u?12u*(k%7u):077776u);}
enum cm_display_sleep_boundary cm_makeplay_sleep_apply(apollo_overlay_state *s,cm_native_io *io,uint16_t n){return n<2048u?cm_makeplay_sleep_enter(s,io):n<4096u?cm_makeplay_sleep_return(s,io):cm_makeplay_sleep_resume(s,io);}
