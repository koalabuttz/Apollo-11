/* SPDX-License-Identifier: MIT */
#include "lm_exec_routines.h"
#include "lm_admission_image.h"
#include "lm_waitlist_image.h"
#include "agc_multiword.h"
#include "lm_exec_sites.inc"
#define W s->words
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define B1(z) (uint16_t)(1024u+((z)-02000u))
#define LONGCADR LM_EXEC_LONGCADR
#define LONGTIME LM_EXEC_LONGTIME
#define LONGEXIT LM_EXEC_LONGEXIT
#define BB_LST1 02003u  /* BBCON LNGCALL2/LONGCYCL/GETCADR: bank 01, EBANK LST1 */

#ifdef AGC_SITES_ROPE
/* Addresses only (agc_transfer.h AGC_SITES_ROPE). */
const agc_site_operand *lm_exec_site_table(uint16_t *count)
{
    if (count) *count = (uint16_t)(LM_EXEC_SITES_COUNT | AGC_SITES_ADDRESSES);
    return (const agc_site_operand *)(const void *)lm_exec_sites_physical;
}
/* Boot-only (rk_boot): the sum its rope check expects. */
__attribute__((section(".boot.text"))) uint16_t lm_exec_site_sum(void)
{
    return LM_EXEC_SITES_WORD_SUM;
}
#else
const agc_site_operand *lm_exec_site_table(uint16_t *count){if(count)*count=LM_EXEC_SITES_COUNT;return lm_exec_sites;}
#endif

