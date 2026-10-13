/* SPDX-License-Identifier: MIT */
/* Native Luminary099 apsides display subroutines
 * (Luminary099/P34-35_P74-75.agc): RPAD, PERIAPO1, PERIAPO, SETRAD/SETRADX,
 * LEMSTORE/ACTIVE, CSMSTORE/PASSIVE, VECSHIFT and SHIFTR1 (bank 23
 * 02314-02431, PRECSET excepted), and R30's MAXCHK with MAXNM
 * (Luminary099/R30.agc, 22,3636-3646). APSIDES and the conic steps it
 * reaches are the lm-conic-timerad milestone's (src/lm_conic_timerad.c).
 * Every interpretive pair is compiled: its original order and operand words
 * appear below only as the literal bookkeeping arguments of the
 * lm_interp_steps and lm_interp_orders paths, and each runtime branch
 * follows the outcome those paths return. RPAD and MAXNM appear as literal
 * words. */
#include "lm_periapo.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#ifdef LM_PERIAPO_TRACE
void lm_periapo_trace(const apollo_overlay_state *);
#define TRACE(s) lm_periapo_trace(s)
#else
#define TRACE(s) ((void)0)
#endif
#include "agc_image_w.h"

/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,PDVL=06557,DLOAD=06052,SLOAD=06501,PDDL=06523,GSHIFT=07621,DSU=07005,DAD=06775,
 SETPD=07613,ABVAL=03176,RVQ=03274,PUSH=03247,LXA=02354,SXA=02364,INCR=02406,CALLITA=02507,
 RTBBHIZ=02447,BZEGOTO=02467,BPLBMN=02474,SWITCH=02516};

static void next(apollo_overlay_state *s){lmi_danzig_next(s);TRACE(s);}
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
/* An address order's operand: the given word, or PUSHUP when the next word
 * (passed instead, negative) is an order pair. */
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static void gshift(apollo_overlay_state *s,agc_word word){lmi_indexed(s,word,GSHIFT);lmi_gshift(s);}
static void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
/* STOVL: the store, then the load of the given word (PUSHUP when it is an
 * order pair). */
static void stovl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dovload(s);
 if(word&040000u)lmi_diradres_pushup(s,word,VLOAD);else lmi_diradres(s,word,VLOAD);
 lmi_vload(s,0);next(s);
}
static void unary(apollo_overlay_state *s,agc_word target,void (*body)(apollo_overlay_state *)){lmi_unary(s,target);body(s);}
static void rvq(apollo_overlay_state *s){lmi_newops(s,077616);lmi_unary(s,RVQ);lmi_rvq(s);TRACE(s);}
static void right_rvq(apollo_overlay_state *s){right(s);lmi_unary(s,RVQ);lmi_rvq(s);TRACE(s);}
static void branch_taken(apollo_overlay_state *s){lmi_goto(s);TRACE(s);}
/* GOTO through an erasable CADR (GOTOERS) to its NEWOPS. */
static void go_erasable(apollo_overlay_state *s,agc_word word){
 lmi_misc(s,word,BZEGOTO);(void)ai_ccs(s,020);lmi_goto_any(s);TRACE(s);
}
/* CALL label (fixed CADR) to the callee's NEWOPS. */
static void call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);branch_taken(s);}
static uint8_t bplbmn(apollo_overlay_state *s,agc_word word){
 lmi_misc(s,word,BPLBMN);
 if(!lmi_bplbmn(s))return 0;
 branch_taken(s);return 1;
}
/* A switch order (word = its switch/operation word): 1 when it branches. */
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,io);}
/* LXA (word = the index address) as the left order of a pair. */
static void lxa(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,LXA);lmi_lxa(s);right(s);}

