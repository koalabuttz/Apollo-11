/* SPDX-License-Identifier: MIT */
#include "lm_time_free_fall_cases.h"
#include "lm_time_free_fall.h"
#include "lm_time_free_fall_phase_target.h"
#ifndef LM_TIME_FREE_FALL_GROUP
#define LM_TIME_FREE_FALL_GROUP 0
#endif
#if LM_TIME_FREE_FALL_GROUP==0||LM_TIME_FREE_FALL_GROUP==1
#define LM_TIME_FREE_FALL_G1_FROZEN_IMPLEMENTATION
#include "lm_time_free_fall_g1_vectors.h"
#endif
#if LM_TIME_FREE_FALL_GROUP==0||LM_TIME_FREE_FALL_GROUP==2
#define LM_TIME_FREE_FALL_G2_FROZEN_IMPLEMENTATION
#include "lm_time_free_fall_g2_vectors.h"
#endif
#if LM_TIME_FREE_FALL_GROUP==0||LM_TIME_FREE_FALL_GROUP==3
#define LM_TIME_FREE_FALL_G3_FROZEN_IMPLEMENTATION
#include "lm_time_free_fall_g3_vectors.h"
#endif
#if LM_TIME_FREE_FALL_GROUP==0||LM_TIME_FREE_FALL_GROUP==4
#define LM_TIME_FREE_FALL_G4_FROZEN_IMPLEMENTATION
#include "lm_time_free_fall_g4_vectors.h"
#endif

/* Only this group's phases are linked; any other phase is a failure. */
static uint8_t run(apollo_overlay_state *s,p66_bias_io *io,uint16_t phase){
 (void)io;
 switch(phase){
#if LM_TIME_FREE_FALL_GROUP==0||LM_TIME_FREE_FALL_GROUP==1
 case LTF_ENTRY:return ltf_entry(s);
 case LTF_CONMU:return ltf_conmu(s);
 case LTF_CONMU_END:return ltf_conmu_end(s);
 case LTF_RPRA:return ltf_rpra(s);
 case LTF_MAXRA:return ltf_maxra(s);
 case LTF_RAPO:return ltf_rapo(s);
#endif
#if LM_TIME_FREE_FALL_GROUP==0||LM_TIME_FREE_FALL_GROUP==2
 case LTF_CALCTPER:return ltf_calctper(s,io);
 case LTF_CALCTFF:return ltf_calctff(s,io);
 case LTF_TERMS:return ltf_terms(s,io);
 case LTF_QTERM:return ltf_qterm(s);
 case LTF_XTERM:return ltf_xterm(s);
 case LTF_SAVEDEN:return ltf_saveden(s);
 case LTF_FALLBACK:return ltf_fallback(s);
 case LTF_INBOUND:return ltf_inbound(s);
 case LTF_XTEST:return ltf_xtest(s);
#endif
#if LM_TIME_FREE_FALL_GROUP==0||LM_TIME_FREE_FALL_GROUP==3
 case LTF_SERIES:return ltf_series(s);
 case LTF_POLY:return ltf_poly(s);
 case LTF_SCALE:return ltf_scale(s);
 case LTF_ENDTFF:return ltf_endtff(s);
 case LTF_NEGTFF:return ltf_negtff(s);
 case LTF_MAXTFF1:return ltf_maxtff1(s);
 case LTF_MAXTFF:return ltf_maxtff(s);
#endif
#if LM_TIME_FREE_FALL_GROUP==0||LM_TIME_FREE_FALL_GROUP==4
 case LTF_TFFELL:return ltf_tffell(s);
 case LTF_TFFEL1:return ltf_tffel1(s,io);
 case LTF_ELLIPSE:return ltf_ellipse(s);
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
  lm_time_free_fall_debris(st,&io,n);
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
int lm_time_free_fall_phase_check(apollo_overlay_state *st,apollo_overlay_state *ex,unsigned *completed){
 int status=0;*completed=0;
#if LM_TIME_FREE_FALL_GROUP==0||LM_TIME_FREE_FALL_GROUP==1
 if(!status)status=group(lm_time_free_fall_g1_reference_begin,LM_TIME_FREE_FALL_G1_FROZEN_ROWS,LM_TIME_FREE_FALL_G1_ENTRY_CHANGES,LM_TIME_FREE_FALL_G1_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_TIME_FREE_FALL_GROUP==0||LM_TIME_FREE_FALL_GROUP==2
 if(!status)status=group(lm_time_free_fall_g2_reference_begin,LM_TIME_FREE_FALL_G2_FROZEN_ROWS,LM_TIME_FREE_FALL_G2_ENTRY_CHANGES,LM_TIME_FREE_FALL_G2_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_TIME_FREE_FALL_GROUP==0||LM_TIME_FREE_FALL_GROUP==3
 if(!status)status=group(lm_time_free_fall_g3_reference_begin,LM_TIME_FREE_FALL_G3_FROZEN_ROWS,LM_TIME_FREE_FALL_G3_ENTRY_CHANGES,LM_TIME_FREE_FALL_G3_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_TIME_FREE_FALL_GROUP==0||LM_TIME_FREE_FALL_GROUP==4
 if(!status)status=group(lm_time_free_fall_g4_reference_begin,LM_TIME_FREE_FALL_G4_FROZEN_ROWS,LM_TIME_FREE_FALL_G4_ENTRY_CHANGES,LM_TIME_FREE_FALL_G4_EXIT_CHANGES,st,ex,completed);
#endif
 return status;
}