/* The inline 2CADR (or ADRES) at Q. */
static int operands(const agc_native_program *p,const apollo_overlay_state *s,const p66_bias_io *io,unsigned words,exec_address *a){
 agc_word q=W[2];a->bank=0;
 if(!p||!io||q>07775u)return 0;
 return agc_transfer_operand(p,s,io->superbank,q,&a->location)&&(words<2u||agc_transfer_operand(p,s,io->superbank,(agc_word)(q+1u),&a->bank));
}
static enum exec_status admit(const agc_native_program *p,apollo_overlay_state *s,void *v,enum lm_admission_entry entry){p66_bias_io *io=v;exec_address job;enum exec_status st;
 if(!s||!operands(p,s,io,2,&job))return EXEC_OUT_OF_SCOPE;
 st=lm_admission_image(s,io,entry,job);
 /* No VAC area or core set: the image is at BAILOUT1 with the source code. */
 return st==EXEC_OK||st==EXEC_NO_VAC||st==EXEC_NO_CORE?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static enum exec_status schedule(const agc_native_program *p,apollo_overlay_state *s,void *v,enum lm_waitlist_entry entry){p66_bias_io *io=v;exec_address task;enum wl_status st;
 if(!s||!operands(p,s,io,entry==LM_TWIDDLE_ENTRY?1u:2u,&task))return EXEC_OUT_OF_SCOPE;
 st=lm_waitlist_image(s,io,entry,task);
 return st==WL_OK||st==WL_FULL||st==WL_BAD_DELAY?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static enum exec_status novac(const agc_native_program *p,apollo_overlay_state *s,void *io){return admit(p,s,io,LM_ADMIT_NOVAC);}
static enum exec_status findvac(const agc_native_program *p,apollo_overlay_state *s,void *io){return admit(p,s,io,LM_ADMIT_FINDVAC);}
static enum exec_status waitlist_call(const agc_native_program *p,apollo_overlay_state *s,void *io){return schedule(p,s,io,LM_WAITLIST_ENTRY);}
static enum exec_status twiddle(const agc_native_program *p,apollo_overlay_state *s,void *io){return schedule(p,s,io,LM_TWIDDLE_ENTRY);}

static void banks(apollo_overlay_state *s,agc_word bb){W[6]=(agc_word)(bb&076007u);W[3]=(agc_word)((bb&7u)<<8);W[4]=(agc_word)(bb&076000u);}
/* DXCH K: A,L <-> K,K+1. */
static void dxch(apollo_overlay_state *s,uint16_t k){agc_word a=agc_correct(W[0]),l=agc_correct(W[1]);
 W[0]=agc_extend(W[k]);W[1]=agc_extend(W[k+1u]);W[k]=a;W[k+1u]=l;
}
/* DTCB (DXCH Z) executed at `at`: Z,BB <-> A,L. */
static void dtcb(apollo_overlay_state *s,agc_word at){agc_word z=agc_correct(W[0]),bb=agc_correct(W[1]);
 W[0]=(agc_word)(at+1u);W[1]=agc_extend(W[6]);W[5]=z;banks(s,bb);
}
static int address(agc_word z){return z!=0u&&z<=07777u;}

/* 05277 LONGCALL: DXCH LONGTIME; EXTEND; NDX Q; DCA 0; DXCH LONGCADR;
 * EXTEND; DCA LGCL2CDR; DTCB. */
static enum exec_status longcall(const agc_native_program *p,apollo_overlay_state *s,void *v){p66_bias_io *io=v;exec_address task;
 if(!s||!operands(p,s,io,2,&task))return EXEC_OUT_OF_SCOPE;
 dxch(s,LONGTIME);
 W[0]=agc_extend(task.location);W[1]=agc_extend(task.bank);dxch(s,LONGCADR);
 W[0]=LM_EXEC_LNGCALL2;W[1]=02063u;dtcb(s,05306u);
 return EXEC_OK;
}
/* 01,3452 LNGCALL2: LXCH LONGEXIT+1; CA TWO; ADS Q; TS LONGEXIT; then the
 * delta-time check, to LONGCYCL or LONGPOOH (DXCH LONGEXIT; TC POODOO1). */
static enum exec_status lngcall2(const agc_native_program *p,apollo_overlay_state *s,void *io){agc_word t,high,low;enum agc_ccs_class k;(void)p;(void)io;
 if(!s||W[6]!=BB_LST1||W[2]>07775u)return EXEC_OUT_OF_SCOPE;
 t=W[LONGEXIT+1u];W[LONGEXIT+1u]=agc_correct(W[1]);W[1]=agc_extend(t);
 W[2]=W[0]=(agc_word)(W[2]+2u);W[LONGEXIT]=W[2];
 high=W[LONGTIME];low=W[LONGTIME+1u];k=agc_classify(high);
 /* CCS A: positive or negative leave |A|-1; either zero reads the low word. */
 if(k==AGC_POSITIVE){W[0]=(agc_word)(high-1u);W[5]=LM_EXEC_LONGCYCL;return EXEC_OK;}
 if(k==AGC_NEGATIVE)W[0]=(agc_word)(agc_negate(high)-1u);
 else{W[0]=agc_extend(low);k=agc_classify(low);
  if(k==AGC_POSITIVE){W[5]=LM_EXEC_LONGCYCL;return EXEC_OK;}}
 /* LONGPOOH */
 dxch(s,LONGEXIT);W[2]=03525u;W[5]=LM_EXEC_POODOO1;
 return EXEC_OK;
}
/* 01,3466 LONGCYCL: EXTEND; DCS DPBIT14; DAS LONGTIME; CCS LONGTIME+1;
 * CCS LONGTIME; then MUCHTIME (CA BIT14; TC WAITLIST; 2CADR LONGCYCL) or
 * LASTTIME (CA BIT14; ADS LONGTIME+1; TC WAITLIST; 2CADR GETCADR). */
static enum exec_status longcycl(const agc_native_program *p,apollo_overlay_state *s,void *io){agc_register_pair r={0177777u,0157777u};agc_dp t;enum agc_ccs_class k;int much;(void)p;(void)io;
 if(!s||W[6]!=BB_LST1)return EXEC_OUT_OF_SCOPE;
 t.high=W[LONGTIME];t.low=W[LONGTIME+1u];agc_double_add(&r,&t);
 k=agc_classify(t.low);much=k==AGC_POSITIVE;
 if(!much){k=agc_classify(t.high);
  if(k==AGC_PLUS_ZERO||k==AGC_NEGATIVE)return EXEC_OUT_OF_SCOPE; /* would execute DPBIT14 */
  much=k==AGC_POSITIVE;}
 W[LONGTIME]=t.high;W[LONGTIME+1u]=t.low;W[1]=0;
 if(much){W[0]=020000u;W[2]=03513u;}
 else{W[0]=agc_add_register(020000u,agc_extend(t.low));W[LONGTIME+1u]=agc_correct(W[0]);W[2]=03504u;}
 W[5]=LM_EXEC_WAITLIST;
 return EXEC_OK;
}
/* 01,3506 LONGRTRN (also reached by TCF LONGRTRN at 01,3515): CA TSKOVCDR;
 * DXCH LONGEXIT; DTCB. */
static enum exec_status longrtrn(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(!s||W[6]!=BB_LST1||!address(W[LONGEXIT]))return EXEC_OUT_OF_SCOPE;
 W[0]=LM_EXEC_TASKOVER;dxch(s,LONGEXIT);dtcb(s,03510u);
 return EXEC_OK;
}
/* 01,3516 GETCADR: DXCH LONGCADR; DTCB. */
static enum exec_status getcadr(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(!s||W[6]!=BB_LST1||!address(W[LONGCADR]))return EXEC_OUT_OF_SCOPE;
 dxch(s,LONGCADR);dtcb(s,03517u);
 return EXEC_OK;
}
static const agc_routine_entry entries[10]={
 {B1(LM_EXEC_LNGCALL2),lngcall2},{B1(LM_EXEC_LONGCYCL),longcycl},{B1(LM_EXEC_LONGRTRN),longrtrn},
 {B1(LM_EXEC_MUCHRTRN),longrtrn},{B1(LM_EXEC_GETCADR),getcadr},
 {FF(LM_EXEC_NOVAC),novac},{FF(LM_EXEC_FINDVAC),findvac},{FF(LM_EXEC_TWIDDLE),twiddle},
 {FF(LM_EXEC_WAITLIST),waitlist_call},{FF(LM_EXEC_LONGCALL),longcall}};
const agc_routine_table lm_exec_routines={entries,10};
