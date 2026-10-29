/* SPDX-License-Identifier: MIT */
#include "danzig_pending_check.h"
#include "danzig_pending_vectors.h"
static apollo_overlay_state image;
static uint16_t calls;
static enum exec_status pending_order(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 ++calls;s->words[AGC_EDOP]=0;s->words[5]=0;return EXEC_OK;}
static enum exec_status at_z(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)s;(void)io;return EXEC_OUT_OF_SCOPE;}
uint16_t dzp_row_count(void){return DZP_ROWS;}
static void load(const dzp_row *r,agc_word edop){uint16_t i;for(i=0;i<APOLLO_SHARED_WORDS;++i)image.words[i]=0;
 image.words[5]=r->cm?06030u:06061u;image.words[4]=r->fb;image.words[0164]=r->loc;image.words[0165]=r->bankset;image.words[AGC_EDOP]=edop;}
uint16_t dzp_check(uint16_t n,uint16_t *checks){const dzp_row *r=&dzp_rows[n];uint16_t f=0,key=(uint16_t)(r->pair+1u),z,steps=0;
 agc_word superbank=(agc_word)(r->bankset&0160u);
 agc_routine_entry site[1],basic[1];agc_routine_table interp[2],btab[1];agc_native_program p={0,0,0,0,0,0,AGC_TRANSFER_NONE,0164,0165,0,0,0};
 static const agc_word edops[3]={040000u,077777u,0u};uint8_t i;enum exec_status st;
 z=r->cm?AGC_CM_DANZIG:AGC_LM_DANZIG;
 site[0].physical=key;site[0].call=pending_order;basic[0].physical=z;basic[0].call=at_z;
 interp[0]=r->cm?agc_cm_danzig_marker:agc_lm_danzig_marker;interp[1].entries=site;interp[1].count=1;btab[0].entries=basic;btab[0].count=1;
 p.basic=btab;p.basic_count=1;p.interpretive=interp;p.interpretive_count=2;
 /* With the marker: LOC under BANKSET, the routine at the pair word +1. */
 load(r,r->edop);++*checks;if(agc_transfer_point(&p,&image,superbank)!=key)++f;
 ++*checks;if(agc_transfer_resolve(&p,&image,superbank)!=pending_order)++f;
 calls=0;st=agc_transfer_run(&p,&image,0,&superbank,AGC_TRANSFER_NONE,4,&steps);
 ++*checks;if(st!=EXEC_UNMAPPED||steps!=1u||calls!=1u)++f;
 /* EDOP +0, negative or -0: DANZIG by Z, as the original's CCS EDOP does not branch to OPJUMP. */
 for(i=0;i<3u;++i){load(r,edops[i]);++*checks;if(agc_transfer_point(&p,&image,superbank)!=z||agc_transfer_resolve(&p,&image,superbank)!=at_z)++f;}
 /* Without a marker, or with the other mission's: DANZIG by Z as before. */
 interp[0]=r->cm?agc_lm_danzig_marker:agc_cm_danzig_marker;load(r,r->edop);
 ++*checks;if(agc_transfer_point(&p,&image,superbank)!=z||agc_transfer_resolve(&p,&image,superbank)!=at_z)++f;
 p.interpretive=interp+1;p.interpretive_count=1;
 ++*checks;if(agc_transfer_point(&p,&image,superbank)!=z||agc_transfer_resolve(&p,&image,superbank)!=at_z)++f;
 /* An empty first table (basic-only programs register {0,0}) is no marker either. */
 {static const agc_routine_table none={0,0};p.interpretive=&none;
  if(agc_transfer_point(&p,&image,superbank)!=z||agc_transfer_resolve(&p,&image,superbank)!=at_z)++f;}
 /* NEWOPS stays the fetch point alongside the marker: LOC at the RTB operand word resolves there too. */
 interp[0]=r->cm?agc_cm_danzig_marker:agc_lm_danzig_marker;p.interpretive=interp;p.interpretive_count=2;
 p.interpreter_fetch=(uint16_t)(3u*1024u+(r->cm?037u:070u));load(r,0);image.words[5]=r->cm?06037u:06070u;
 ++*checks;if(agc_transfer_point(&p,&image,superbank)!=key||agc_transfer_resolve(&p,&image,superbank)!=pending_order)++f;
 return f;
}
