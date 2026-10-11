/* SPDX-License-Identifier: MIT */
#include "sband_shift_cases.h"
#include "sband_shift_target.h"
#define SBAND_SHIFT_FROZEN_IMPLEMENTATION
#include "sband_shift_vectors.h"
static apollo_overlay_state state,expected;
int sband_shift_target_check(unsigned *completed){uint16_t row,i,nlo,nhi,irq,super,status,count,at,value,lo,hi;uint32_t n;unsigned actual;agc_word actual_irq,actual_super;uint8_t shift_count;enum apollo_fixed_mission mission;frozen_words reader;*completed=0;
 for(row=0;row<SBAND_SHIFT_FROZEN_ROWS;++row){
  if(!sband_shift_reference_begin(&reader,row)||!frozen_words_next(&reader,&nlo)||!frozen_words_next(&reader,&nhi)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&count))return 1;
  n=(uint32_t)nlo|((uint32_t)nhi<<16);sband_shift_input(&state,&actual_irq,&actual_super,n,&mission,&shift_count);
  for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&at)||at>=2048u||!frozen_words_next(&reader,&value))return 2;expected.words[at]=value;}
  for(i=count;i<SBAND_SHIFT_MAX_CHANGES;++i)if(!frozen_words_next(&reader,&lo)||!frozen_words_next(&reader,&hi)||lo||hi)return 3;
  if(!frozen_words_finished(&reader))return 4;
  actual=sband_shift_left(&state,mission,shift_count);if(actual!=status||irq!=actual_irq||super!=actual_super)return 5;
  for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 6;
  *completed=row+1u;
 }return 0;
}
