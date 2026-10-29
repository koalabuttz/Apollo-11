/* SPDX-License-Identifier: MIT */
#include "lm_conic_lambert_cases.h"
#include "lm_conic_lambert.h"
#include "lm_conic_lambert_phase_target.h"
#ifndef LM_CONIC_LAMBERT_GROUP
#define LM_CONIC_LAMBERT_GROUP 0
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==1
#define LM_CONIC_LAMBERT_G1_FROZEN_IMPLEMENTATION
#include "lm_conic_lambert_g1_vectors.h"
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==2
#define LM_CONIC_LAMBERT_G2_FROZEN_IMPLEMENTATION
#include "lm_conic_lambert_g2_vectors.h"
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==3
#define LM_CONIC_LAMBERT_G3_FROZEN_IMPLEMENTATION
#include "lm_conic_lambert_g3_vectors.h"
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==4
#define LM_CONIC_LAMBERT_G4_FROZEN_IMPLEMENTATION
#include "lm_conic_lambert_g4_vectors.h"
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==5
#define LM_CONIC_LAMBERT_G5_FROZEN_IMPLEMENTATION
#include "lm_conic_lambert_g5_vectors.h"
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==6
#define LM_CONIC_LAMBERT_G6_FROZEN_IMPLEMENTATION
#include "lm_conic_lambert_g6_vectors.h"
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==7
#define LM_CONIC_LAMBERT_G7_FROZEN_IMPLEMENTATION
#include "lm_conic_lambert_g7_vectors.h"
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==8
#define LM_CONIC_LAMBERT_G8_FROZEN_IMPLEMENTATION
#include "lm_conic_lambert_g8_vectors.h"
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==9
#define LM_CONIC_LAMBERT_G9_FROZEN_IMPLEMENTATION
#include "lm_conic_lambert_g9_vectors.h"
#endif

/* Only this group's phases are linked; any other phase is a failure. The
 * shared time-theta phases continue as lm_conic_lambert_step does. */
