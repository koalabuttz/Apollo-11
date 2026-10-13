/* SPDX-License-Identifier: MIT */
#include "keyrupt_uprupt.h"
#include "agc_image_ops.h"
#include "agc_image_w.h"
#define BANKRUPT 016u
#define QRUPT 012u
#define INLINK 045u
#define LOCCTR 064u
#define RUPTREG4 073u            /* = KEYTEMP1 */
#define KEYTEMP1 073u
#define FLAGWRD5 0101u
#define FLAGWRD7 0103u
#define MPAC 0154u
#define KEYTEMP2 0734u
#define LOW5 037u
#define BIT3 04u
#define BIT4 010u                /* UPLOCBIT (UPLOCKFL) */
#define BIT10 01000u
#define BIT15 040000u            /* DSKYFBIT */
#define CHRPRIO 030000u
#define HI10 077740u
#define ELRCODE 022u

/* Where the file sits in each mission and the routines it calls. */
typedef struct {agc_word fb,base,lodsampt,novac,resume;} keyrupt_site;
static const keyrupt_site lm_site={010000u,03274u,04400u,05072u,05270u};
static const keyrupt_site cm_site={016000u,03673u,04414u,05134u,05333u};
enum {KEYRUPT1=0u,RETURN1=04u,KEYCOM=07u,ACCEPTUP=013u,NOVAC_RETURN=017u,UPRUPT=023u,RETURN2=027u,UPOK=054u,TMFAIL2=071u,UPTEST=075u};

static agc_word a15(const apollo_overlay_state *s){return agc_correct(W[0]);}
static int overflowed(agc_word a){return ((a>>15)^(a>>14))&1u;}
/* TC target from the instruction at offset `at`: Z = target, Q = at+1. */
static void tc(apollo_overlay_state *s,const keyrupt_site *k,unsigned at,agc_word target){ai_tc(s,(agc_word)(k->base+at+1u));W[5]=target;}
/* KEYRUPT1, UPRUPT: TS BANKRUPT; XCH Q; TS QRUPT; TC LODSAMPT. An
 * interrupted Q holding overflow makes TS QRUPT skip the call: returns 1
 * when the routine continues at the entry +4 without LODSAMPT. */
static int save(apollo_overlay_state *s,const keyrupt_site *k,unsigned at){
 (void)ai_ts(s,BANKRUPT);ai_xch(s,2u);if(ai_ts(s,QRUPT))return 1;
 tc(s,k,at+3u,k->lodsampt);return 0;
}
/* ACCEPTUP: CAF CHRPRIO; TC NOVAC (the 2CADR of CHARIN follows). */
static void acceptup(apollo_overlay_state *s,const keyrupt_site *k){ai_caf(s,CHRPRIO);tc(s,k,ACCEPTUP+1u,k->novac);}
/* KEYCOM: TS RUPTREG4; CS FLAGWRD5; MASK DSKYFBIT; ADS FLAGWRD5; ACCEPTUP. */
static void keycom(apollo_overlay_state *s,const keyrupt_site *k){
 (void)ai_ts(s,RUPTREG4);ai_cs(s,FLAGWRD5);ai_maskf(s,BIT15);ai_ads(s,FLAGWRD5);acceptup(s,k);
}
/* TMFAIL2: CS FLAGWRD7; MASK UPLOCBIT; ADS FLAGWRD7; TC RESUME. */
static void tmfail2(apollo_overlay_state *s,const keyrupt_site *k){ai_cs(s,FLAGWRD7);ai_maskf(s,BIT4);ai_ads(s,FLAGWRD7);tc(s,k,TMFAIL2+3u,k->resume);}
/* TC UPTEST from `at`, then UPTEST: AD KEYTEMP1; CCS A; TC TMFAIL2; HI10
 * (OCT 77740, executed as MASK 7740); TC TMFAIL2; TC Q. Returns 1 when the
 * code passed (A was -0); else the routine has ended at TMFAIL2's RESUME.
 * The MASK on the +0 path only changes A, which TMFAIL2 overwrites. */
