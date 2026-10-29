/* SPDX-License-Identifier: MIT */
#ifdef CM_EPHEMERIS_POSITIONS_DEBUG
#include <stdio.h>
#endif
#include "cm_ephemeris_time_cases.h"
#include "cm_ephemeris_positions.h"
#include "cm_ephemeris_positions_phase_target.h"
#ifndef CM_EPHEMERIS_POSITIONS_STAGE
#define CM_EPHEMERIS_POSITIONS_STAGE 0
#endif
#ifndef CM_EPHEMERIS_POSITIONS_COHORT
#define CM_EPHEMERIS_POSITIONS_COHORT 0
#endif
#define CM_EPHEMERIS_POSITIONS_PHASE_FROZEN_IMPLEMENTATION
#include "cm_ephemeris_positions_phase_vectors.h"
#define EXECUTE cm_ephemeris_positions_moon
static apollo_overlay_state state,expected;
static void input(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){cm_ephemeris_time_input(m,io,n);m->words[0164]=02110;m->words[m->words[0120]+052u]=n%2u?030231u:022557u;}
int cm_ephemeris_positions_phase_target_check(unsigned *completed){uint16_t row,i,n,stage,irq,super,status,ni,no,at,value;p66_bias_io io;frozen_words reader;unsigned actual,input_limit=CM_EPHEMERIS_POSITIONS_MAX_INPUT;*completed=0;
 for(row=0;row<CM_EPHEMERIS_POSITIONS_PHASE_FROZEN_ROWS;++row){
  if(!cm_ephemeris_positions_phase_reference_begin(&reader,row)||!frozen_words_next(&reader,&n)||!frozen_words_next(&reader,&stage)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&ni)||!frozen_words_next(&reader,&no)||stage!=CM_EPHEMERIS_POSITIONS_STAGE)return 1;
  input(&state,&io,n);
  for(i=0;i<input_limit;++i){if(!frozen_words_next(&reader,&at)||!frozen_words_next(&reader,&value))return 2;if(i<ni){if(at>=2048u)return 2;state.words[at]=value;}else if(at||value)return 2;}
  for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
  for(i=0;i<CM_EPHEMERIS_POSITIONS_MAX_OUTPUT;++i){if(!frozen_words_next(&reader,&at)||!frozen_words_next(&reader,&value))return 2;if(i<no){if(at>=2048u)return 2;expected.words[at]=value;}else if(at||value)return 2;}
  if(!frozen_words_finished(&reader))return 3;
  actual=EXECUTE(&state,&io);if(actual!=status||irq!=io.interrupt_enabled||super!=io.superbank)return 4;
  for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i]){
#ifdef CM_EPHEMERIS_POSITIONS_DEBUG
fprintf(stderr,"n%u phase%u word%04o expected%06o actual%06o\n",n,stage,i,expected.words[i],state.words[i]);
#endif
return 5;}
  *completed=(unsigned)row+1u;
 }
 /* Every phase rejects a wrong source PC atomically; external metadata and
  * null guards are checked against the actual final canonical fixture. */
 state.words[5]=0;for(i=0;i<2048u;++i)expected.words[i]=state.words[i];if(EXECUTE(&state,&io))return 6;for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 7;
 if(EXECUTE(0,&io)||EXECUTE(&state,0))return 8;
 return 0;
}