static uint8_t run(apollo_overlay_state *s,p66_bias_io *io,uint16_t phase){
 switch(phase){
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==1
 case LLA_LAMBERT:return lla_lambert(s,io);
 case LTT_GEOM:return lla_remap(s,ltt_geom(s,io));
 case LTT_UNITNORM:return lla_remap(s,ltt_unitnorm(s,io));
 case LTT_COLINEAR:return lla_remap(s,ltt_colinear(s,io));
 case LTT_HAVENORM:return lla_remap(s,ltt_havenorm(s,io));
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==2
 case LLA_AFTERGEOM:return lla_aftergeom(s,io);
 case LLA_UPLIM:return lla_uplim(s,io);
 case LLA_MAXCOGA:return lla_maxcoga(s,io);
 case LLA_LOLIM:return lla_lolim(s,io);
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==3
 case LLA_MINCOGA:return lla_mincoga(s,io);
 case LLA_NOGUESS:return lla_noguess(s,io);
 case LLA_LAMBLOOP:return lla_lambloop(s,io);
 case LLA_AFTERGETX:return lla_aftergetx(s,io);
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==4
 case LTT_GETX:return lla_remap(s,ltt_getx(s,io));
 case LTT_WLOOP:return lla_remap(s,ltt_wloop(s,io));
 case LTT_POLYCOEF:return lla_remap(s,ltt_polycoef(s,io));
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==5
 case LTT_XCOMMON:return lla_remap(s,ltt_xcommon(s,io));
 case LTT_RESETX2:return lla_remap(s,ltt_resetx2(s,io));
 case LTT_360CHECK:return lla_remap(s,ltt_360check(s,io));
 case LTT_INVRSEQN:return lla_remap(s,ltt_invrseqn(s,io));
 case LTT_1WLOOP:return lla_remap(s,ltt_1wloop(s,io));
 case LTT_TRUE360X:return lla_remap(s,ltt_true360x(s,io));
 case LTT_INFINITY:return lla_remap(s,ltt_infinity(s,io));
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==6
 case LTT_DELTIME:return lla_remap(s,ltt_deltime(s,io));
 case LTT_DELTIME2:return lla_remap(s,ltt_deltime2(s,io));
 case LTT_DELTAIL:return lla_remap(s,ltt_deltail(s,io));
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==7
 case LLA_AFTERDT:return lla_afterdt(s,io);
 case LLA_AFTERITER:return lla_afteriter(s,io);
 case LLA_NEGP:return lla_negp(s,io);
 case LLA_HIENERGY:return lla_hienergy(s,io);
 case LLA_COMMONLM:return lla_commonlm(s,io);
 case LLA_BIGTIME:return lla_bigtime(s,io);
 case LLA_LOENERGY:return lla_loenergy(s,io);
 case LLA_SUFFCHEK:return lla_suffchek(s,io);
 case LLA_360LAMB:return lla_360lamb(s,io);
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==8
 case LLA_INITV:return lla_initv(s,io);
 case LLA_TARGETV:return lla_targetv(s,io);
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==9
 case LLA_ITERATOR:return lla_iterator(s,io);
 case LLA_SGNCHECK:return lla_sgncheck(s,io);
 case LLA_MINCHECK:return lla_mincheck(s,io);
 case LLA_MODNGDEL:return lla_modngdel(s,io);
 case LLA_FIRSTIME:return lla_firstime(s,io);
 case LLA_POSDEL:return lla_posdel(s,io);
 case LLA_MAXCHECK:return lla_maxcheck(s,io);
 case LLA_DELOK:return lla_delok(s,io);
 case LLA_NEWDEL:return lla_newdel(s,io);
 case LLA_MODPSDEL:return lla_modpsdel(s,io);
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
  lm_conic_lambert_debris(st,&io,n);
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
int lm_conic_lambert_phase_check(apollo_overlay_state *st,apollo_overlay_state *ex,unsigned *completed){
 int status=0;*completed=0;
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==1
 if(!status)status=group(lm_conic_lambert_g1_reference_begin,LM_CONIC_LAMBERT_G1_FROZEN_ROWS,LM_CONIC_LAMBERT_G1_ENTRY_CHANGES,LM_CONIC_LAMBERT_G1_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==2
 if(!status)status=group(lm_conic_lambert_g2_reference_begin,LM_CONIC_LAMBERT_G2_FROZEN_ROWS,LM_CONIC_LAMBERT_G2_ENTRY_CHANGES,LM_CONIC_LAMBERT_G2_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==3
 if(!status)status=group(lm_conic_lambert_g3_reference_begin,LM_CONIC_LAMBERT_G3_FROZEN_ROWS,LM_CONIC_LAMBERT_G3_ENTRY_CHANGES,LM_CONIC_LAMBERT_G3_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==4
 if(!status)status=group(lm_conic_lambert_g4_reference_begin,LM_CONIC_LAMBERT_G4_FROZEN_ROWS,LM_CONIC_LAMBERT_G4_ENTRY_CHANGES,LM_CONIC_LAMBERT_G4_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==5
 if(!status)status=group(lm_conic_lambert_g5_reference_begin,LM_CONIC_LAMBERT_G5_FROZEN_ROWS,LM_CONIC_LAMBERT_G5_ENTRY_CHANGES,LM_CONIC_LAMBERT_G5_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==6
 if(!status)status=group(lm_conic_lambert_g6_reference_begin,LM_CONIC_LAMBERT_G6_FROZEN_ROWS,LM_CONIC_LAMBERT_G6_ENTRY_CHANGES,LM_CONIC_LAMBERT_G6_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==7
 if(!status)status=group(lm_conic_lambert_g7_reference_begin,LM_CONIC_LAMBERT_G7_FROZEN_ROWS,LM_CONIC_LAMBERT_G7_ENTRY_CHANGES,LM_CONIC_LAMBERT_G7_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==8
 if(!status)status=group(lm_conic_lambert_g8_reference_begin,LM_CONIC_LAMBERT_G8_FROZEN_ROWS,LM_CONIC_LAMBERT_G8_ENTRY_CHANGES,LM_CONIC_LAMBERT_G8_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_CONIC_LAMBERT_GROUP==0||LM_CONIC_LAMBERT_GROUP==9
 if(!status)status=group(lm_conic_lambert_g9_reference_begin,LM_CONIC_LAMBERT_G9_FROZEN_ROWS,LM_CONIC_LAMBERT_G9_ENTRY_CHANGES,LM_CONIC_LAMBERT_G9_EXIT_CHANGES,st,ex,completed);
#endif
 return status;
}
