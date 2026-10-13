/* SPDX-License-Identifier: MIT */
/* The Comanche055 INTER-BANK_COMMUNICATION routines the self-check idle loop
 * and most callers use, BANKCALL through POSTJUMP (cm_interbank_routines
 * entries 0-4), in a file of their own that compiles no operand table, so
 * the flight store can pack cm_interbank_idle_chunk beside the self-check
 * idle loop with cm_interbank_complete but without the routines' site table
 * (docs/FLIGHT-STORE.md). Inline CADRs come from the program's operand
 * table, as in cm_interbank_routines. */
#include "cm_interbank_routines.h"
static bool inline_cadr(const agc_native_program *p,const apollo_overlay_state *s,const cm_native_io *io,agc_word *literal){
 return !(s->words[2]>=02000u&&s->words[2]<=07777u)||agc_transfer_operand(p,s,io->superbank,s->words[2],literal);}
/* One body for the four, so the wrappers stay a few bytes each. */
static __attribute__((noinline)) enum exec_status ib(const agc_native_program *p,apollo_overlay_state *s,void *v,enum cm_interbank_complete_entry entry){cm_native_io *io=v;agc_word literal=0;
 if(!p||!s||!io)return EXEC_OUT_OF_SCOPE;
 if((entry==CM_IC_BANKCALL||entry==CM_IC_POSTJUMP)&&!inline_cadr(p,s,io,&literal))return EXEC_OUT_OF_SCOPE;
 return cm_interbank_complete(s,io,entry,literal,0)?EXEC_OK:EXEC_OUT_OF_SCOPE;}
enum exec_status cm_ib_bankcall(const agc_native_program *p,apollo_overlay_state *s,void *v){return ib(p,s,v,CM_IC_BANKCALL);}
enum exec_status cm_ib_swcall(const agc_native_program *p,apollo_overlay_state *s,void *v){return ib(p,s,v,CM_IC_SWCALL);}
enum exec_status cm_ib_swreturn(const agc_native_program *p,apollo_overlay_state *s,void *v){return ib(p,s,v,CM_IC_SWRETURN);}
enum exec_status cm_ib_postjump(const agc_native_program *p,apollo_overlay_state *s,void *v){return ib(p,s,v,CM_IC_POSTJUMP);}
/* SWCALL -1 (04665, BANKCALL's INCR Q): RTB's `TC SWCALL -1` enters here so
 * that SWCALL returns one word later. Q is a return address from that TC. */
enum exec_status cm_ib_swcall_m1(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;
 if(!s||!io||s->words[7]!=0||s->words[2]>07776u)return EXEC_OUT_OF_SCOPE;
 s->words[2]=(agc_word)(s->words[2]+1u);s->words[5]=04666u;return EXEC_OK;}
#define AT(z) (uint16_t)(2048u+((z)-04000u))
static const agc_routine_entry entries[5]={
 {AT(04662),cm_ib_bankcall},{AT(04665),cm_ib_swcall_m1},{AT(04666),cm_ib_swcall},{AT(04675),cm_ib_swreturn},{AT(04701),cm_ib_postjump}};
const agc_routine_table cm_interbank_idle_chunk={entries,5};
