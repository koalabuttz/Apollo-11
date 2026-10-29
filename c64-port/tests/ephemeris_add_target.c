/* SPDX-License-Identifier: MIT */
#include "ephemeris_add_cases.h"
#include "ephemeris_add_target.h"
#define EPHEMERIS_ADD_FROZEN_IMPLEMENTATION
#include "ephemeris_add_vectors.h"
static apollo_overlay_state state,expected;
int ephemeris_add_target_check(unsigned *completed){uint16_t row,i,nlo,nhi,irq,super,status,count,at,value,lo,hi;agc_word input_irq,input_super;unsigned actual;enum apollo_fixed_mission mission;frozen_words reader;*completed=0;
 for(row=0;row<EPHEMERIS_ADD_FROZEN_ROWS;++row){
  if(!ephemeris_add_reference_begin(&reader,row)||!frozen_words_next(&reader,&nlo)||!frozen_words_next(&reader,&nhi)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&count))return 1;
  mission=nhi?APOLLO_FIXED_CM:APOLLO_FIXED_LM;ephemeris_add_input(&state,&input_irq,&input_super,nlo,mission);
  for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&at)||at>=2048u||!frozen_words_next(&reader,&value))return 2;expected.words[at]=value;}
  for(i=count;i<EPHEMERIS_ADD_MAX_CHANGES;++i)if(!frozen_words_next(&reader,&lo)||!frozen_words_next(&reader,&hi)||lo||hi)return 3;
  if(!frozen_words_finished(&reader))return 4;
  actual=ephemeris_triple_add(&state,mission);
  if(actual!=status||irq!=input_irq||super!=input_super)return 5;
  for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 6;
  *completed=(unsigned)row+1u;
 }
 {static const uint16_t invalid[13][2]={{5,0},{7,1},{3,1},{3,04000},{4,1},{4,0100000},{6,1},{0116,0},{0116,027},{0116,01776},{0116,02000},{030,0100000},{01400,0100000}};
  for(row=0;row<13u;++row){
   ephemeris_add_input(&state,&input_irq,&input_super,0,APOLLO_FIXED_LM);state.words[invalid[row][0]]=invalid[row][1];
   for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
   if(ephemeris_triple_add(&state,APOLLO_FIXED_LM))return 7;
   for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 8;
  }
  if(ephemeris_triple_add(0,APOLLO_FIXED_LM))return 9;
 }
 return 0;
}
