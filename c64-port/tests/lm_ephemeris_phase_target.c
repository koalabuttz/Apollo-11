/* SPDX-License-Identifier: MIT */
#include "lm_ephemeris_cases.h"
#include "lm_ephemeris.h"
#include "lm_ephemeris_phase_target.h"
#ifndef LM_EPHEMERIS_GROUP
#define LM_EPHEMERIS_GROUP 0
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==1
#define LM_EPHEMERIS_G1_FROZEN_IMPLEMENTATION
#include "lm_ephemeris_g1_vectors.h"
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==2
#define LM_EPHEMERIS_G2_FROZEN_IMPLEMENTATION
#include "lm_ephemeris_g2_vectors.h"
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==3
#define LM_EPHEMERIS_G3_FROZEN_IMPLEMENTATION
#include "lm_ephemeris_g3_vectors.h"
#endif

/* Only this group's phases are linked; any other phase is a failure. */
static uint8_t run(apollo_overlay_state *s,p66_bias_io *io,uint16_t phase){
 (void)io;
 switch(phase){
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==1
 case LME_ENTRY:return lme_entry(s,io);
 case LME_POSITA:return lme_posita(s,io);
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==2
 case LME_POSITD:return lme_positd(s);
 case LME_POSITF:return lme_positf(s);
 case LME_MOON_VDEF:return lme_moon_vdef(s);
 case LME_SUN_ANGLES:return lme_sun_angles(s);
 case LME_SUN_VDEF:return lme_sun_vdef(s);
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==3
 case LME_MOON_UNIT:return lme_moon_unit(s,io);
 case LME_SUN_UNIT:return lme_sun_unit(s,io);
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
  lm_ephemeris_input(st,&io,n);
  if(!deltas(&r,st,entry_width))return 2;
  io.interrupt_enabled=irq;io.superbank=super;
  for(i=0;i<2048u;++i)ex->words[i]=st->words[i];
  if(!frozen_words_next(&r,&next)||!frozen_words_next(&r,&irq2)||!frozen_words_next(&r,&super2)||!deltas(&r,ex,exit_width)||!frozen_words_finished(&r))return 3;
  if(run(st,&io,phase)!=next||io.interrupt_enabled!=irq2||io.superbank!=super2)return 4;
  for(i=0;i<2048u;++i)if(st->words[i]!=ex->words[i])return 5;
  ++*completed;
 }
 return 0;
}
int lm_ephemeris_phase_check(apollo_overlay_state *st,apollo_overlay_state *ex,unsigned *completed){
 int status=0;*completed=0;
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==1
 if(!status)status=group(lm_ephemeris_g1_reference_begin,LM_EPHEMERIS_G1_FROZEN_ROWS,LM_EPHEMERIS_G1_ENTRY_CHANGES,LM_EPHEMERIS_G1_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==2
 if(!status)status=group(lm_ephemeris_g2_reference_begin,LM_EPHEMERIS_G2_FROZEN_ROWS,LM_EPHEMERIS_G2_ENTRY_CHANGES,LM_EPHEMERIS_G2_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==3
 if(!status)status=group(lm_ephemeris_g3_reference_begin,LM_EPHEMERIS_G3_FROZEN_ROWS,LM_EPHEMERIS_G3_ENTRY_CHANGES,LM_EPHEMERIS_G3_EXIT_CHANGES,st,ex,completed);
#endif
 return status;
}
