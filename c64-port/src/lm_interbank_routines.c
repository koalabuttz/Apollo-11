/* SPDX-License-Identifier: MIT */
#include "lm_interbank_routines.h"
#include "lm_interbank_sites.inc"
const agc_site_operand *lm_interbank_site_table(uint16_t *count){if(count)*count=LM_INTERBANK_SITES_COUNT;return lm_interbank_sites;}
static enum exec_status run(const agc_native_program *p,apollo_overlay_state *s,void *v,enum lm_interbank_entry entry){p66_bias_io *io=v;agc_word literal=0,word=0;uint16_t physical;
 if(!p||!s||!io)return EXEC_OUT_OF_SCOPE;
 /* Fixed inline CADR: the caller's compiled operand. Erasable ones are read by lm_interbank after its own stores. */
 if((entry==LM_IB_BANKCALL||entry==LM_IB_IBNKCALL||entry==LM_IB_POSTJUMP||entry==LM_IB_USPRCADR)&&s->words[2]>=02000u&&s->words[2]<=07777u&&!agc_transfer_operand(p,s,io->superbank,s->words[2],&literal))return EXEC_OUT_OF_SCOPE;
 if(entry==LM_IB_SUPDACAL){if(!p->fixed_data||!lm_interbank_data_address(s,io,&physical)||!p->fixed_data(p,physical,&word))return EXEC_OUT_OF_SCOPE;}
 return lm_interbank(s,io,entry,literal,word)?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
#define ROUTINE(name,entry) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io){return run(p,s,io,entry);}
ROUTINE(bankcall,LM_IB_BANKCALL)
ROUTINE(swcall,LM_IB_SWCALL)
ROUTINE(swreturn,LM_IB_SWRETURN)
ROUTINE(postjump,LM_IB_POSTJUMP)
ROUTINE(bankjump,LM_IB_BANKJUMP)
ROUTINE(qjump,LM_IB_QJUMP)
ROUTINE(makecadr,LM_IB_MAKECADR)
ROUTINE(supdacal,LM_IB_SUPDACAL)
ROUTINE(ibnkcall,LM_IB_IBNKCALL)
ROUTINE(iswcalll,LM_IB_ISWCALLL)
ROUTINE(iswretrn,LM_IB_ISWRETRN)
ROUTINE(usprcadr,LM_IB_USPRCADR)
ROUTINE(supersw,LM_IB_SUPERSW)
/* SWCALL -1 (04621, BANKCALL's INCR Q): RTB's `TC SWCALL -1` enters here so
 * that SWCALL returns one word later. Q is a return address from that TC. */
static enum exec_status swcall_m1(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;
 if(!s||!io||s->words[7]!=0||s->words[2]>07776u)return EXEC_OUT_OF_SCOPE;
 s->words[2]=(agc_word)(s->words[2]+1u);s->words[5]=04622u;return EXEC_OK;}
/* Fixed-fixed bank 2: physical 2048+(Z-04000). PRIO12 (04644) is a constant. */
#define AT(z) (uint16_t)(2048u+((z)-04000u))
static const agc_routine_entry entries[14]={
 {AT(04616),bankcall},{AT(04621),swcall_m1},{AT(04622),swcall},{AT(04631),swreturn},{AT(04635),postjump},{AT(04640),bankjump},
 {AT(04643),qjump},{AT(04645),makecadr},{AT(04651),supdacal},{AT(04674),ibnkcall},{AT(04700),iswcalll},
 {AT(04707),iswretrn},{AT(04713),usprcadr},{AT(04727),supersw}};
const agc_routine_table lm_interbank_routines={entries,14};
