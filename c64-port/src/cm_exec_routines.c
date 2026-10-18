/* SPDX-License-Identifier: MIT */
#include "cm_exec_routines.h"
#include "cm_admission_zero.h"
#include "cm_waitlist_extended.h"
#include "cm_waitlist_internal.h"
#include "cm_task_transfer.h"
#include "agc_multiword.h"
#include "cm_exec_sites.inc"
#define W s->words
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define B1(z) (uint16_t)(1024u+((z)-02000u))
#define LONGCADR CM_EXEC_LONGCADR
#define LONGTIME CM_EXEC_LONGTIME
#define LONGEXIT CM_EXEC_LONGEXIT
#define BB_LST1 02003u  /* BBCON LNGCALL2/LONGCYCL/GETCADR: bank 01, EBANK LST1 */

#ifdef AGC_SITES_ROPE
/* Addresses only (agc_transfer.h AGC_SITES_ROPE). */
const agc_site_operand *cm_exec_site_table(uint16_t *count)
{
    if (count) *count = (uint16_t)(CM_EXEC_SITES_COUNT | AGC_SITES_ADDRESSES | AGC_SITES_CM_ROPE);
    return (const agc_site_operand *)(const void *)cm_exec_sites_physical;
}
/* Boot-only (rk_boot): the sum its rope check expects. */
__attribute__((section(".boot.text"))) uint16_t cm_exec_site_sum(void)
{
    return CM_EXEC_SITES_WORD_SUM;
}
#else
const agc_site_operand *cm_exec_site_table(uint16_t *count){if(count)*count=CM_EXEC_SITES_COUNT;return cm_exec_sites;}
#endif

