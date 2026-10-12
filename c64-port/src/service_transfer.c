/* SPDX-License-Identifier: MIT */
#include "service_transfer.h"
#include "service_routines.h"
#include "cm_native_image.h"
#include "lm_service_sites.inc"
#include "cm_service_sites.inc"
const agc_site_operand *lm_service_site_table(uint16_t *count){if(count)*count=LM_SERVICE_SITES_COUNT;return lm_service_sites;}
const agc_site_operand *cm_service_site_table(uint16_t *count){if(count)*count=CM_SERVICE_SITES_COUNT;return cm_service_sites;}
static const agc_native_program *reading;
static bool fixed_reader(void *ctx,uint16_t physical,agc_word *word){(void)ctx;return reading&&reading->fixed_data&&reading->fixed_data(reading,physical,word);}
static enum exec_status run(const agc_native_program *p,apollo_overlay_state *s,void *v,enum service_routines_mission mission,enum service_routines_entry entry){
 p66_bias_io lio,*io;cm_native_io *cio=0;agc_word q,fixed0=0,fixed1=0;uint16_t iterations=0;bool ok;
 if(!p||!s||!v)return EXEC_OUT_OF_SCOPE;
 if(mission==SERVICE_ROUTINES_CM){cio=v;if(cio->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;lio.superbank=(uint16_t)cio->superbank;lio.interrupt_enabled=(uint16_t)cio->interrupt_enabled;io=&lio;}else io=v;
 q=s->words[2];
 /* Fixed inline words: the caller's compiled operands. Erasable ones are read by the step itself. */
 if(entry==SERVICE_UPFLAG||entry==SERVICE_DOWNFLAG||entry==SERVICE_GENTRAN||entry==SERVICE_GENTRAN_LOOP){
  if(q>=02000u&&q<=07777u&&!agc_transfer_operand(p,s,io->superbank,q,&fixed0))return EXEC_OUT_OF_SCOPE;
  if((entry==SERVICE_GENTRAN||entry==SERVICE_GENTRAN_LOOP)&&q>=01777u&&q<=07776u&&!agc_transfer_operand(p,s,io->superbank,(agc_word)(q+1u),&fixed1))return EXEC_OUT_OF_SCOPE;
 }
 /* One GENTRAN iteration from either point (the block driver picks the entry by Z), ending at the back edge or the exit. */
 if(entry==SERVICE_GENTRAN||entry==SERVICE_GENTRAN_LOOP){
  reading=p;ok=service_routines_block(s,io,mission,fixed0,fixed1,fixed_reader,0,1,&iterations)!=SERVICE_BLOCK_BAD;reading=0;
 }else ok=service_routines_step(s,io,mission,entry,fixed0,fixed1,0);
 if(!ok)return EXEC_OUT_OF_SCOPE;
 if(cio)cio->interrupt_enabled=io->interrupt_enabled;
 return EXEC_OK;
}
#define LM(name,entry) static enum exec_status lm_##name(const agc_native_program *p,apollo_overlay_state *s,void *io){return run(p,s,io,SERVICE_ROUTINES_LM,entry);}
#define CM(name,entry) static enum exec_status cm_##name(const agc_native_program *p,apollo_overlay_state *s,void *io){return run(p,s,io,SERVICE_ROUTINES_CM,entry);}
#define BOTH(name,entry) LM(name,entry) CM(name,entry)
BOTH(upent2,SERVICE_UPENT2) BOTH(downent2,SERVICE_DOWNENT2) BOTH(upflag,SERVICE_UPFLAG) BOTH(downflag,SERVICE_DOWNFLAG)
BOTH(delayjob,SERVICE_DELAYJOB) BOTH(tcgetcad,SERVICE_TCGETCAD) BOTH(waker,SERVICE_WAKER) BOTH(waker_finish,SERVICE_WAKER_FINISH)
BOTH(gentran,SERVICE_GENTRAN) BOTH(gentran_loop,SERVICE_GENTRAN_LOOP) BOTH(b5off,SERVICE_B5OFF)
CM(trfailof,SERVICE_TRFAILOF) CM(trfailon,SERVICE_TRFAILON)
/* Physical keys: bank*1024 + offset. DELAYJOB, TCGETCAD and WAKER run in
 * bank 0 (FB 0); LM UPENT2/DOWNENT2 in bank 010; the rest in fixed-fixed. */
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define B(bank,z) (uint16_t)((bank)*1024u+((z)-02000u))
static const agc_routine_entry lm_entries[11]={
 {B(0u,03735),lm_delayjob},{B(0u,03761),lm_tcgetcad},{B(0u,03765),lm_waker},{B(0u,03771),lm_waker_finish},
 {FF(05504),lm_upflag},{FF(05516),lm_downflag},{FF(05544),lm_gentran},{FF(05545),lm_gentran_loop},{FF(05563),lm_b5off},
 {B(010u,03544),lm_upent2},{B(010u,03554),lm_downent2}};
static const agc_routine_entry cm_entries[13]={
 {B(0u,03732),cm_delayjob},{B(0u,03755),cm_tcgetcad},{B(0u,03761),cm_waker},{B(0u,03765),cm_waker_finish},
 {FF(05546),cm_upflag},{FF(05560),cm_downflag},{FF(05606),cm_gentran},{FF(05607),cm_gentran_loop},{FF(05625),cm_b5off},
 {FF(05631),cm_trfailof},{FF(05643),cm_trfailon},{FF(07722),cm_upent2},{FF(07740),cm_downent2}};
const agc_routine_table lm_service_routines={lm_entries,11},cm_service_routines={cm_entries,13};
