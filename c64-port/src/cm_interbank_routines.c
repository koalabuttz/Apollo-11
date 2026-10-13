/* SPDX-License-Identifier: MIT */
#include "cm_interbank_routines.h"
#include "cm_interbank_sites.inc"
const agc_site_operand *cm_interbank_site_table(uint16_t *count){if(count)*count=CM_INTERBANK_SITES_COUNT;return cm_interbank_sites;}
static enum exec_status run(const agc_native_program *p,apollo_overlay_state *s,void *v,enum cm_interbank_complete_entry entry){cm_native_io *io=v;agc_word literal=0,word=0;uint16_t physical;
 if(!p||!s||!io)return EXEC_OUT_OF_SCOPE;
 /* Fixed inline CADR: the caller's compiled operand. Erasable ones are read by cm_interbank_complete after its own stores. */
 if((entry==CM_IC_BANKCALL||entry==CM_IC_IBNKCALL||entry==CM_IC_POSTJUMP||entry==CM_IC_USPRCADR)&&s->words[2]>=02000u&&s->words[2]<=07777u&&!agc_transfer_operand(p,s,io->superbank,s->words[2],&literal))return EXEC_OUT_OF_SCOPE;
 if(entry==CM_IC_SUPDACAL){if(!p->fixed_data||!cm_interbank_complete_data_address(s,io,&physical)||!p->fixed_data(p,physical,&word))return EXEC_OUT_OF_SCOPE;}
 return cm_interbank_complete(s,io,entry,literal,word)?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
#define ROUTINE(name,entry) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io){return run(p,s,io,entry);}
ROUTINE(bankjump,CM_IC_BANKJUMP)
ROUTINE(qjump,CM_IC_QJUMP)
ROUTINE(makecadr,CM_IC_MAKECADR)
ROUTINE(supdacal,CM_IC_SUPDACAL)
ROUTINE(ibnkcall,CM_IC_IBNKCALL)
ROUTINE(iswcall,CM_IC_ISWCALL)
ROUTINE(iswretrn,CM_IC_ISWRETRN)
ROUTINE(usprcadr,CM_IC_USPRCADR)
ROUTINE(supersw,CM_IC_SUPERSW)
/* Fixed-fixed bank 2: physical 2048+(Z-04000). PRIO12 (04710) is a constant. */
#define AT(z) (uint16_t)(2048u+((z)-04000u))
static const agc_routine_entry entries[14]={
 {AT(04662),cm_ib_bankcall},{AT(04665),cm_ib_swcall_m1},{AT(04666),cm_ib_swcall},{AT(04675),cm_ib_swreturn},{AT(04701),cm_ib_postjump},{AT(04704),bankjump},
 {AT(04707),qjump},{AT(04711),makecadr},{AT(04715),supdacal},{AT(04740),ibnkcall},{AT(04744),iswcall},
 {AT(04753),iswretrn},{AT(04757),usprcadr},{AT(04773),supersw}};
const agc_routine_table cm_interbank_routines={entries,14};
