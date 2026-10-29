/* SPDX-License-Identifier: MIT */
#include "service_routines_cases.h"
#include "service_routines_target.h"
#include "service_routines_source.h"
#define SERVICE_ROUTINES_FROZEN_IMPLEMENTATION
#include "service_routines_vectors.h"
static apollo_overlay_state state,expected;
int service_routines_target_check(unsigned *completed){uint16_t row,i,index,value,nlo,nhi,kind,fixed[3],input[3],status,super,irq,count,lo,hi,iterations;uint32_t n;p66_bias_io io;frozen_words reader;enum service_routines_mission mission;enum service_routines_entry entry;unsigned actual;
 *completed=0;
 for(i=0;i<2u;++i){const service_routines_bindings *b=&service_routines_source[i];value=0;if(!service_routines_literal((enum service_routines_mission)i,(uint16_t)(b->tcsleep-02000u),&value)||value!=b->tcsleep_word||!service_routines_literal((enum service_routines_mission)i,(uint16_t)(b->wakecad-02000u),&value)||value!=b->wakecad_word||!service_routines_literal((enum service_routines_mission)i,(uint16_t)(b->delayjob+(i?013u:014u)-02000u),&value)||value!=01104||service_routines_literal((enum service_routines_mission)i,0,&value)||value!=01104)return 9;}
 for(row=0;row<SERVICE_ROUTINES_FROZEN_ROWS;++row){
  if(!service_routines_reference_begin(&reader,row))return 1;
  if(!frozen_words_next(&reader,&nlo)||!frozen_words_next(&reader,&nhi)||!frozen_words_next(&reader,&kind))return 2;
  for(i=0;i<3u;++i)if(!frozen_words_next(&reader,&fixed[i]))return 2;
  if(!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&count))return 2;
  n=(uint32_t)nlo|((uint32_t)nhi<<16);service_routines_input(&state,&io,n,&mission,&entry,input);for(i=0;i<3u;++i)if(input[i]!=fixed[i])return 3;
  for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&index)||index>=2048u||!frozen_words_next(&reader,&value))return 4;expected.words[index]=(agc_word)value;}
  for(i=count;i<SERVICE_ROUTINES_MAX_CHANGES;++i)if(!frozen_words_next(&reader,&lo)||!frozen_words_next(&reader,&hi)||lo!=0||hi!=0)return 5;
  if(!frozen_words_finished(&reader))return 6;
  iterations=0;if(kind>=24u)actual=(unsigned)(service_routines_block(&state,&io,mission,fixed[0],fixed[1],0,0,1024,&iterations)!=SERVICE_BLOCK_DONE);else actual=(unsigned)!service_routines_step(&state,&io,mission,entry,fixed[0],fixed[1],fixed[2]);
  if(actual!=status||io.superbank!=super||io.interrupt_enabled!=irq)return 7;
  for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 8;
  *completed=row+1u;
 }
 return 0;
}
