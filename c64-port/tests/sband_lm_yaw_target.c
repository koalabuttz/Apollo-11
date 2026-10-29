/* SPDX-License-Identifier: MIT */
#include "sband_lm_yaw_cases.h"
#include "sband_lm_yaw_target.h"
#define SBAND_LM_YAW_FROZEN_IMPLEMENTATION
#if defined(SBAND_LM_YAW_GROUP)
#if SBAND_LM_YAW_GROUP == 0
#include "sband_lm_yaw_vectors_0.h"
#elif SBAND_LM_YAW_GROUP == 1
#include "sband_lm_yaw_vectors_1.h"
#elif SBAND_LM_YAW_GROUP == 2
#include "sband_lm_yaw_vectors_2.h"
#else
#include "sband_lm_yaw_vectors_3.h"
#endif
#else
#include "sband_lm_yaw_vectors.h"
#endif
#ifdef __mos__
#define state (*(apollo_overlay_state *)0xd000u)
#define expected (*(apollo_overlay_state *)0xe000u)
#else
static apollo_overlay_state state,expected;
#endif
int sband_lm_yaw_target_check(unsigned *completed){uint16_t row,i,nlo,nhi,irq,super,status,count,at,value,lo,hi;uint32_t n;unsigned actual;enum apollo_fixed_mission mission;p66_bias_io io;frozen_words reader;*completed=0;
 for(row=0;row<SBAND_LM_YAW_FROZEN_ROWS;++row){
  if(!sband_lm_yaw_reference_begin(&reader,row)||!frozen_words_next(&reader,&nlo)||!frozen_words_next(&reader,&nhi)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&count))return 1;
  n=(uint32_t)nlo|((uint32_t)nhi<<16);sband_lm_yaw_input(&state,&io,n,&mission);
  for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&at)||at>=2048u||!frozen_words_next(&reader,&value))return 2;expected.words[at]=value;}
  for(i=count;i<SBAND_LM_YAW_MAX_CHANGES;++i)if(!frozen_words_next(&reader,&lo)||!frozen_words_next(&reader,&hi)||lo||hi)return 3;
  if(!frozen_words_finished(&reader))return 4;
  actual=sband_lm_yaw(&state,&io);if(actual!=status||irq!=io.interrupt_enabled||super!=io.superbank)return 5;
  for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 6;
  *completed=row+1u;
 }return 0;
}
