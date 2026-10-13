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
/* LM COMFLAG and DEBIT, entered directly by BURN_BABY_BURN's INVFLAG (CA Q;
 * TC DEBIT; COM; EXTEND; RXOR LCHAN; TCF COMFLAG). Both end in a TC through a
 * register that holds a plain TC word (DEBIT's TC Q, COMFLAG's TC L), so the
 * register must hold an ordinary address 0100..07777: Z gets it, Q gets 3 or 2. */
static bool lm_banks(const apollo_overlay_state *s,const p66_bias_io *io){const agc_word *w=s->words;
 return w[7]==0&&w[3]<=03400u&&!(w[3]&0377u)&&!(w[4]&~076000u)&&w[6]==(agc_word)(w[4]|(w[3]>>8))&&!(io->superbank&~0160u)&&io->interrupt_enabled<=1u;}
static uint16_t lm_erasable(const apollo_overlay_state *s,uint16_t a){return a<01400u?a:(uint16_t)(s->words[3]+(a&0377u));}
/* COMFLAG: INDEX ITEMP1; TS FLAGWRD0; LXCH ITEMP3; RELINT; TC L. TS with an
 * overflowing A stores the corrected word, leaves A +-1 and skips the LXCH. */
static enum exec_status lm_comflag(const agc_native_program *p,apollo_overlay_state *s,void *v){p66_bias_io *io=v;agc_word *w,idx,l,store;uint16_t f;int ov;
 (void)p;if(!s||!io||!lm_banks(s,io)||s->words[5]!=05511u)return EXEC_OUT_OF_SCOPE;
 w=s->words;idx=w[061];if(idx>01777u-074u)return EXEC_OUT_OF_SCOPE;
 f=lm_erasable(s,(uint16_t)(074u+idx));if(f<030u)return EXEC_OUT_OF_SCOPE;
 ov=agc_overflow(w[0]);store=agc_correct(w[0]);l=ov?w[1]:agc_extend(f==063u?store:w[063]);
 if(l<0100u||l>07777u)return EXEC_OUT_OF_SCOPE;
 w[f]=store;
 if(ov)w[0]=ov>0?1u:0177776u;else{w[063]=agc_correct(w[1]);w[1]=l;}
 io->interrupt_enabled=1;w[2]=2;w[5]=l;return EXEC_OK;}
/* DEBIT: AD ONE; INHINT; TS ITEMP3; CA LOW4; TS ITEMP1; INDEX ITEMP3; CA 0 -1
 * (the caller's ADRES at A); TS L; CA ZERO; EXTEND; DV ITEMP1; DXCH ITEMP1;
 * INDEX ITEMP1; CA FLAGWRD0; TS L; INDEX ITEMP2; CS BIT15; TC Q. */
static enum exec_status lm_debit(const agc_native_program *p,apollo_overlay_state *s,void *v){p66_bias_io *io=v;agc_word *w,adr,descriptor,index,rem,old;agc_register a;uint16_t f;
 if(!p||!s||!io||!lm_banks(s,io)||s->words[5]!=05522u)return EXEC_OUT_OF_SCOPE;
 w=s->words;if(w[2]<0100u||w[2]>07777u)return EXEC_OUT_OF_SCOPE;
 a=agc_add_register(w[0],agc_extend(1));if(agc_overflow(a))return EXEC_OUT_OF_SCOPE;
 adr=agc_correct(w[0]);if(agc_overflow(w[0])||adr>07777u)return EXEC_OUT_OF_SCOPE;
 if(adr<02000u){f=lm_erasable(s,adr);if(f<030u)return EXEC_OUT_OF_SCOPE;descriptor=f==061u?15u:f==063u?agc_correct(a):w[f];}
 /* A fixed ADRES is a compiled site operand (rule 3), never a rope read. */
 else if(!agc_transfer_operand(p,s,io->superbank,adr,&descriptor))return EXEC_OUT_OF_SCOPE;
 if(descriptor>14459u)return EXEC_OUT_OF_SCOPE;
 index=(agc_word)(descriptor/15u);rem=(agc_word)(descriptor%15u);f=lm_erasable(s,(uint16_t)(074u+index));if(f<030u)return EXEC_OUT_OF_SCOPE;
 w[063]=agc_correct(a);w[061]=index;w[062]=rem;old=w[f];
 w[1]=agc_extend(old);w[0]=agc_extend((agc_word)(AGC_MASK^(040000u>>rem)));
 io->interrupt_enabled=0;w[5]=w[2];w[2]=3;return EXEC_OK;}
/* Physical keys: bank*1024 + offset. DELAYJOB, TCGETCAD and WAKER run in
 * bank 0 (FB 0); LM UPENT2/DOWNENT2 in bank 010; the rest in fixed-fixed. */
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define B(bank,z) (uint16_t)((bank)*1024u+((z)-02000u))
static const agc_routine_entry lm_entries[13]={
 {B(0u,03735),lm_delayjob},{B(0u,03761),lm_tcgetcad},{B(0u,03765),lm_waker},{B(0u,03771),lm_waker_finish},
 {FF(05504),lm_upflag},{FF(05511),lm_comflag},{FF(05516),lm_downflag},{FF(05522),lm_debit},{FF(05544),lm_gentran},{FF(05545),lm_gentran_loop},{FF(05563),lm_b5off},
 {B(010u,03544),lm_upent2},{B(010u,03554),lm_downent2}};
static const agc_routine_entry cm_entries[13]={
 {B(0u,03732),cm_delayjob},{B(0u,03755),cm_tcgetcad},{B(0u,03761),cm_waker},{B(0u,03765),cm_waker_finish},
 {FF(05546),cm_upflag},{FF(05560),cm_downflag},{FF(05606),cm_gentran},{FF(05607),cm_gentran_loop},{FF(05625),cm_b5off},
 {FF(05631),cm_trfailof},{FF(05643),cm_trfailon},{FF(07722),cm_upent2},{FF(07740),cm_downent2}};
const agc_routine_table lm_service_routines={lm_entries,13},cm_service_routines={cm_entries,13};
#if defined(CM_SERVICE_DELAYJOB_ONLY)
/* For a C64 program that only enters the CM DELAYJOB (waitlist-entries kind
 * 3): that entry and TCGETCAD, where WAIT2 returns after inserting the task,
 * alone, so the rest of the table is not linked. */
static const agc_routine_entry cm_delayjob_only[2]={{B(0u,03732),cm_delayjob},{B(0u,03755),cm_tcgetcad}};
const agc_routine_table cm_service_delayjob_routines={cm_delayjob_only,2};
#endif
