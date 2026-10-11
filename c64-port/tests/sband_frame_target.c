/* SPDX-License-Identifier: MIT */
#include "sband_frame_cases.h"
#include "sband_frame_target.h"
#define SBAND_FRAME_FROZEN_IMPLEMENTATION
#if SBAND_FRAME_GROUP == 0
#include "sband_frame_vectors_0.h"
#elif SBAND_FRAME_GROUP == 1
#include "sband_frame_vectors_1.h"
#elif SBAND_FRAME_GROUP == 2
#include "sband_frame_vectors_2.h"
#elif SBAND_FRAME_GROUP == 3
#include "sband_frame_vectors_3.h"
#elif SBAND_FRAME_GROUP == 4
#include "sband_frame_vectors_4.h"
#else
#include "sband_frame_vectors_5.h"
#endif
static apollo_overlay_state state,expected;
int sband_frame_target_check(unsigned *completed){uint16_t row,i,nlo,nhi,irq,super,status,count,at,value,lo,hi;uint32_t n;unsigned actual;enum apollo_fixed_mission mission;p66_bias_io io;enum sband_frame_stage stage;frozen_words reader;*completed=0;
 for(row=0;row<SBAND_FRAME_FROZEN_ROWS;++row){
  if(!sband_frame_reference_begin(&reader,row)||!frozen_words_next(&reader,&nlo)||!frozen_words_next(&reader,&nhi)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&count))return 1;
  n=(uint32_t)nlo|((uint32_t)nhi<<16);sband_frame_input(&state,&io,n,&mission,&stage);
  for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&at)||at>=2048u||!frozen_words_next(&reader,&value))return 2;expected.words[at]=value;}
  for(i=count;i<SBAND_FRAME_MAX_CHANGES;++i)if(!frozen_words_next(&reader,&lo)||!frozen_words_next(&reader,&hi)||lo||hi)return 3;
  if(!frozen_words_finished(&reader))return 4;
  actual=sband_frame(&state,&io,mission,stage);if(actual!=status||irq!=io.interrupt_enabled||super!=io.superbank)return 5;
  for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 6;
  *completed=row+1u;
 }return 0;
}
