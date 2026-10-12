/* SPDX-License-Identifier: MIT */
#include "lm_vn1645_cases.h"
#include "lm_vn1645.h"
#include "lm_vn1645_phase_target.h"
#ifndef LM_VN1645_GROUP
#define LM_VN1645_GROUP 0
#endif
#if LM_VN1645_GROUP==0||LM_VN1645_GROUP==1
#define LM_VN1645_G1_FROZEN_IMPLEMENTATION
#include "lm_vn1645_g1_vectors.h"
#endif
#if LM_VN1645_GROUP==0||LM_VN1645_GROUP==2
#define LM_VN1645_G2_FROZEN_IMPLEMENTATION
#include "lm_vn1645_g2_vectors.h"
#endif

/* Only this group's phase is linked; any other phase is a failure. */
static uint8_t run(apollo_overlay_state *s,p66_bias_io *io,uint16_t phase){
 switch(phase){
#if LM_VN1645_GROUP==0||LM_VN1645_GROUP==1
 case LVN_VN1645:return lvn_vn1645(s,io);
 case LVN_MGA:return lvn_mga(s,io);
 case LVN_GET45:return lvn_get45(s,io);
#endif
#if LM_VN1645_GROUP==0||LM_VN1645_GROUP==2
 case LMG_GETMGA:return lvn_remap(s,lmg_getmga(s,io));
#endif
 default:return 0xffu;
 }
}
typedef uint8_t (*begin_fn)(frozen_words *,unsigned);
/* Reads `count` index/value pairs into the image and checks the zero padding. */
static int deltas(frozen_words *r,apollo_overlay_state *image,uint16_t width){
 uint16_t count,i,at,value;
 if(!frozen_words_next(r,&count)||count>width)return 0;
 for(i=0;i<width;++i){
  if(!frozen_words_next(r,&at)||!frozen_words_next(r,&value))return 0;
  if(i<count){if(at>=2048u)return 0;image->words[at]=value;}
  else if(at||value)return 0;
 }
 return 1;
}
static int group(begin_fn begin,uint16_t rows,uint16_t entry_width,uint16_t exit_width,
                 apollo_overlay_state *st,apollo_overlay_state *ex,unsigned *completed){
 uint16_t row,n,phase,irq,super,next,irq2,super2;unsigned i;p66_bias_io io;frozen_words r;
 for(row=0;row<rows;++row){
  if(!begin(&r,row)||!frozen_words_next(&r,&n)||!frozen_words_next(&r,&phase)||!frozen_words_next(&r,&irq)||!frozen_words_next(&r,&super))return 1;
  lm_vn1645_debris(st,&io,n);
  if(!deltas(&r,st,entry_width))return 2;
  io.interrupt_enabled=irq;io.superbank=super;
  for(i=0;i<2048u;++i)ex->words[i]=st->words[i];
  if(!frozen_words_next(&r,&next)||!frozen_words_next(&r,&irq2)||!frozen_words_next(&r,&super2)||!deltas(&r,ex,exit_width)||!frozen_words_finished(&r))return 3;
  if(run(st,&io,phase)!=next||io.interrupt_enabled!=irq2||io.superbank!=super2)return 4;
  st->words[2]&=077777u; /* rows hold Q in 15 bits */
  for(i=0;i<2048u;++i)if(st->words[i]!=ex->words[i])return 5;
  ++*completed;
 }
 return 0;
}
int lm_vn1645_phase_check(apollo_overlay_state *st,apollo_overlay_state *ex,unsigned *completed){
 int status=0;*completed=0;
#if LM_VN1645_GROUP==0||LM_VN1645_GROUP==1
 if(!status)status=group(lm_vn1645_g1_reference_begin,LM_VN1645_G1_FROZEN_ROWS,LM_VN1645_G1_ENTRY_CHANGES,LM_VN1645_G1_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_VN1645_GROUP==0||LM_VN1645_GROUP==2
 if(!status)status=group(lm_vn1645_g2_reference_begin,LM_VN1645_G2_FROZEN_ROWS,LM_VN1645_G2_ENTRY_CHANGES,LM_VN1645_G2_EXIT_CHANGES,st,ex,completed);
#endif
 return status;
}
