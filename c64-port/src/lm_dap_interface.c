/* SPDX-License-Identifier: MIT
 * Luminary099 DAP_INTERFACE_SUBROUTINES.agc, all of it (bank 20,
 * 02123-02225), as routines of the control-transfer ABI. Each routine runs
 * from its entry to the next transfer and leaves Z, Q and the banks where
 * the original continues. Fixed constants are the source's own literals. */
#include "lm_dap_interface.h"
#include "agc_image_w.h"
#define P(z) (uint16_t)(020u*1024u+((z)-02000u))
/* Fixed constants (fixed-fixed): DBSELECT 04750, DRIFTBIT 04744, PRIO27
 * 07715, EBANK6 05015, ZERO 04755; bank 20: NARROWDB 02150, WIDEDB 02151,
 * POWERDB 02152 (DEC .02222). */
#define DBSELECT 010u
#define DRIFTBIT 0200u
#define PRIO27 027000u
#define EBANK6 03000u
#define NARROWDB 00155u
#define WIDEDB 03434u
#define POWERDB 00554u
#define NOVAC 05072u
#define ACCJOB_2CADR 02135u  /* the 2CADR 1/ACCJOB after TC NOVAC */
/* CDU counters (unswitched) and the desired CDUs (EBANK6). */
#define CDUX 032u
#define CDUY 033u
#define CDUZ 034u
#define CDUXD 01635u
#define CDUYD 01636u
#define CDUZD 01637u

/* Physical erasable word for a logical erasable address under EB. */
static uint16_t ea(const apollo_overlay_state *s,agc_word a){return a<01400u?a:(uint16_t)((W[3]&03400u)+(a&0377u));}
/* Writing EB also sets the EB bits of BB. */
static void set_eb(apollo_overlay_state *s,agc_word v){W[3]=(agc_word)(v&03400u);W[6]=(agc_word)((W[6]&076000u)|(W[3]>>8));}
/* XCH EBANK: A <-> EB. LXCH EBANK: L <-> EB. */
static void xch_eb(apollo_overlay_state *s,unsigned r){agc_word eb=W[3];set_eb(s,agc_correct(W[r]));W[r]=agc_extend(eb);}
/* EXTEND; QXCH K. */
static void qxch(apollo_overlay_state *s,agc_word k){agc_word t=W[k];W[k]=agc_correct(W[2]);W[2]=agc_extend(t);}
/* A plain TC into fixed memory, as every caller's return is. */
static int tc_word(agc_register w){return w>=02000u&&w<=07777u;}
/* TC K where K holds the return word: the word runs as TC, so Z is the word
 * and Q is K+1. */
static enum exec_status tc_indirect(apollo_overlay_state *s,agc_word k){agc_register w=W[k];
 if(!tc_word(w))return EXEC_OUT_OF_SCOPE;
 W[2]=(agc_word)(k+1u);W[5]=w;return EXEC_OK;
}
/* CALLACCS: CAF PRIO27; TC NOVAC (2CADR 1/ACCJOB at Q). */
static enum exec_status callaccs(apollo_overlay_state *s){W[0]=PRIO27;W[2]=ACCJOB_2CADR;W[5]=NOVAC;return EXEC_OK;}
/* SETMAXDB +1: TS DB; EXTEND; QXCH RUPTREG1; CALLACCS. */
static enum exec_status set_db(apollo_overlay_state *s,agc_word db){W[0]=db;W[LM_DAP_DB]=db;qxch(s,LM_DAP_RUPTREG1);return callaccs(s);}
/* RESTORDB +1: MASK DBSELECT; EXTEND; BZF SETMINDB. */
static enum exec_status restordb_a(apollo_overlay_state *s){
 return set_db(s,(W[0]&DBSELECT)?WIDEDB:NARROWDB);
}
static int entry(const apollo_overlay_state *s){return s&&(W[4]&076000u)==LM_DAP_FB;}
static enum exec_status restordb(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(!entry(s))return EXEC_OUT_OF_SCOPE;
 W[0]=agc_extend(W[LM_DAP_DAPBOOLS]);return restordb_a(s);}
static enum exec_status restordb1(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(!entry(s))return EXEC_OUT_OF_SCOPE;
 return restordb_a(s);}
static enum exec_status setmaxdb(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(!entry(s))return EXEC_OUT_OF_SCOPE;
 return set_db(s,WIDEDB);}
static enum exec_status setmindb(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(!entry(s))return EXEC_OUT_OF_SCOPE;
 return set_db(s,NARROWDB);}
/* 20,2137 (NOVAC's return): TC RUPTREG1. */
static enum exec_status accreturn(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(!entry(s))return EXEC_OUT_OF_SCOPE;
 return tc_indirect(s,LM_DAP_RUPTREG1);}