/* The inline 2CADR (or ADRES) at Q. */
static int operands(const agc_native_program *p,const apollo_overlay_state *s,const cm_native_io *io,unsigned words,exec_address *a){
 agc_word q=W[2];a->bank=0;
 if(!p||!io||q>07775u)return 0;
 return agc_transfer_operand(p,s,io->superbank,q,&a->location)&&(words<2u||agc_transfer_operand(p,s,io->superbank,(agc_word)(q+1u),&a->bank));
}
static enum exec_status admit(const agc_native_program *p,apollo_overlay_state *s,void *v,enum cm_admission_entry entry){cm_native_io *io=v;exec_address job;enum exec_status st;
 if(!s||!operands(p,s,io,2,&job))return EXEC_OUT_OF_SCOPE;
 st=cm_admission_zero(s,io,entry,job);
 /* No VAC area or core set: the image is at BAILOUT with the source code. */
 return st==EXEC_OK||st==EXEC_NO_VAC||st==EXEC_NO_CORE?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static enum exec_status schedule(const agc_native_program *p,apollo_overlay_state *s,void *v,uint8_t twiddle){cm_native_io *io=v;exec_address task;enum cm_wl_status st;
 if(!s||!operands(p,s,io,twiddle?1u:2u,&task))return EXEC_OUT_OF_SCOPE;
 st=cm_waitlist_extended(s,io,task,twiddle);
 return st==CM_WL_OK||st==CM_WL_FULL||st==CM_WL_BAD_DELAY?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
static enum exec_status novac(const agc_native_program *p,apollo_overlay_state *s,void *io){return admit(p,s,io,CM_ADMIT_NOVAC);}
static enum exec_status findvac(const agc_native_program *p,apollo_overlay_state *s,void *io){return admit(p,s,io,CM_ADMIT_FINDVAC);}
static enum exec_status waitlist_call(const agc_native_program *p,apollo_overlay_state *s,void *io){return schedule(p,s,io,0);}
static enum exec_status twiddle(const agc_native_program *p,apollo_overlay_state *s,void *io){return schedule(p,s,io,1);}

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

/* 05342 LONGCALL: DXCH LONGTIME; EXTEND; NDX Q; DCA 0; DXCH LONGCADR;
 * EXTEND; DCA LGCL2CDR; DTCB. */
static enum exec_status longcall(const agc_native_program *p,apollo_overlay_state *s,void *v){cm_native_io *io=v;exec_address task;
 if(!s||!operands(p,s,io,2,&task))return EXEC_OUT_OF_SCOPE;
 dxch(s,LONGTIME);
 W[0]=agc_extend(task.location);W[1]=agc_extend(task.bank);dxch(s,LONGCADR);
 W[0]=CM_EXEC_LNGCALL2;W[1]=02063u;dtcb(s,05351u);
 return EXEC_OK;
}
/* 01,3461 LNGCALL2: LXCH LONGEXIT+1; CA TWO; ADS Q; TS LONGEXIT; on into
 * LONGCYCL. */
static enum exec_status lngcall2(const agc_native_program *p,apollo_overlay_state *s,void *io){agc_word t;(void)p;(void)io;
 if(!s||W[6]!=BB_LST1||W[2]>07775u)return EXEC_OUT_OF_SCOPE;
 t=W[LONGEXIT+1u];W[LONGEXIT+1u]=agc_correct(W[1]);W[1]=agc_extend(t);
 W[2]=W[0]=(agc_word)(W[2]+2u);W[LONGEXIT]=W[2];W[5]=CM_EXEC_LONGCYCL;
 return EXEC_OK;
}
/* 01,3465 LONGCYCL: EXTEND; DCS DPBIT14; DAS LONGTIME; CCS LONGTIME+1;
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
 if(much){W[0]=020000u;W[2]=03512u;}
 else{W[0]=agc_add_register(020000u,agc_extend(t.low));W[LONGTIME+1u]=agc_correct(W[0]);W[2]=03503u;}
 W[5]=CM_EXEC_WAITLIST;
 return EXEC_OK;
}
/* 01,3505 LONGRTRN (also reached by TCF LONGRTRN at 01,3514): CA TSKOVCDR;
 * DXCH LONGEXIT; DTCB. */
static enum exec_status longrtrn(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(!s||W[6]!=BB_LST1||!address(W[LONGEXIT]))return EXEC_OUT_OF_SCOPE;
 W[0]=CM_EXEC_TASKOVER;dxch(s,LONGEXIT);dtcb(s,03507u);
 return EXEC_OK;
}
/* 01,3515 GETCADR: DXCH LONGCADR; DTCB. */
static enum exec_status getcadr(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(!s||W[6]!=BB_LST1||!address(W[LONGCADR]))return EXEC_OUT_OF_SCOPE;
 dxch(s,LONGCADR);dtcb(s,03516u);
 return EXEC_OK;
}

/* FIXDELAY (the delay is the DEC at Q) and VARDELAY (delay in A): the calling
 * task's remainder goes on the Waitlist and control reaches TASKOVER, or
 * BAILOUT/POODOO with 1203/1204. */
/* The WTLST5 path whose last CCS sees +0 executes the erasable word 01203
 * (TC 1203, CM_WL_SOURCE_OVERFLOW): refused, with the words that path wrote
 * (A..BB and WAITEXIT/WAITADR/WAITTEMP 061-063) and io restored. Every other
 * refusal of cm_waitlist_internal writes nothing. */
static const uint8_t frontier[10]={0,1,2,3,4,5,6,061,062,063};
static enum exec_status delay(const agc_native_program *p,apollo_overlay_state *s,void *v,enum cm_waitlist_internal_entry entry){cm_native_io *io=v,io0;agc_word saved[10],d=0;uint8_t i;enum cm_wl_status st;
 if(!p||!s||!io)return EXEC_OUT_OF_SCOPE;
 if(entry==CM_WL_FIXDELAY&&(W[2]>07776u||!agc_transfer_operand(p,s,io->superbank,W[2],&d)))return EXEC_OUT_OF_SCOPE;
 for(i=0;i<10u;++i)saved[i]=W[frontier[i]];
 io0=*io;st=cm_waitlist_internal(s,io,entry,d);
 if(st==CM_WL_OK||st==CM_WL_FULL||st==CM_WL_BAD_DELAY)return EXEC_OK;
 for(i=0;i<10u;++i)W[frontier[i]]=saved[i];
 *io=io0;return EXEC_OUT_OF_SCOPE;
}
static enum exec_status fixdelay(const agc_native_program *p,apollo_overlay_state *s,void *v){return delay(p,s,v,CM_WL_FIXDELAY);}
static enum exec_status vardelay(const agc_native_program *p,apollo_overlay_state *s,void *v){return delay(p,s,v,CM_WL_VARDELAY);}
/* 04014 T3RUPT vector, entered by the T3 interrupt: DXCH ARUPT; CAF T3RPTBB;
 * XCH BBANK; TCF T3RUPT; EXTEND; ROR SUPERBNK; TS BANKRUPT; EXTEND;
 * QXCH QRUPT; then T3RUPT2 shifts the lists and DTCBs to the next task. */
static const uint8_t leadin[10]={0,1,2,3,4,6,010,011,012,016};
/* body: entered at T3RUPT 01,3416 itself (the vector's TCF T3RUPT), with
 * BB = T3RPTBB and A the interrupted BB. */
static enum exec_status t3(const agc_native_program *p,apollo_overlay_state *s,cm_native_io *io,bool body){agc_word saved[10],old_bb;uint8_t i;
 if(!p||!p->hardware||!s||!io||p->hardware->in_interrupt!=1u||agc_overflow(W[0]))return EXEC_OUT_OF_SCOPE;
 if(body&&(W[7]||W[6]!=BB_LST1||(agc_correct(W[0])&(agc_word)~076007u)))return EXEC_OUT_OF_SCOPE;
 for(i=0;i<10u;++i)saved[i]=W[leadin[i]];
 if(body)old_bb=agc_correct(W[0]);else{dxch(s,010);old_bb=W[6];banks(s,BB_LST1);}
 W[0]=(agc_word)(old_bb|io->superbank);W[016]=W[0];
 /* QXCH QRUPT: Q may hold data, not an address (TPAGREE leaves POSMAX, #143). */
 {agc_word q=W[2];W[2]=agc_extend(W[012]);W[012]=agc_correct(q);}
 /* A refused T3RUPT2 writes nothing itself. */
 if(cm_task_transfer(s,io,CM_TASK_DISPATCH)!=CM_TASK_SELECTED){for(i=0;i<10u;++i)W[leadin[i]]=saved[i];return EXEC_OUT_OF_SCOPE;}
 return EXEC_OK;
}
static enum exec_status t3rupt(const agc_native_program *p,apollo_overlay_state *s,void *v){return t3(p,s,v,false);}
static enum exec_status t3rupt_body(const agc_native_program *p,apollo_overlay_state *s,void *v){return t3(p,s,v,true);}
/* DLY2 -1 05255, from DELAYJOB's TCF DLY2 -1: TS WAITADR, WAIT2's insertion,
 * back to WAITEXIT +2 (TCGETCAD) or BAILOUT. */
static enum exec_status dly2_m1(const agc_native_program *p,apollo_overlay_state *s,void *v){return delay(p,s,v,CM_WL_DLY2_MINUS1);}
/* 05324 TASKOVER: RUPTAGN +1 dispatches the next task that fell due; -0
 * resumes the interrupted program (RESUME: Z = ZRUPT-1, the instruction
 * saved in BRUPT next, and the interrupt ends). */
static enum exec_status taskover(const agc_native_program *p,apollo_overlay_state *s,void *v){cm_native_io *io=v;agc_word eb,fb,bb,super;enum cm_task_transfer_exit x;
 if(!p||!p->hardware||!s||!io||p->hardware->in_interrupt!=1u)return EXEC_OUT_OF_SCOPE;
 if(W[0734]==AGC_NEG_ZERO&&(W[015]==0u||W[015]>010000u))return EXEC_OUT_OF_SCOPE;
 /* RESUME: BRUPT is the word at ZRUPT-1 under BANKRUPT's banks and superbank (#144). */
 if(W[0734]==AGC_NEG_ZERO&&!agc_resume_exact(p,s,(agc_word)(W[016]&076000u),(agc_word)((W[016]&7u)<<8),(agc_word)(W[016]&0160u)))return EXEC_OUT_OF_SCOPE;
 /* A refused dispatch has only switched to the Waitlist bank. */
 eb=W[3];fb=W[4];bb=W[6];super=io->superbank;x=cm_task_transfer(s,io,CM_TASK_FINISH);
 if(x==CM_TASK_SELECTED)return EXEC_OK;
 if(x!=CM_TASK_RESUME){W[3]=eb;W[4]=fb;W[6]=bb;io->superbank=super;return EXEC_OUT_OF_SCOPE;}
 W[5]=(agc_word)(W[015]-1u);W[7]=0;p->hardware->in_interrupt=0;
 return EXEC_OK;
}
static const agc_routine_entry entries[16]={
 {B1(03416u),t3rupt_body},{B1(CM_EXEC_LNGCALL2),lngcall2},{B1(CM_EXEC_LONGCYCL),longcycl},{B1(CM_EXEC_LONGRTRN),longrtrn},
 {B1(CM_EXEC_MUCHRTRN),longrtrn},{B1(CM_EXEC_GETCADR),getcadr},
 {FF(CM_EXEC_T3RUPT_VECTOR),t3rupt},{FF(CM_EXEC_NOVAC),novac},{FF(CM_EXEC_FINDVAC),findvac},{FF(CM_EXEC_TWIDDLE),twiddle},
 {FF(CM_EXEC_WAITLIST),waitlist_call},{FF(05255u),dly2_m1},{FF(CM_EXEC_FIXDELAY),fixdelay},{FF(CM_EXEC_VARDELAY),vardelay},
 {FF(CM_EXEC_TASKOVER),taskover},{FF(CM_EXEC_LONGCALL),longcall}};
const agc_routine_table cm_exec_routines={entries,16};
#if defined(CM_EXEC_DLY2_ONLY)
/* For a C64 program that only enters DLY2 -1 (waitlist-entries kind 3): the
 * same entry alone, so the rest of the table is not linked. */
static const agc_routine_entry dly2_only[1]={{FF(05255u),dly2_m1}};
const agc_routine_table cm_exec_dly2_routines={dly2_only,1};
#endif