static int uptest(apollo_overlay_state *s,const keyrupt_site *k,unsigned at){unsigned c;
 ai_tc(s,(agc_word)(k->base+at+1u));ai_ad(s,KEYTEMP1);c=ai_ccs(s,0u);
 if(c==3u){W[5]=W[2];W[2]=3u;return 1;}   /* TC Q */
 ai_tc(s,(agc_word)(k->base+(c==0u?UPTEST+2u:UPTEST+4u)+1u));tmfail2(s,k);return 0;
}
/* UPRUPT +4 to the end: the uplink word through the triple-redundancy test. */
static void uplink(apollo_overlay_state *s,const keyrupt_site *k,agc_hardware *h){
 ai_caf(s,0u);ai_xch(s,INLINK);(void)ai_ts(s,KEYTEMP1);
 ai_caf(s,BIT3);{agc_word v=(agc_word)(a15(s)|h->channel11);h->channel11=v;W[0]=agc_extend(v);}   /* EXTEND; WOR DSALMOUT */
 ai_caf(s,LOW5);ai_mask(s,KEYTEMP1);ai_xch(s,KEYTEMP1);ai_mpf(s,BIT10);(void)ai_ts(s,KEYTEMP2);
 ai_maskf(s,LOW5);ai_adf(s,HI10);if(!uptest(s,k,045u))return;
 ai_caf(s,BIT10);ai_mp(s,KEYTEMP2);ai_maskf(s,LOW5);ai_com(s);if(!uptest(s,k,053u))return;
 ai_csf(s,ELRCODE);ai_ad(s,KEYTEMP1);   /* UPOK */
 if(ai_bzf(s)){ai_csf(s,BIT4);ai_mask(s,FLAGWRD7);(void)ai_ts(s,FLAGWRD7);ai_tc(s,(agc_word)(k->base+TMFAIL2));acceptup(s,k);return;}   /* CLUPLOCK */
 ai_caf(s,BIT4);ai_mask(s,FLAGWRD7);
 if(ai_ccs(s,0u)==0u)tc(s,k,UPOK+7u,k->resume);   /* UPLOCKFL = 1 */
 else{ai_tc(s,(agc_word)(k->base+UPOK+011u));acceptup(s,k);}
}
/* KEYRUPT1 +4: CAF LOW5; EXTEND; RAND MNKEYIN; KEYCOM. */
static void keys(apollo_overlay_state *s,const keyrupt_site *k,const agc_hardware *h){
 ai_caf(s,LOW5);W[0]=agc_extend((agc_word)(a15(s)&h->channel15));keycom(s,k);
}
/* NOVAC's return: CA RUPTREG4; INDEX LOCCTR; TS MPAC; TC RESUME. */
static void to_job(apollo_overlay_state *s,const keyrupt_site *k){
 ai_ca(s,RUPTREG4);(void)ai_ts(s,MPAC+W[LOCCTR]);tc(s,k,NOVAC_RETURN+3u,k->resume);
}
static int core_set(agc_word x){return x%12u==0u&&x<=72u;}
static enum exec_status run(const agc_native_program *p,apollo_overlay_state *s,const keyrupt_site *k){unsigned at;
 if(!s||!p||!p->hardware||p->hardware->in_interrupt!=1u||W[4]!=k->fb)return EXEC_OUT_OF_SCOPE;
 at=(unsigned)(W[5]-k->base);
 switch(at){
 case KEYRUPT1:if(overflowed(W[0]))return EXEC_OUT_OF_SCOPE;if(save(s,k,at))keys(s,k,p->hardware);break;
 case UPRUPT:if(overflowed(W[0]))return EXEC_OUT_OF_SCOPE;if(save(s,k,at))uplink(s,k,p->hardware);break;
 case RETURN1:keys(s,k,p->hardware);break;
 case KEYCOM:if(overflowed(W[0]))return EXEC_OUT_OF_SCOPE;keycom(s,k);break;
 case NOVAC_RETURN:if(!core_set(W[LOCCTR]))return EXEC_OUT_OF_SCOPE;to_job(s,k);break;
 case RETURN2:uplink(s,k,p->hardware);break;
 default:return EXEC_OUT_OF_SCOPE;}
 return EXEC_OK;
}
static enum exec_status lm_run(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)io;return run(p,s,&lm_site);}
static enum exec_status cm_run(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)io;return run(p,s,&cm_site);}
#define B4(z) (uint16_t)(04u*1024u+((z)-02000u))
#define B7(z) (uint16_t)(07u*1024u+((z)-02000u))
static const agc_routine_entry lm_entries[6]={{B4(03274u),lm_run},{B4(03300u),lm_run},{B4(03303u),lm_run},{B4(03313u),lm_run},{B4(03317u),lm_run},{B4(03323u),lm_run}};
const agc_routine_table lm_keyrupt_routines={lm_entries,6};
static const agc_routine_entry cm_entries[6]={{B7(03673u),cm_run},{B7(03677u),cm_run},{B7(03702u),cm_run},{B7(03712u),cm_run},{B7(03716u),cm_run},{B7(03722u),cm_run}};
const agc_routine_table cm_keyrupt_routines={cm_entries,6};
