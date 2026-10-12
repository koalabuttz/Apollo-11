/* SPDX-License-Identifier: MIT */
#include "lm_conic_lambert_cases.h"
#include "lm_conic_lambert.h"
#include "lm_conic_lambert_target.h"
#define LM_CONIC_LAMBERT_FROZEN_IMPLEMENTATION
#include "lm_conic_lambert_vectors.h"
#define state (*st)
#define expected (*ex)
static apollo_overlay_state *st,*ex;
static void snapshot(void){unsigned i;for(i=0;i<2048u;++i)expected.words[i]=state.words[i];}
static int same(void){unsigned i;for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 0;return 1;}
/* An inadmissible entry leaves the image and the external fields unchanged. */
static int refused(p66_bias_io *io){p66_bias_io before=*io;snapshot();
 return !lm_conic_lambert(&state,io)&&same()&&io->interrupt_enabled==before.interrupt_enabled&&io->superbank==before.superbank;}
int lm_conic_lambert_target_check(apollo_overlay_state *state_image,apollo_overlay_state *expected_image,unsigned *completed){uint16_t row,i,n,irq,super,status,count,at,value,lo,hi;p66_bias_io io;unsigned actual;frozen_words reader;*completed=0;st=state_image;ex=expected_image;
 for(row=0;row<LM_CONIC_LAMBERT_FROZEN_ROWS;++row){
  if(!lm_conic_lambert_reference_begin(&reader,row)||!frozen_words_next(&reader,&n)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&count))return 1;
  lm_conic_lambert_input(&state,&io,n);snapshot();
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&at)||at>=2048u||!frozen_words_next(&reader,&value))return 2;expected.words[at]=value;}
  for(i=count;i<LM_CONIC_LAMBERT_MAX_CHANGES;++i)if(!frozen_words_next(&reader,&lo)||!frozen_words_next(&reader,&hi)||lo||hi)return 3;
  if(!frozen_words_finished(&reader))return 4;
  actual=lm_conic_lambert(&state,&io);
  state.words[2]&=077777u; /* rows hold Q in 15 bits */
  if(actual!=status||irq!=io.interrupt_enabled||super!=io.superbank)return 5;
  if(!same())return 6;
  *completed=(unsigned)row+1u;
 }
 {/* Entry frontier, banks, BANKSET, INTBIT15, EDOP, NEWJOB, ZERO, VAC, LOC,
   * a 16-bit word above A/L, X1, QPRET not INITVEL's return, and ITERCTR
   * neither 20 nor 5 (case 0: VAC 0401). */
  static const uint16_t invalid[20][2]={{5,06071},{4,010000},{3,1},{3,04000},{6,024001},{0165,0},{0115,040000},{023,040000},{067,1},{7,1},
   {0120,0},{0120,0402},{0164,03213},{030,0100000},{0447,077776},{0447,077766},{0453,022146},{0427,023},{0427,6},{0427,0}};
  for(row=0;row<20u;++row){lm_conic_lambert_input(&state,&io,0);state.words[invalid[row][0]]=invalid[row][1];if(!refused(&io))return 7;}
  lm_conic_lambert_input(&state,&io,0);io.interrupt_enabled=2;if(!refused(&io))return 8;
  lm_conic_lambert_input(&state,&io,0);io.superbank=1;if(!refused(&io))return 8;
  if(lm_conic_lambert(0,&io)||lm_conic_lambert(&state,0))return 9;
 }
 return 0;
}