/* PFLITEDB: EXTEND; QXCH RUPTREG1; TC ZATTEROR. */
static enum exec_status pflitedb(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(!entry(s))return EXEC_OUT_OF_SCOPE;
 qxch(s,LM_DAP_RUPTREG1);W[2]=02145u;W[5]=LM_DAP_ZATTEROR;return EXEC_OK;}
/* 20,2145: CAF POWERDB; TS DB; TCF CALLACCS. */
static enum exec_status pflite_return(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(!entry(s))return EXEC_OUT_OF_SCOPE;
 W[0]=POWERDB;W[LM_DAP_DB]=POWERDB;return callaccs(s);}
/* STOPRATE +3: CAF ZERO; nine TS; LXCH EBANK; TC Q. */
static enum exec_status clear_rates(apollo_overlay_state *s){/* OMEGAPD, OMEGAQD, OMEGARD, DELCDUX, DELCDUY, DELCDUZ (EBANK6) and
  * DELPEROR, DELQEROR, DELREROR (unswitched). */
 static const agc_word k[9]={01643u,01644u,01645u,01640u,01641u,01642u,01277u,01300u,01301u};unsigned i;
 if(!tc_word(W[2]))return EXEC_OUT_OF_SCOPE;
 W[0]=0;for(i=0;i<9u;++i)W[ea(s,k[i])]=0;
 xch_eb(s,1);return tc_indirect(s,2);
}
/* CAF EBANK6; XCH EBANK; TS L. */
static void save_eb(apollo_overlay_state *s){W[0]=EBANK6;xch_eb(s,0);W[1]=W[0];}
static enum exec_status zatteror(const agc_native_program *p,apollo_overlay_state *s,void *io){static const agc_word cdu[3]={CDUX,CDUY,CDUZ},desired[3]={CDUXD,CDUYD,CDUZD};unsigned i;(void)p;(void)io;
 if(!entry(s)||!tc_word(W[2]))return EXEC_OUT_OF_SCOPE;
 save_eb(s);
 /* CAE CDUX; TS CDUXD (and Y, Z). */
 for(i=0;i<3u;++i){W[0]=agc_extend(W[cdu[i]]);W[ea(s,desired[i])]=W[cdu[i]];}
 return clear_rates(s);
}
static enum exec_status stoprate(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(!entry(s)||!tc_word(W[2]))return EXEC_OUT_OF_SCOPE;
 save_eb(s);return clear_rates(s);}
/* ALLCOAST: EXTEND; QXCH RUPTREG2; TC STOPRATE. */
static enum exec_status allcoast(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(!entry(s))return EXEC_OUT_OF_SCOPE;
 qxch(s,LM_DAP_RUPTREG2);W[2]=02207u;W[5]=LM_DAP_STOPRATE;return EXEC_OK;}
/* 20,2207 (STOPRATE's return): LXCH EBANK; eight TS of A; LXCH EBANK;
 * CS DAPBOOLS; MASK DRIFTBIT; ADS DAPBOOLS; TC RESTORDB +1. */
static enum exec_status allcoast_clear(const agc_native_program *p,apollo_overlay_state *s,void *io){/* AOSQ, AOSQ+1, AOSR, AOSR+1, ALPHAQ, ALPHAR, AOSQTERM, AOSRTERM (EBANK6). */
 static const agc_word k[8]={01537u,01540u,01541u,01542u,01424u,01425u,01545u,01546u};agc_word a;unsigned i;(void)p;(void)io;
 if(!entry(s)||agc_overflow(W[0]))return EXEC_OUT_OF_SCOPE;
 xch_eb(s,1);a=agc_correct(W[0]);for(i=0;i<8u;++i)W[ea(s,k[i])]=a;xch_eb(s,1);
 W[0]=(agc_register)(~agc_extend(W[LM_DAP_DAPBOOLS])&DRIFTBIT);
 W[0]=agc_add_register(W[0],agc_extend(W[LM_DAP_DAPBOOLS]));W[LM_DAP_DAPBOOLS]=agc_correct(W[0]);
 W[2]=02225u;W[5]=02124u;return EXEC_OK;
}
/* 20,2225 (RESTORDB's return through RUPTREG1): TC RUPTREG2. */
static enum exec_status allcoast_return(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(!entry(s))return EXEC_OUT_OF_SCOPE;
 return tc_indirect(s,LM_DAP_RUPTREG2);}
static const agc_routine_entry entries[12]={
 {P(02123u),restordb},{P(02124u),restordb1},{P(02127u),setmaxdb},{P(02137u),accreturn},
 {P(02140u),setmindb},{P(02142u),pflitedb},{P(02145u),pflite_return},{P(02153u),zatteror},
 {P(02165u),stoprate},{P(02204u),allcoast},{P(02207u),allcoast_clear},{P(02225u),allcoast_return}};
const agc_routine_table lm_dap_interface_routines={entries,12};