/* PERIAPO STQ CALL NORMEX SETRAD. */
static uint8_t head(apollo_overlay_state *s){
 lmi_newops(s,045020);lmi_misc(s,003463,CALLITA);lmi_ita(s);right(s);call(s,046344);
 return LPA_SETRAD;
}
uint8_t lpa_periapo1(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* PERIAPO1 LXA,2 VSR* RTX2 0,2; STOVL VVEC (the load pushes up);
  * LXA,1 VSR* RTX1 0,2; STORE RVEC; then PERIAPO. */
 lmi_newops(s,053754);lxa(s,002777);gshift(s,057176);next(s);
 stovl(s,026744,053750);
 lmi_newops(s,053750);lxa(s,002776);gshift(s,057176);next(s);
 store(s,002655);
 return head(s);
}
uint8_t lpa_periapo(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 return head(s);
}
uint8_t lpa_setrad(apollo_overlay_state *s,p66_bias_io *io){
 /* RPAD 2DEC 6373338 B-29 (23,2314). */
 static const agc_word RPAD[2]={000302,017755};
 (void)io;
 /* SETRAD DLOAD PUSH RPAD; SXA,1 INCR,2 X2 2D; SLOAD BHIZ X2 SETRADX. */
 lmi_newops(s,041545);opnd(s,006315,DLOAD);lmi_dload_op(s,RPAD);right(s);unary(s,PUSH,lmi_push);next(s);
 lmi_newops(s,063130);lmi_misc(s,000047,SXA);lmi_sxa(s);right(s);lmi_misc(s,000002,INCR);lmi_incr(s);next(s);
 lmi_newops(s,046135);lmi_direct(s,000050,SLOAD);lmi_sload(s);right(s);lmi_misc(s,046357,RTBBHIZ);(void)ai_ccs(s,020);
 if(lmi_bhiz(s))branch_taken(s);
 else{
  next(s);
  /* VLOAD ABVAL RLS; PDDL (pushes up). */
  lmi_newops(s,051575);opnd(s,002023,VLOAD);lmi_vload(s,0);right(s);lmi_unary(s,ABVAL);if(lmi_abvalabs(s))return LTT_ABORTED;next(s);
  lmi_newops(s,077725);opnd(s,043545,PDDL);lmi_pddl_op(s,0);next(s);
 }
 /* SETRADX DLOAD RVQ (the load pushes up). */
 lmi_newops(s,043545);opnd(s,077620,DLOAD);lmi_dload_op(s,0);right_rvq(s);
 return W[4]==046000u&&W[0164]==02331u?LPA_AFTERSETRAD:LTT_FAILED;
}
uint8_t lpa_aftersetrad(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* STCALL XXXALT APSIDES. */
 lmi_store_code(s,037671);lmi_15bitadr(s,025674,CALLITA);lmi_call(s);branch_taken(s);
 return LTR_APSIDES;
}
uint8_t lpa_afterapsides(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* SETPD PUSH 2D; DSU PDDL XXXALT 0D; PUSH DSU XXXALT; PUSH GOTO NORMEX. */
 lmi_newops(s,041401);lmi_direct(s,000003,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);unary(s,PUSH,lmi_push);next(s);
 lmi_newops(s,065225);opnd(s,003671,DSU);lmi_dsu(s,0);right(s);opnd(s,000001,PDDL);lmi_pddl_op(s,0);next(s);
 lmi_newops(s,045206);unary(s,PUSH,lmi_push);right(s);opnd(s,003671,DSU);lmi_dsu(s,0);next(s);
 lmi_newops(s,052006);unary(s,PUSH,lmi_push);right(s);go_erasable(s,003463);
 return LTT_RETURNED;
}
/* ACTIVE STOVL RACT3 VATT; STORE VACT3; RVQ. */
static uint8_t active(apollo_overlay_state *s){
 stovl(s,027536,000007);store(s,003544);rvq(s);
 return LTT_RETURNED;
}
/* PASSIVE STOVL RPASS3 VATT; STORE VPASS3; RVQ. */
static uint8_t passive(apollo_overlay_state *s){
 stovl(s,027552,000007);store(s,003560);rvq(s);
 return LTT_RETURNED;
}
uint8_t lpa_lemstore(apollo_overlay_state *s,p66_bias_io *io){
 /* LEMSTORE VLOAD BOFF RATT AVFLAG PASSIVE; ACTIVE. */
 lmi_newops(s,043175);opnd(s,000001,VLOAD);lmi_vload(s,0);right(s);
 if(sw(s,io,001352)){lmi_swbranch(s,046412);TRACE(s);return passive(s);}
 next(s);
 return active(s);
}
uint8_t lpa_csmstore(apollo_overlay_state *s,p66_bias_io *io){
 /* CSMSTORE VLOAD BOFF RATT AVFLAG ACTIVE; PASSIVE. */
 lmi_newops(s,043175);opnd(s,000001,VLOAD);lmi_vload(s,0);right(s);
 if(sw(s,io,001352)){lmi_swbranch(s,046402);TRACE(s);return active(s);}
 next(s);
 return passive(s);
}
uint8_t lpa_vecshift(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* VECSHIFT LXA,2 VSR* RTX2 0,2; LXA,1 PDVL RTX1 (PDVL pushes up);
  * VSR* PDVL 0,2 (PDVL pushes up); RVQ. */
 lmi_newops(s,053754);lxa(s,002777);gshift(s,057176);next(s);
 lmi_newops(s,063350);lxa(s,002776);opnd(s,063257,PDVL);lmi_pdvl(s,0);next(s);
 lmi_newops(s,063257);gshift(s,057176);right(s);opnd(s,077616,PDVL);lmi_pdvl(s,0);next(s);
 rvq(s);
 return LTT_RETURNED;
}
uint8_t lpa_shiftr1(apollo_overlay_state *s,p66_bias_io *io){
 (void)io;
 /* SHIFTR1 LXA,2 SL* RTX2 0,2; RVQ. */
 lmi_newops(s,053754);lxa(s,002777);gshift(s,057576);next(s);
 rvq(s);
 return LTT_RETURNED;
}
uint8_t lpa_maxchk(apollo_overlay_state *s,p66_bias_io *io){
 /* MAXNM 2OCT 0106505603 (22,3645). */
 static const agc_word MAXNM[2]={001065,005603};
 (void)io;
 /* MAXCHK DSU BPL MAXNM +3; DAD RVQ MAXNM; +3 DLOAD RVQ MAXNM. */
 lmi_newops(s,051025);opnd(s,005646,DSU);lmi_dsu(s,MAXNM);right(s);
 if(bplbmn(s,045643)){
  lmi_newops(s,043545);opnd(s,005646,DLOAD);lmi_dload_op(s,MAXNM);right_rvq(s);
  return LTT_RETURNED;
 }
 next(s);
 lmi_newops(s,043415);opnd(s,005646,DAD);lmi_dad(s,MAXNM);right_rvq(s);
 return LTT_RETURNED;
}

