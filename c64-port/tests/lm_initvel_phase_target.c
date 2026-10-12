/* SPDX-License-Identifier: MIT */
#include "lm_initvel_cases.h"
#include "lm_initvel.h"
#include "lm_initvel_phase_target.h"
#ifndef LM_INITVEL_GROUP
#define LM_INITVEL_GROUP 0
#endif
#if LM_INITVEL_GROUP==0||LM_INITVEL_GROUP==1
#define LM_INITVEL_G1_FROZEN_IMPLEMENTATION
#include "lm_initvel_g1_vectors.h"
#endif
#if LM_INITVEL_GROUP==0||LM_INITVEL_GROUP==2
#define LM_INITVEL_G2_FROZEN_IMPLEMENTATION
#include "lm_initvel_g2_vectors.h"
#endif
#if LM_INITVEL_GROUP==0||LM_INITVEL_GROUP==3
#define LM_INITVEL_G3_FROZEN_IMPLEMENTATION
#include "lm_initvel_g3_vectors.h"
#endif
#if LM_INITVEL_GROUP==0||LM_INITVEL_GROUP==4
#define LM_INITVEL_G4_FROZEN_IMPLEMENTATION
#include "lm_initvel_g4_vectors.h"
#endif

/* Only this group's phases are linked; any other phase is a failure.
 * LAMBERT's returns continue as lm_initvel_step does. */
static uint8_t run(apollo_overlay_state *s,p66_bias_io *io,uint16_t phase){
 switch(phase){
#if LM_INITVEL_GROUP==0||LM_INITVEL_GROUP==1
 case LIV_INITVEL:return liv_initvel(s,io);
 case LIV_HAVEGUES:return liv_havegues(s,io);
 case LIV_INITVEL1:return liv_initvel1(s,io);
#endif
#if LM_INITVEL_GROUP==0||LM_INITVEL_GROUP==3
 case LIV_INITVEL2:return liv_initvel2(s,io);
 case LIV_INITVEL3:return liv_initvel3(s,io);
#endif
#if LM_INITVEL_GROUP==0||LM_INITVEL_GROUP==4
 case LIV_AFTERLAMBERT:return liv_afterlambert(s,io);
 case LIV_AFTERINTSTALL:return liv_afterintstall(s,io);
 case LIV_AFTERINTEGRVS:return liv_afterintegrvs(s,io);
 case LIV_INITVEL6:return liv_initvel6(s,io);
 case LIV_INITVEL7:return liv_initvel7(s,io);
#endif
#if LM_INITVEL_GROUP==0||LM_INITVEL_GROUP==2
 case LLA_LAMBERT:return lla_lambert(s,io);
 case LLA_INITV:return liv_remap(s,lla_initv(s,io));
 case LLA_TARGETV:return liv_remap(s,lla_targetv(s,io));
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
  lm_initvel_debris(st,&io,n);
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
int lm_initvel_phase_check(apollo_overlay_state *st,apollo_overlay_state *ex,unsigned *completed){
 int status=0;*completed=0;
#if LM_INITVEL_GROUP==0||LM_INITVEL_GROUP==1
 if(!status)status=group(lm_initvel_g1_reference_begin,LM_INITVEL_G1_FROZEN_ROWS,LM_INITVEL_G1_ENTRY_CHANGES,LM_INITVEL_G1_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_INITVEL_GROUP==0||LM_INITVEL_GROUP==2
 if(!status)status=group(lm_initvel_g2_reference_begin,LM_INITVEL_G2_FROZEN_ROWS,LM_INITVEL_G2_ENTRY_CHANGES,LM_INITVEL_G2_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_INITVEL_GROUP==0||LM_INITVEL_GROUP==3
 if(!status)status=group(lm_initvel_g3_reference_begin,LM_INITVEL_G3_FROZEN_ROWS,LM_INITVEL_G3_ENTRY_CHANGES,LM_INITVEL_G3_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_INITVEL_GROUP==0||LM_INITVEL_GROUP==4
 if(!status)status=group(lm_initvel_g4_reference_begin,LM_INITVEL_G4_FROZEN_ROWS,LM_INITVEL_G4_ENTRY_CHANGES,LM_INITVEL_G4_EXIT_CHANGES,st,ex,completed);
#endif
 return status;
}
