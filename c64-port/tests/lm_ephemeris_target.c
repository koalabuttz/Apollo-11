/* SPDX-License-Identifier: MIT */
#include "lm_ephemeris_cases.h"
#include "lm_ephemeris.h"
#include "lm_ephemeris_target.h"
#define LM_EPHEMERIS_FROZEN_IMPLEMENTATION
#include "lm_ephemeris_vectors.h"
#define state (*st)
#define expected (*ex)
static apollo_overlay_state *st,*ex;
static void snapshot(void){unsigned i;for(i=0;i<2048u;++i)expected.words[i]=state.words[i];}
static int same(void){unsigned i;for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 0;return 1;}
/* An inadmissible entry leaves the image and the external fields unchanged. */
static int refused(p66_bias_io *io){p66_bias_io before=*io;snapshot();
 return !lm_ephemeris(&state,io)&&same()&&io->interrupt_enabled==before.interrupt_enabled&&io->superbank==before.superbank;}
int lm_ephemeris_target_check(apollo_overlay_state *state_image,apollo_overlay_state *expected_image,unsigned *completed){uint16_t row,i,n,irq,super,status,count,at,value,lo,hi;p66_bias_io io;unsigned actual;frozen_words reader;*completed=0;st=state_image;ex=expected_image;
 for(row=0;row<LM_EPHEMERIS_FROZEN_ROWS;++row){
  if(!lm_ephemeris_reference_begin(&reader,row)||!frozen_words_next(&reader,&n)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&count))return 1;
  lm_ephemeris_input(&state,&io,n);snapshot();
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&at)||at>=2048u||!frozen_words_next(&reader,&value))return 2;expected.words[at]=value;}
  for(i=count;i<LM_EPHEMERIS_MAX_CHANGES;++i)if(!frozen_words_next(&reader,&lo)||!frozen_words_next(&reader,&hi)||lo||hi)return 3;
  if(!frozen_words_finished(&reader))return 4;
  actual=lm_ephemeris(&state,&io);
  if(actual!=status||irq!=io.interrupt_enabled||super!=io.superbank)return 5;
  if(!same())return 6;
  *completed=(unsigned)row+1u;
 }
 {static const uint16_t invalid[16][2]={{5,06071},{7,1},{3,1},{3,04000},{4,030000},{6,032001},{0165,0},{0120,0},{0120,0402},{0164,03665},{030,0100000},{0163,077777},{023,1},{0115,040000},{067,1},{0453,022564}};
  for(row=0;row<16u;++row){lm_ephemeris_input(&state,&io,0);state.words[invalid[row][0]]=invalid[row][1];if(!refused(&io))return 7;}
  lm_ephemeris_input(&state,&io,63);state.words[state.words[0120]+052u]=022563;if(!refused(&io))return 7;
  lm_ephemeris_input(&state,&io,0);io.interrupt_enabled=2;if(!refused(&io))return 8;
  lm_ephemeris_input(&state,&io,0);io.superbank=1;if(!refused(&io))return 8;
  if(lm_ephemeris(0,&io)||lm_ephemeris(&state,0))return 9;
 }
 return 0;
}