uint8_t lpa_remap(const apollo_overlay_state *s,uint8_t next){
 if(next==LTT_RETURNED&&W[4]==046000u&&W[0164]==02333u)return LPA_AFTERAPSIDES;
 return next;
}
uint8_t lm_periapo_step(apollo_overlay_state *s,p66_bias_io *io,uint8_t phase){
 switch(phase){
 case LPA_PERIAPO1:return lpa_periapo1(s,io);
 case LPA_PERIAPO:return lpa_periapo(s,io);
 case LPA_SETRAD:return lpa_setrad(s,io);
 case LPA_AFTERSETRAD:return lpa_aftersetrad(s,io);
 case LPA_AFTERAPSIDES:return lpa_afterapsides(s,io);
 case LPA_LEMSTORE:return lpa_lemstore(s,io);
 case LPA_CSMSTORE:return lpa_csmstore(s,io);
 case LPA_VECSHIFT:return lpa_vecshift(s,io);
 case LPA_SHIFTR1:return lpa_shiftr1(s,io);
 case LPA_MAXCHK:return lpa_maxchk(s,io);
 default:
  if(LM_PERIAPO_DONE(phase)||phase>LTR_INFINAPO)return LTT_FAILED;
  return lpa_remap(s,lm_conic_timerad_step(s,io,phase));
 }
}

