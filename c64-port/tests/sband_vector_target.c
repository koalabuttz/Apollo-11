/* SPDX-License-Identifier: MIT */
#include "sband_vector_cases.h"
#include "sband_vector_target.h"
#define SBAND_VECTOR_FROZEN_IMPLEMENTATION
#include "sband_vector_vectors.h"
static apollo_overlay_state state,expected;
int sband_vector_target_check(unsigned *completed){uint16_t row,i,nlo,nhi,irq,super,status,count,at,value,lo,hi;agc_word input_irq,input_super,factor[6];uint32_t local;unsigned actual;enum apollo_fixed_mission mission;frozen_words reader;*completed=0;
 for(row=0;row<SBAND_VECTOR_FROZEN_ROWS;++row){
  if(!sband_vector_reference_begin(&reader,row)||!frozen_words_next(&reader,&nlo)||!frozen_words_next(&reader,&nhi)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&count))return 1;
  mission=(nhi&2u)?APOLLO_FIXED_CM:APOLLO_FIXED_LM;local=(uint32_t)nlo|((uint32_t)(nhi&1u)<<16);sband_vector_input(&state,&input_irq,&input_super,local,mission);sband_vector_fixed_input(factor,local);
  for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&at)||at>=2048u||!frozen_words_next(&reader,&value))return 2;expected.words[at]=value;}
  for(i=count;i<SBAND_VECTOR_MAX_CHANGES;++i)if(!frozen_words_next(&reader,&lo)||!frozen_words_next(&reader,&hi)||lo||hi)return 3;
  if(!frozen_words_finished(&reader))return 4;
  actual=(nhi&1u)?((nlo&32768u)?sband_vector_projection_fixed(&state,mission,factor):sband_vector_cross_fixed(&state,mission,factor)):((nlo&32768u)?sband_vector_projection(&state,mission):sband_vector_cross(&state,mission));
  if(actual!=status||irq!=input_irq||super!=input_super)return 5;
  for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 6;
  *completed=row+1u;
 }return 0;
}
