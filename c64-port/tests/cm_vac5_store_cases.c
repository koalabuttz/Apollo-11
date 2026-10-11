/* SPDX-License-Identifier: MIT */
#include "cm_vac5_store_cases.h"
#include "cm_admission_complete_cases.h"
void cm_vac5_store_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){unsigned i;uint32_t seed=0x5554321u+n;cm_admission_complete_input(s,io,n%CM_ADMISSION_COMPLETE_CASES);for(i=0100;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}s->words[4]=040000;s->words[6]=(agc_word)(040000u|(s->words[3]>>8));s->words[5]=02000;s->words[2]=04675;s->words[0]=(agc_word)(n*79u);s->words[1]=(agc_word)(n*571u);}
