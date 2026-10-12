/* SPDX-License-Identifier: MIT */
#include "lm_conic_kepler_cases.h"
#include "lm_conic_kepler.h"
#include "lm_conic_kepler_phase_target.h"
#ifndef LM_CONIC_KEPLER_GROUP
#define LM_CONIC_KEPLER_GROUP 0
#endif
#if LM_CONIC_KEPLER_GROUP==0||LM_CONIC_KEPLER_GROUP==1
#define LM_CONIC_KEPLER_G1_FROZEN_IMPLEMENTATION
#include "lm_conic_kepler_g1_vectors.h"
#endif
#if LM_CONIC_KEPLER_GROUP==0||LM_CONIC_KEPLER_GROUP==2
#define LM_CONIC_KEPLER_G2_FROZEN_IMPLEMENTATION
#include "lm_conic_kepler_g2_vectors.h"
#endif
#if LM_CONIC_KEPLER_GROUP==0||LM_CONIC_KEPLER_GROUP==3
#define LM_CONIC_KEPLER_G3_FROZEN_IMPLEMENTATION
#include "lm_conic_kepler_g3_vectors.h"
#endif
#if LM_CONIC_KEPLER_GROUP==0||LM_CONIC_KEPLER_GROUP==4
#define LM_CONIC_KEPLER_G4_FROZEN_IMPLEMENTATION
#include "lm_conic_kepler_g4_vectors.h"
#endif

/* Only this group's phases are linked; any other phase is a failure. */
static uint8_t run(apollo_overlay_state *s,p66_bias_io *io,uint16_t phase){
 (void)io;
 switch(phase){
#if LM_CONIC_KEPLER_GROUP==0||LM_CONIC_KEPLER_GROUP==1
 case LCK_ENTRY:return lck_entry(s);
 case LCK_1REV:return lck_1rev(s);
 case LCK_STOREMAX:return lck_storemax(s);
 case LCK_PERIODCH:return lck_periodch(s);
 case LCK_MODDONE:return lck_moddone(s);
 case LCK_STORBNDS:return lck_storbnds(s);
#endif
#if LM_CONIC_KEPLER_GROUP==0||LM_CONIC_KEPLER_GROUP==2
 case LCK_KEPLOOP:return lck_keploop(s);
 case LCK_DELTIME:return lck_deltime(s,io);
 case LCK_DELTIME2:return lck_deltime2(s,io);
 case LCK_DELTAIL:return lck_deltail(s);
 case LCK_KEPRET:return lck_kepret(s);
#endif
#if LM_CONIC_KEPLER_GROUP==0||LM_CONIC_KEPLER_GROUP==3
 case LCK_STOREMIN:return lck_storemin(s);
 case LCK_DXCOMP:return lck_dxcomp(s);
 case LCK_NDXCHNGE:return lck_ndxchnge(s);
 case LCK_POSDELX:return lck_posdelx(s);
 case LCK_PDXCHNGE:return lck_pdxchnge(s);
 case LCK_NEWDELX:return lck_newdelx(s);
 case LCK_BRNCHCTR:return lck_brnchctr(s);
#endif
#if LM_CONIC_KEPLER_GROUP==0||LM_CONIC_KEPLER_GROUP==4
 case LCK_BADX:return lck_badx(s);
 case LCK_TIMEOVFL:return lck_timeovfl(s);
 case LCK_NEGTOVFL:return lck_negtovfl(s);
 case LCK_CMNTOVFL:return lck_cmntovfl(s);
 case LCK_KEPCONVG:return lck_kepconvg(s);
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
  lm_conic_kepler_debris(st,&io,n);
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
int lm_conic_kepler_phase_check(apollo_overlay_state *st,apollo_overlay_state *ex,unsigned *completed){
 int status=0;*completed=0;
#if LM_CONIC_KEPLER_GROUP==0||LM_CONIC_KEPLER_GROUP==1
 if(!status)status=group(lm_conic_kepler_g1_reference_begin,LM_CONIC_KEPLER_G1_FROZEN_ROWS,LM_CONIC_KEPLER_G1_ENTRY_CHANGES,LM_CONIC_KEPLER_G1_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_CONIC_KEPLER_GROUP==0||LM_CONIC_KEPLER_GROUP==2
 if(!status)status=group(lm_conic_kepler_g2_reference_begin,LM_CONIC_KEPLER_G2_FROZEN_ROWS,LM_CONIC_KEPLER_G2_ENTRY_CHANGES,LM_CONIC_KEPLER_G2_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_CONIC_KEPLER_GROUP==0||LM_CONIC_KEPLER_GROUP==3
 if(!status)status=group(lm_conic_kepler_g3_reference_begin,LM_CONIC_KEPLER_G3_FROZEN_ROWS,LM_CONIC_KEPLER_G3_ENTRY_CHANGES,LM_CONIC_KEPLER_G3_EXIT_CHANGES,st,ex,completed);
#endif
#if LM_CONIC_KEPLER_GROUP==0||LM_CONIC_KEPLER_GROUP==4
 if(!status)status=group(lm_conic_kepler_g4_reference_begin,LM_CONIC_KEPLER_G4_FROZEN_ROWS,LM_CONIC_KEPLER_G4_ENTRY_CHANGES,LM_CONIC_KEPLER_G4_EXIT_CHANGES,st,ex,completed);
#endif
 return status;
}