/* The actual entries: NEWOPS at the routine after one of its Luminary
 * callers' CALL or STCALL, QPRET that caller's return (P30 S30.1, P31,
 * P32-P35 CSI/CDH and P34 TPI, STABLE_ORBIT, R30 and PRECSET). */
static const struct {uint16_t fbank,loc;uint8_t phase,mode;uint8_t count;agc_word q[11];} entries[]={
 {046000u,02316u,LPA_PERIAPO1,1,5,{070043,070444,071473,072517,072706}},
 {046000u,02326u,LPA_PERIAPO,2,1,{070254}},
 {046000u,02376u,LPA_LEMSTORE,2,1,{046366}},
 {046000u,02406u,LPA_CSMSTORE,2,1,{046374}},
 {046000u,02416u,LPA_VECSHIFT,1,3,{070241,070356,071164}},
 {046000u,02426u,LPA_SHIFTR1,0,11,{070045,070053,070256,070273,070323,070446,071215,071475,072521,072527,072710}},
 {044000u,03636u,LPA_MAXCHK,0,6,{045600,045612,070047,070055,072523,072531}},
};
/* Canonical image with coherent banks, VAC, PUSHLOC within the VAC, MODE
 * vector (1) or double precision (0) where the routine starts on MPAC, X1
 * -2 or -10 (RTX1) where APSIDES follows, and RTX1/RTX2 the Earth or Moon
 * pair where the routine loads them. */
uint8_t lm_periapo_entry(const apollo_overlay_state *s,const p66_bias_io *io){
 unsigned i,j,vac=W[0120],pd=W[0166];agc_word q,x1=W[vac+046u],rtx1=W[02776],rtx2=W[02777];
 if(W[5]!=06070u||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6]||W[0115]!=(W[4]&040000u)||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u))return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u||pd<vac||pd>vac+040u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 q=W[vac+052u];
 for(i=0;i<sizeof entries/sizeof entries[0];++i){
  if(W[4]!=entries[i].fbank||W[0164]!=entries[i].loc)continue;
  for(j=0;j<entries[i].count&&entries[i].q[j]!=q;++j){}
  if(j==entries[i].count)return 0;
  if(entries[i].mode==1&&W[0163]!=077776u)return 0;
  if(entries[i].mode==0&&W[0163]!=0u)return 0;
  switch(entries[i].phase){
  case LPA_PERIAPO1:case LPA_VECSHIFT:
   if(!((rtx1==077775u&&rtx2==0u)||(rtx1==077765u&&rtx2==2u))||pd<vac+6u)return 0;
   break;
  case LPA_PERIAPO:
   if(x1!=077775u&&x1!=077765u)return 0;
   break;
  case LPA_SHIFTR1:
   if(rtx2!=0u&&rtx2!=2u)return 0;
   break;
  default:break;
  }
  return entries[i].phase;
 }
 return 0;
}
uint8_t lm_periapo(apollo_overlay_state *s,p66_bias_io *io){
 uint8_t phase;unsigned guard;
 if(!s||!io||!(phase=lm_periapo_entry(s,io)))return 0;
 /* APSIDES runs about twenty steps; the guard only stops a runaway. */
 for(guard=0;guard<200u&&!LM_PERIAPO_DONE(phase);++guard)phase=lm_periapo_step(s,io,phase);
 return phase==LTT_RETURNED?1u:phase==LTT_ABORTED?2u:phase==LTT_CONICABORT?3u:0u;
}
