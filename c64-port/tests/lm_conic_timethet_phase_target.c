/* SPDX-License-Identifier: MIT */
#include "lm_conic_timethet_cases.h"
#include "lm_conic_timethet.h"
#include "lm_conic_timethet_phase_target.h"
#ifndef LM_CONIC_TIMETHET_GROUP
#define LM_CONIC_TIMETHET_GROUP 0
#endif
#if LM_CONIC_TIMETHET_GROUP==0||LM_CONIC_TIMETHET_GROUP==1
#define LM_CONIC_TIMETHET_G1_FROZEN_IMPLEMENTATION
#include "lm_conic_timethet_g1_vectors.h"
#endif
#if LM_CONIC_TIMETHET_GROUP==0||LM_CONIC_TIMETHET_GROUP==2
#define LM_CONIC_TIMETHET_G2_FROZEN_IMPLEMENTATION
#include "lm_conic_timethet_g2_vectors.h"
#endif
#if LM_CONIC_TIMETHET_GROUP==0||LM_CONIC_TIMETHET_GROUP==3
#define LM_CONIC_TIMETHET_G3_FROZEN_IMPLEMENTATION
#include "lm_conic_timethet_g3_vectors.h"
#endif
#if LM_CONIC_TIMETHET_GROUP==0||LM_CONIC_TIMETHET_GROUP==4
#define LM_CONIC_TIMETHET_G4_FROZEN_IMPLEMENTATION
#include "lm_conic_timethet_g4_vectors.h"
#endif

/* Only this group's phases are linked; any other phase is a failure. */
static uint8_t run(apollo_overlay_state *s,p66_bias_io *io,uint16_t phase){
 switch(phase){
#if LM_CONIC_TIMETHET_GROUP==0||LM_CONIC_TIMETHET_GROUP==1
 case LTT_ENTRY:return ltt_entry(s,io);
 case LTT_PARAM:return ltt_param(s,io);
 case LTT_GEOM:return ltt_geom(s,io);
 case LTT_UNITNORM:return ltt_unitnorm(s,io);
 case LTT_COLINEAR:return ltt_colinear(s,io);
 case LTT_PARAM2:return ltt_param2(s,io);
#endif
#if LM_CONIC_TIMETHET_GROUP==0||LM_CONIC_TIMETHET_GROUP==2
 case LTT_AFTERPARAM:return ltt_afterparam(s,io);
 case LTT_COGAOVFL:return ltt_cogaovfl(s,io);
 case LTT_GETX:return ltt_getx(s,io);
 case LTT_WLOOP:return ltt_wloop(s,io);
 case LTT_POLYCOEF:return ltt_polycoef(s,io);
 case LTT_ABTCONIC:return ltt_abtconic(s,io);
#endif
#if LM_CONIC_TIMETHET_GROUP==0||LM_CONIC_TIMETHET_GROUP==3
 case LTT_XCOMMON:return ltt_xcommon(s,io);
 case LTT_RESETX2:return ltt_resetx2(s,io);
 case LTT_360CHECK:return ltt_360check(s,io);
 case LTT_INVRSEQN:return ltt_invrseqn(s,io);
 case LTT_1WLOOP:return ltt_1wloop(s,io);
 case LTT_TRUE360X:return ltt_true360x(s,io);
 case LTT_INFINITY:return ltt_infinity(s,io);
#endif
#if LM_CONIC_TIMETHET_GROUP==0||LM_CONIC_TIMETHET_GROUP==4
 case LTT_COMMNOUT:return ltt_commnout(s,io);
 case LTT_DELTIME:return ltt_deltime(s,io);
 case LTT_DELTIME2:return ltt_deltime2(s,io);
 case LTT_DELTAIL:return ltt_deltail(s,io);
 case LTT_AFTERDT:return ltt_afterdt(s,io);
 case LTT_NEWSTATE:return ltt_newstate(s,io);
 case LTT_FINAL:return ltt_final(s,io);
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
  lm_conic_timethet_debris(st,&io,n);
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
int lm_conic_timethet_phase_check(apollo_overlay_state *st,apollo_overlay_state *ex,unsigned *completed){
 int status=0;*completed=0;
#if LM_CONIC_TIMETHET_GROUP==0||LM_CONIC_TIMETHET_GROUP==1
 if(!status)status=group(lm_conic_timethet_g1_reference_begin,LM_CONIC_TIMETHET_G1_FROZEN_ROWS,LM_CONIC_TIMETHET_G1_ENTRY_CHANGES,LM_CONIC_TIMETHET_G1_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_CONIC_TIMETHET_GROUP==0||LM_CONIC_TIMETHET_GROUP==2
 if(!status)status=group(lm_conic_timethet_g2_reference_begin,LM_CONIC_TIMETHET_G2_FROZEN_ROWS,LM_CONIC_TIMETHET_G2_ENTRY_CHANGES,LM_CONIC_TIMETHET_G2_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_CONIC_TIMETHET_GROUP==0||LM_CONIC_TIMETHET_GROUP==3
 if(!status)status=group(lm_conic_timethet_g3_reference_begin,LM_CONIC_TIMETHET_G3_FROZEN_ROWS,LM_CONIC_TIMETHET_G3_ENTRY_CHANGES,LM_CONIC_TIMETHET_G3_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_CONIC_TIMETHET_GROUP==0||LM_CONIC_TIMETHET_GROUP==4
 if(!status)status=group(lm_conic_timethet_g4_reference_begin,LM_CONIC_TIMETHET_G4_FROZEN_ROWS,LM_CONIC_TIMETHET_G4_ENTRY_CHANGES,LM_CONIC_TIMETHET_G4_EXIT_CHANGES,st,ex,completed);
#endif
 return status;
}
