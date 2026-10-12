/* SPDX-License-Identifier: MIT */
#include "lm_vn1645_cases.h"
#include "lm_vn1645.h"
#include "lm_vn1645_target.h"
#define LM_VN1645_FROZEN_IMPLEMENTATION
#include "lm_vn1645_vectors.h"
#define state (*st)
#define expected (*ex)
static apollo_overlay_state *st,*ex;
static void snapshot(void){unsigned i;for(i=0;i<2048u;++i)expected.words[i]=state.words[i];}
static int same(void){unsigned i;for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 0;return 1;}
/* An inadmissible entry leaves the image and the external fields unchanged. */
static int refused(p66_bias_io *io){p66_bias_io before=*io;snapshot();
 return !lm_vn1645(&state,io)&&same()&&io->interrupt_enabled==before.interrupt_enabled&&io->superbank==before.superbank;}
int lm_vn1645_target_check(apollo_overlay_state *state_image,apollo_overlay_state *expected_image,unsigned *completed){uint16_t row,i,n,irq,super,status,count,at,value,lo,hi;p66_bias_io io;unsigned actual;frozen_words reader;*completed=0;st=state_image;ex=expected_image;
 for(row=0;row<LM_VN1645_FROZEN_ROWS;++row){
  if(!lm_vn1645_reference_begin(&reader,row)||!frozen_words_next(&reader,&n)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&count))return 1;
  lm_vn1645_input(&state,&io,n);snapshot();
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&at)||at>=2048u||!frozen_words_next(&reader,&value))return 2;expected.words[at]=value;}
  for(i=count;i<LM_VN1645_MAX_CHANGES;++i)if(!frozen_words_next(&reader,&lo)||!frozen_words_next(&reader,&hi)||lo||hi)return 3;
  if(!frozen_words_finished(&reader))return 4;
  actual=lm_vn1645(&state,&io);
  state.words[2]&=077777u; /* rows hold Q in 15 bits */
  if(actual!=status||irq!=io.interrupt_enabled||super!=io.superbank)return 5;
  if(!same())return 6;
  *completed=(unsigned)row+1u;
 }
 {/* Case 0 (VAC 0401, PUSHLOC 0401): entry frontier, FBANK, LOC, banks,
   * BANKSET, INTBIT15 clear, EDOP, NEWJOB, ZERO, the VAC (zero, misaligned,
   * past the last area), a 16-bit word above A/L, PUSHLOC below the VAC and
   * past it, and MODE. */
  static const uint16_t invalid0[20][2]={{5,06071},{4,014000},{0164,03607},{3,1},{3,04000},{6,072001},{0165,0},{0115,0},{023,040000},{067,1},{7,1},
   {0120,0},{0120,0402},{0120,0735},{030,0100000},{0166,0400},{0166,0442},{0163,2},{0164,03634},{4,070000}};
  for(row=0;row<20u;++row){lm_vn1645_input(&state,&io,0);state.words[invalid0[row][0]]=invalid0[row][1];if(!refused(&io))return 7;}
  lm_vn1645_input(&state,&io,1);io.superbank=0100;if(!refused(&io))return 8;
  lm_vn1645_input(&state,&io,1);io.interrupt_enabled=2;if(!refused(&io))return 8;
  lm_vn1645_input(&state,&io,1);io.superbank=1;if(!refused(&io))return 8;
  if(lm_vn1645(0,&io)||lm_vn1645(&state,0))return 9;
 }
 return 0;
}
