/* SPDX-License-Identifier: MIT */
#include "cm_phase_cleanup_cases.h"
#include "cm_vac5_store_cases.h"
void cm_phase_cleanup_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){static const agc_word pc[3]={02501,02505,02513};cm_vac5_store_input(s,io,n/3u);s->words[4]=012000;s->words[6]=(agc_word)(012000u|(s->words[3]>>8));s->words[5]=pc[n%3u];s->words[2]=0705;}
