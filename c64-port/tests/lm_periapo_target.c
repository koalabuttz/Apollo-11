/* SPDX-License-Identifier: MIT */
#include "lm_periapo_cases.h"
#include "lm_periapo.h"
#include "lm_periapo_target.h"
#define LM_PERIAPO_FROZEN_IMPLEMENTATION
#include "lm_periapo_vectors.h"
#define state (*st)
#define expected (*ex)
static apollo_overlay_state *st,*ex;
static void snapshot(void){unsigned i;for(i=0;i<2048u;++i)expected.words[i]=state.words[i];}
static int same(void){unsigned i;for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 0;return 1;}
/* An inadmissible entry leaves the image and the external fields unchanged. */
static int refused(p66_bias_io *io){p66_bias_io before=*io;snapshot();
 return !lm_periapo(&state,io)&&same()&&io->interrupt_enabled==before.interrupt_enabled&&io->superbank==before.superbank;}
int lm_periapo_target_check(apollo_overlay_state *state_image,apollo_overlay_state *expected_image,unsigned *completed){uint16_t row,i,n,irq,super,status,count,at,value,lo,hi;p66_bias_io io;unsigned actual;frozen_words reader;*completed=0;st=state_image;ex=expected_image;
 for(row=0;row<LM_PERIAPO_FROZEN_ROWS;++row){
  if(!lm_periapo_reference_begin(&reader,row)||!frozen_words_next(&reader,&n)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&count))return 1;
  lm_periapo_input(&state,&io,n);snapshot();
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&at)||at>=2048u||!frozen_words_next(&reader,&value))return 2;expected.words[at]=value;}
  for(i=count;i<LM_PERIAPO_MAX_CHANGES;++i)if(!frozen_words_next(&reader,&lo)||!frozen_words_next(&reader,&hi)||lo||hi)return 3;
  if(!frozen_words_finished(&reader))return 4;
  actual=lm_periapo(&state,&io);
  state.words[2]&=077777u; /* rows hold Q in 15 bits */
  if(actual!=status||irq!=io.interrupt_enabled||super!=io.superbank)return 5;
  if(!same())return 6;
  *completed=(unsigned)row+1u;
 }
 {/* Case 0 (PERIAPO1, VAC 0401): entry frontier, banks, BANKSET,
   * INTBIT15, EDOP, NEWJOB, ZERO, VAC, LOC, a 16-bit word above A/L,
   * QPRET not a PERIAPO1 call return, MODE, RTX1/RTX2 and PUSHLOC. */
  static const uint16_t invalid0[19][2]={{5,06071},{4,010000},{3,1},{3,04000},{6,046001},{0165,0},{0115,0},{023,040000},{067,1},{7,1},
   {0120,0},{0120,0402},{0164,02317},{030,0100000},{0453,070044},{0163,0},{02776,077776},{0166,0401},{0166,0442}};
  /* Case 3 (PERIAPO): X1 not RTX1; case 5 (SHIFTR1): RTX2 and MODE;
   * case 6 (MAXCHK): QPRET and INTBIT15; case 8 (PERIAPO1, Moon): RTX2. */
  static const uint16_t invalid[7][3]={{3,0447,077776},{5,02777,1},{5,0163,077776},{6,0453,045601},{6,0115,0},{6,0164,03637},{8,02777,0}};
  for(row=0;row<19u;++row){lm_periapo_input(&state,&io,0);state.words[invalid0[row][0]]=invalid0[row][1];if(!refused(&io))return 7;}
  for(row=0;row<7u;++row){lm_periapo_input(&state,&io,invalid[row][0]);state.words[invalid[row][1]]=invalid[row][2];if(!refused(&io))return 7;}
  lm_periapo_input(&state,&io,1);io.interrupt_enabled=2;if(!refused(&io))return 8;
  lm_periapo_input(&state,&io,1);io.superbank=1;if(!refused(&io))return 8;
  if(lm_periapo(0,&io)||lm_periapo(&state,0))return 9;
 }
 return 0;
}
