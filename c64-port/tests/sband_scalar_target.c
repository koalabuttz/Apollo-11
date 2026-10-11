/* SPDX-License-Identifier: MIT */
#include "sband_scalar_cases.h"
#include "sband_scalar_target.h"
#define SBAND_SCALAR_FROZEN_IMPLEMENTATION
#include "sband_scalar_vectors.h"
static apollo_overlay_state state,expected;
int sband_scalar_target_check(unsigned *completed){uint16_t row,i,nlo,nhi,irq,super,status,count,at,value,lo,hi;uint32_t n;unsigned actual;agc_word actual_irq,actual_super;uint8_t use_fixed;agc_word fixed[2];enum apollo_fixed_mission mission;enum sband_scalar_operation op;frozen_words reader;*completed=0;
 for(row=0;row<SBAND_SCALAR_FROZEN_ROWS;++row){
  if(!sband_scalar_reference_begin(&reader,row)||!frozen_words_next(&reader,&nlo)||!frozen_words_next(&reader,&nhi)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&count))return 1;
  n=(uint32_t)nlo|((uint32_t)nhi<<16);sband_scalar_input(&state,&actual_irq,&actual_super,fixed,&use_fixed,n,&mission,&op);
  for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&at)||at>=2048u||!frozen_words_next(&reader,&value))return 2;expected.words[at]=value;}
  for(i=count;i<SBAND_SCALAR_MAX_CHANGES;++i)if(!frozen_words_next(&reader,&lo)||!frozen_words_next(&reader,&hi)||lo||hi)return 3;
  if(!frozen_words_finished(&reader))return 4;
  actual=sband_scalar(&state,mission,op,use_fixed?fixed:0);if(actual!=status||irq!=actual_irq||super!=actual_super)return 5;
  for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 6;
  *completed=row+1u;
 }return 0;
}
