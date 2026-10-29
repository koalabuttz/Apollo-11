/* SPDX-License-Identifier: MIT */
#include "cm_ephemeris_time_cases.h"
#include "cm_ephemeris_moon_callers.h"
#include "cm_ephemeris_moon_callers_target.h"
#define CM_EPHEMERIS_MOON_CALLERS_FROZEN_IMPLEMENTATION
#include "cm_ephemeris_moon_callers_vectors.h"
static apollo_overlay_state state,expected;
static void input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){cm_ephemeris_time_input(m,io,n);m->words[0164]=02115;m->words[m->words[0120]+052u]=n%3u==0u?065673u:(n%3u==1u?062742u:023327u);}
int cm_ephemeris_moon_callers_target_check(unsigned *completed){uint16_t row,i,nlo,nhi,irq,super,status,count,at,value,lo,hi;p66_bias_io io;unsigned actual;frozen_words reader;*completed=0;
 for(row=0;row<CM_EPHEMERIS_MOON_CALLERS_FROZEN_ROWS;++row){
  if(!cm_ephemeris_moon_callers_reference_begin(&reader,row)||!frozen_words_next(&reader,&nlo)||!frozen_words_next(&reader,&nhi)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&count))return 1;
  if(nhi){return 10;}
  input(&state,&io,nlo);
  for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&at)||at>=2048u||!frozen_words_next(&reader,&value))return 2;expected.words[at]=value;}
  for(i=count;i<CM_EPHEMERIS_MOON_CALLERS_MAX_CHANGES;++i)if(!frozen_words_next(&reader,&lo)||!frozen_words_next(&reader,&hi)||lo||hi)return 3;
  if(!frozen_words_finished(&reader))return 4;
  actual=cm_ephemeris_moon_callers(&state,&io);
  if(actual!=status||irq!=io.interrupt_enabled||super!=io.superbank)return 5;
  for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 6;
  *completed=(unsigned)row+1u;
 }
 {static const uint16_t invalid[16][2]={{5,0},{7,1},{3,1},{3,04000},{4,1},{4,0100000},{6,1},{0165,0},{0120,0},{0120,0402},{0164,0},{030,0100000},{0163,077777},{023,1},{0115,0},{067,1}};
  for(row=0;row<16u;++row){
   input(&state,&io,0);state.words[invalid[row][0]]=invalid[row][1];
   for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
   if(cm_ephemeris_moon_callers(&state,&io))return 7;
   for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 8;
  }
  input(&state,&io,0);for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
  io.interrupt_enabled=2;if(cm_ephemeris_moon_callers(&state,&io))return 7;
  io.interrupt_enabled=0;io.superbank=1;if(cm_ephemeris_moon_callers(&state,&io))return 7;
  for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 8;
  if(cm_ephemeris_moon_callers(0,&io)||cm_ephemeris_moon_callers(&state,0))return 9;
  input(&state,&io,0);state.words[state.words[0120]+052u]=0;
  for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
  if(cm_ephemeris_moon_callers(&state,&io))return 7;
  for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 8;
 }
 return 0;
}
