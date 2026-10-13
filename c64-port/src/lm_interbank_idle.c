/* SPDX-License-Identifier: MIT */
/* The Luminary099 INTER-BANK_COMMUNICATION routines the self-check idle loop
 * and most callers use, BANKCALL through POSTJUMP (lm_interbank_routines
 * entries 0-4), in a file of their own that compiles no operand table, so
 * the flight store can pack lm_interbank_idle_chunk beside the self-check
 * idle loop with lm_interbank but without the routines' site table
 * (docs/FLIGHT-STORE.md). Inline CADRs come from the program's operand
 * table, as in lm_interbank_routines. */
#include "lm_interbank_routines.h"
static bool inline_cadr(const agc_native_program *p,const apollo_overlay_state *s,const p66_bias_io *io,agc_word *literal){
 return !(s->words[2]>=02000u&&s->words[2]<=07777u)||agc_transfer_operand(p,s,io->superbank,s->words[2],literal);}
/* One body for the four, so the wrappers stay a few bytes each. */
static __attribute__((noinline)) enum exec_status ib(const agc_native_program *p,apollo_overlay_state *s,void *v,enum lm_interbank_entry entry){p66_bias_io *io=v;agc_word literal=0;
 if(!p||!s||!io)return EXEC_OUT_OF_SCOPE;
 if((entry==LM_IB_BANKCALL||entry==LM_IB_POSTJUMP)&&!inline_cadr(p,s,io,&literal))return EXEC_OUT_OF_SCOPE;
 return lm_interbank(s,io,entry,literal,0)?EXEC_OK:EXEC_OUT_OF_SCOPE;}
enum exec_status lm_ib_bankcall(const agc_native_program *p,apollo_overlay_state *s,void *v){return ib(p,s,v,LM_IB_BANKCALL);}
enum exec_status lm_ib_swcall(const agc_native_program *p,apollo_overlay_state *s,void *v){return ib(p,s,v,LM_IB_SWCALL);}
enum exec_status lm_ib_swreturn(const agc_native_program *p,apollo_overlay_state *s,void *v){return ib(p,s,v,LM_IB_SWRETURN);}
enum exec_status lm_ib_postjump(const agc_native_program *p,apollo_overlay_state *s,void *v){return ib(p,s,v,LM_IB_POSTJUMP);}
/* SWCALL -1 (04621, BANKCALL's INCR Q): RTB's `TC SWCALL -1` enters here so
 * that SWCALL returns one word later. Q is a return address from that TC. */
enum exec_status lm_ib_swcall_m1(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;
 if(!s||!io||s->words[7]!=0||s->words[2]>07776u)return EXEC_OUT_OF_SCOPE;
 s->words[2]=(agc_word)(s->words[2]+1u);s->words[5]=04622u;return EXEC_OK;}
#define AT(z) (uint16_t)(2048u+((z)-04000u))
static const agc_routine_entry entries[5]={
 {AT(04616),lm_ib_bankcall},{AT(04621),lm_ib_swcall_m1},{AT(04622),lm_ib_swcall},{AT(04631),lm_ib_swreturn},{AT(04635),lm_ib_postjump}};
const agc_routine_table lm_interbank_idle_chunk={entries,5};
