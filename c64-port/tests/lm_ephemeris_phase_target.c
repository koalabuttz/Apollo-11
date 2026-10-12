/* SPDX-License-Identifier: MIT */
#include "lm_ephemeris_cases.h"
#include "lm_ephemeris.h"
#include "lm_ephemeris_routines.h"
/* INTERPRETER NEWOPS 06070, the interpreter fetch point. */
#define LM_EPHEMERIS_FETCH ((uint16_t)(2048u+(06070u-04000u)))
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
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==4
#define LM_EPHEMERIS_G4_FROZEN_IMPLEMENTATION
#include "lm_ephemeris_g4_vectors.h"
#endif

/* Each frozen step runs through the control-transfer trampoline: the
 * program's interpretive table holds only this group's phase routines, keyed
 * by their frontier LOCs, so the trampoline must select the step's own
 * routine from the entry image and run exactly that one step. */
static agc_routine route(uint16_t phase){
 switch(phase){
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==1
 case LME_ENTRY:return lm_ephemeris_route_lspos;
 case LME_POSITA:return lm_ephemeris_route_posita;
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==2
 case LME_POSITD:return lm_ephemeris_route_positd;
 case LME_POSITF:return lm_ephemeris_route_positf;
 case LME_MOON_VDEF:return lm_ephemeris_route_moon_vdef;
 case LME_SUN_ANGLES:return lm_ephemeris_route_sun_angles;
 case LME_SUN_VDEF:return lm_ephemeris_route_sun_vdef;
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==3
 case LME_MOON_UNIT:return lm_ephemeris_route_moon_unit;
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==4
 case LME_SUN_UNIT:return lm_ephemeris_route_sun_unit;
#endif
 default:return 0;
 }
}
static const agc_routine_entry entries[]={
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==1
 {LM_EPHEMERIS_KEY(03664),lm_ephemeris_route_lspos},{LM_EPHEMERIS_KEY(03700),lm_ephemeris_route_posita},
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==2
 {LM_EPHEMERIS_KEY(03722),lm_ephemeris_route_positd},{LM_EPHEMERIS_KEY(03743),lm_ephemeris_route_positf},
 {LM_EPHEMERIS_KEY(03752),lm_ephemeris_route_moon_vdef},
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==3
 {LM_EPHEMERIS_KEY(03753),lm_ephemeris_route_moon_unit},
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==2
 {LM_EPHEMERIS_KEY(03756),lm_ephemeris_route_sun_angles},{LM_EPHEMERIS_KEY(03763),lm_ephemeris_route_sun_vdef},
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==4
 {LM_EPHEMERIS_KEY(03764),lm_ephemeris_route_sun_unit},
#endif
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==1
 {LM_EPHEMERIS_KEY(03775),lm_ephemeris_route_lspos},
#endif
};
static const agc_routine_table table={entries,(uint16_t)(sizeof entries/sizeof entries[0])};
static const agc_native_program program={0,0,&table,1,0,0,LM_EPHEMERIS_FETCH,0164,0165,0,0,0};
/* One step: the routine the trampoline selects must be the phase's own, and
 * it must stop at the expected frontier after exactly one call. */
static int run(apollo_overlay_state *s,p66_bias_io *io,uint16_t phase,const apollo_overlay_state *ex,agc_word super2){
 uint16_t steps=0,stop=agc_transfer_point(&program,ex,super2);
 if(!route(phase)||agc_transfer_resolve(&program,s,io->superbank)!=route(phase))return 0;
 return agc_transfer_run(&program,s,io,&io->superbank,stop,1u,&steps)==EXEC_OK&&steps==1u;
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
  (void)next;
  if(!run(st,&io,phase,ex,(agc_word)super2)||io.interrupt_enabled!=irq2||io.superbank!=super2)return 4;
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
#if LM_EPHEMERIS_GROUP==0||LM_EPHEMERIS_GROUP==4
 if(!status)status=group(lm_ephemeris_g4_reference_begin,LM_EPHEMERIS_G4_FROZEN_ROWS,LM_EPHEMERIS_G4_ENTRY_CHANGES,LM_EPHEMERIS_G4_EXIT_CHANGES,st,ex,completed);
#endif
 return status;
}
