/* SPDX-License-Identifier: MIT */
/* Comanche055 RESTARTS_ROUTINE: RESTARTS, GETPART2, PHSPART2, ITSAVAR,
 * ITSLIKEB, ITSATBL/ITSEVEN/CONTBL2, ITSAJOB/ITSAJOB2/CHKNOVAC/ITSNOVAC,
 * ITSAWAIT/ITSWTLST/TIMETEST/ITSINDIR/FINDTIME/IMEDIATE and
 * ITSLNGCL/ITSLGCL1/ITSLGCL2/LONGCLCL, to the TC GOLOC-1 (or display FINDVAC);
 * and RESTART_TABLES' NEWPHASE/NUFAZ+10. */
#include "cm_restart_routines.h"
#include "agc_multiword.h"
#include "agc_word.h"
#include "cm_native_image.h"
#define CM_RESTART_TABLE
#include "cm_restart_table_data.inc"

#define W s->words
#define B1(z) (uint16_t)(1024u+((z)-02000u))
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define E(a) (uint16_t)(AGC_TRANSFER_ERASABLE_BASE+(a))
#define TEMPPHS 0154u
#define TEMP2G 0155u
#define POINTER 0156u
#define TEMPSWCH 0157u
#define GROUPIX 0161u
#define CYR 020u
#define SR 021u
#define TIME2 024u
#define TIME1 025u
#define TEMPG 061u
#define GOLOCM1 0704u
#define GOLOC 0705u
#define GOLOCP1 0706u
#define GOLOCP2 0707u
#define PHASE1M2 0750u   /* -PHASE1 -2 */
#define TBASE1M2 01051u  /* TBASE1 -2 */
#define TBASE1 01053u
#define PHSPRDT1 01054u
#define PHSNAME1 01436u
#define LONGBASE 01135u
#define LONGTIME 01137u
#define RTRN CM_RR_SWRETURN /* the word at RTRNCADR: TC SWRETURN */
#define PRIO14 014000u
#define SECS600_ADDRESS 02654u
#define SECS600_BANK 026000u /* BBCON STATEINT: FB 13 */
#define Q_FINDVAC 03637u
#define Q_NOVAC 03643u
#define Q_IMEDIATE 03614u
#define Q_LONGCLCL 03726u
#define Q_DISPLAY 03540u

static agc_register com(agc_register a){return (agc_register)(a^0177777u);}
static agc_word cyr(agc_word a){return (agc_word)((a>>1)|((a&1u)<<14));}
/* Erasable address under the task's EBANK (BBCON low bits). */
static agc_word phys(agc_word address,agc_word bank){return address<01400u?address:(agc_word)((bank&7u)*0400u+(address&0377u));}
/* Table word at PRDTTAB+index (01,2000+index). */
static int table(unsigned index,agc_word *word){
 if(index>=CM_RESTART_TABLE_WORDS)return 0;
 *word=cm_restart_table_words[index];return 1;
}
static enum agc_ccs_class regclass(agc_register a){
 int ov=agc_overflow(a);
 return ov>0?AGC_POSITIVE:ov<0?AGC_NEGATIVE:agc_classify(agc_correct(a));
}
static agc_register dabs(agc_register a){agc_register m=(a&0100000u)?com(a):a;return m?(agc_register)(m-1u):0;}
static agc_register plus(agc_register a,agc_word b){return agc_add_register(a,agc_extend(b));}

/* CHKNOVAC/ITSNOVAC: priority in GOLOC-1 decides FINDVAC or NOVAC. */
static void job(apollo_overlay_state *s,agc_word priority){
 if(priority==0||(priority&AGC_SIGN)!=0){W[GOLOCM1]=CM_RR_NOVAC;W[0]=com(agc_extend(priority));W[2]=Q_NOVAC;}
 else{W[GOLOCM1]=CM_RR_FINDVAC;W[0]=agc_extend(priority);W[2]=Q_FINDVAC;}
 W[5]=GOLOCM1;
}
/* FINDTIME and IMEDIATE: A holds the delta; result is the WAITLIST delay. */
static void wait_delay(apollo_overlay_state *s,uint8_t group){
 agc_register a;enum agc_ccs_class kind;
 a=com(W[0]);W[1]=a;
 a=agc_add_register(agc_extend(agc_negate(W[TBASE1+group*2u])),agc_extend(agc_negate(W[TIME1])));
 kind=regclass(a);a=dabs(a);
 if(kind==AGC_POSITIVE)a=com(a);
 if(kind==AGC_POSITIVE||kind==AGC_PLUS_ZERO)a=plus(a,037776);
 if(kind!=AGC_MINUS_ZERO)a=plus(a,1);
 a=agc_add_register(a,W[1]);kind=regclass(a);a=dabs(a);
 if(kind==AGC_POSITIVE)a=0;
 W[0]=plus(a,1);W[2]=Q_IMEDIATE;W[5]=GOLOCM1;
}
static void pairadd(apollo_overlay_state *s,agc_register_pair *r){
 agc_dp p={W[LONGTIME],W[LONGTIME+1u]};agc_double_add(r,&p);
 W[LONGTIME]=p.high;W[LONGTIME+1u]=p.low;
}
/* ITSLGCL2/LONGCLCL: time left = LONGTIME - TIME2 + LONGBASE. */
static enum cm_rr_exit long_delay(apollo_overlay_state *s){
 agc_register_pair r;agc_word high,low;enum agc_ccs_class kind;
 if(W[1]!=agc_extend(agc_correct(W[1])))return CM_RR_INVALID;
 W[LONGTIME]=agc_correct(W[0]);W[LONGTIME+1u]=agc_correct(W[1]);
 r.a=agc_extend(agc_negate(W[TIME2]));r.l=agc_extend(agc_negate(W[TIME1]));pairadd(s,&r);
 r.a=agc_extend(W[LONGBASE]);r.l=agc_extend(W[LONGBASE+1u]);pairadd(s,&r);
 high=W[LONGTIME];low=W[LONGTIME+1u];kind=agc_classify(high);
 if(kind==AGC_POSITIVE||((kind==AGC_PLUS_ZERO||kind==AGC_MINUS_ZERO)&&agc_classify(low)==AGC_POSITIVE)){
  W[GOLOCM1]=CM_RR_LONGCALL;W[0]=agc_extend(high);W[1]=agc_extend(low);
  W[2]=Q_LONGCLCL;W[5]=GOLOCM1;return CM_RR_SEL_LONGCALL;
 }
 /* Negative and signed-zero times share the immediate branches; at -0/-0
  * IMEDIATE adds one to the preceding CCS diminished zero. */
 W[0]=1;W[1]=0;W[2]=Q_IMEDIATE;W[5]=GOLOCM1;
 return CM_RR_SEL_TASK;
}
/* TIMETEST (waitlist, delta direct/indirect/immediate) or ITSLGCL1 (longcall,
 * DP delta at an address under the task's banks). */
static enum cm_rr_exit timed(apollo_overlay_state *s,uint8_t group,agc_word delta,int lng){
 agc_word bank=W[GOLOCP1];
 if(lng){
  if(delta==SECS600_ADDRESS&&(bank&076000u)==SECS600_BANK){W[0]=3;W[1]=025140;}
  else{agc_word address=phys(delta,bank);if(delta>=02000u||address>=03777u)return CM_RR_INVALID;W[0]=agc_extend(W[address]);W[1]=agc_extend(W[address+1u]);}
  W[GOLOCP1]=(agc_word)(bank&076007u);
  return long_delay(s);
 }
 if(delta==077777u){W[0]=1;W[2]=Q_IMEDIATE;W[5]=GOLOCM1;return CM_RR_SEL_TASK;}
 if((delta&AGC_SIGN)!=0){agc_word address=agc_negate(delta);if(address>=02000u)return CM_RR_INVALID;W[0]=agc_extend(W[phys(address,bank)]);W[GOLOCP1]=(agc_word)(bank&076007u);}
 else W[0]=agc_extend(delta);
 wait_delay(s,group);return CM_RR_SEL_TASK;
}

enum cm_rr_exit cm_restart_routines_select(apollo_overlay_state *s,enum cm_rr_entry entry){
 uint8_t group;agc_word phase,priority,pointer,code,bank,oldloc,oldbank;
 if(s==0||(unsigned)entry>CM_RR_TYPEB_TABLE||W[6]!=02003u||W[3]!=01400u||W[4]!=02000u)return CM_RR_INVALID;
 /* RESTARTS doubles MPAC+5 into TEMP2G; the second parts index by TEMP2G. */
 if(entry==CM_RR_GROUP){if(W[GROUPIX]>=6u)return CM_RR_INVALID;group=(uint8_t)W[GROUPIX];}
 else{if(W[TEMP2G]>=12u||(W[TEMP2G]&1u))return CM_RR_INVALID;group=(uint8_t)(W[TEMP2G]>>1);}
 phase=W[TEMPPHS];
 if(entry==CM_RR_PART2){
  /* PHSPART2: second entry of an even table phase. */
  pointer=(agc_word)(W[POINTER]+3u);
  W[POINTER]=pointer;W[GOLOCP2]=RTRN;
 }else{
  if(phase==0||phase==077777u||phase>01777u)return CM_RR_INVALID;
  if(entry==CM_RR_GROUP){W[TEMP2G]=(agc_word)(group*2u);W[TEMPSWCH]=CM_RR_PHSPART2;W[GOLOCP2]=RTRN;}
  if((phase&01400u)!=0&&entry==CM_RR_GROUP){
   /* ITSAVAR: GOLOC takes the group's stored 2CADR; A,L the old GOLOC. */
   oldloc=W[GOLOC];oldbank=W[GOLOCP1];W[GOLOC]=W[PHSNAME1+group*2u];W[GOLOCP1]=W[PHSNAME1+1u+group*2u];
   W[0]=agc_extend(oldloc);W[1]=agc_extend(oldbank);priority=W[PHSPRDT1+group*2u];
   if((phase&01000u)!=0){W[TEMPSWCH]=RTRN;W[GOLOCP2]=CM_RR_GETPART2;W[TEMPPHS]=(agc_word)(phase&0177u);job(s,priority);return CM_RR_SEL_JOB;}
   if((phase&7u)==2u){job(s,priority);return CM_RR_SEL_JOB;}
   W[GOLOCM1]=CM_RR_WAITLIST;
   return timed(s,group,(phase&7u)>=3u?agc_negate(priority):priority,(phase&7u)>=3u);
  }
  if(phase==1u){W[0]=PRIO14;W[2]=Q_DISPLAY;W[5]=CM_RR_FINDVAC;return CM_RR_SEL_DISPLAY;}
  if(!table(group*2u+(phase&1u),&code))return CM_RR_INVALID;
  W[CYR]=cyr(cyr((agc_word)(phase-2u)));
  if((phase&1u)!=0){
   W[SR]=(agc_word)(phase>>2);W[GOLOCP2]=RTRN;
   pointer=(agc_word)(code+phase+(phase>>1));
  }else{
   W[GOLOCP2]=W[TEMPSWCH];
   pointer=(agc_word)(code+3u*phase);
  }
  W[POINTER]=pointer;
 }
 if(!table(pointer,&priority)||!table(pointer+1u,&code)||!table(pointer+2u,&bank))return CM_RR_INVALID;
 oldbank=W[GOLOCP1];W[GOLOCP1]=bank;W[1]=agc_extend(oldbank);
 if((code&AGC_SIGN)==0){W[0]=agc_extend(W[GOLOC]);W[GOLOC]=code;job(s,priority);return CM_RR_SEL_JOB;}
 W[GOLOC]=agc_negate(code);W[GOLOCM1]=CM_RR_WAITLIST;
 if((bank&01000u)!=0){W[GOLOCP1]=agc_negate(bank);return timed(s,group,priority,0);}
 return timed(s,group,priority,1);
}

static enum exec_status selected(enum cm_rr_exit e){return e==CM_RR_INVALID?EXEC_OUT_OF_SCOPE:EXEC_OK;}
static enum exec_status restarts(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return selected(cm_restart_routines_select(s,CM_RR_GROUP));}
static enum exec_status getpart2(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return selected(cm_restart_routines_select(s,CM_RR_TYPEB_TABLE));}
static enum exec_status phspart2(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return selected(cm_restart_routines_select(s,CM_RR_PART2));}
/* 01,3542 TC RTRNCADR; 01,3557 TC SWRETURN. */
static enum exec_status display_return(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 W[2]=(agc_word)(CM_RR_RTRNCADR+1u);W[5]=CM_RR_SWRETURN;return EXEC_OK;
}
/* TC GOLOC-1: the service address RESTARTS stored. */
static enum exec_status goloc_call(const agc_native_program *p,apollo_overlay_state *s,void *io){agc_word w=W[GOLOCM1];(void)p;(void)io;
 if(w!=CM_RR_FINDVAC&&w!=CM_RR_NOVAC&&w!=CM_RR_WAITLIST&&w!=CM_RR_LONGCALL)return EXEC_OUT_OF_SCOPE;
 W[2]=GOLOC;W[5]=w;return EXEC_OK;
}
/* The service returns to GOLOC+2: TC SWRETURN, PHSPART2 or GETPART2. */
static enum exec_status goloc_return(const agc_native_program *p,apollo_overlay_state *s,void *io){agc_word w=W[GOLOCP2];(void)p;(void)io;
 if(w!=CM_RR_SWRETURN&&((w!=CM_RR_PHSPART2&&w!=CM_RR_GETPART2)||W[4]!=02000u))return EXEC_OUT_OF_SCOPE;
 W[2]=(agc_word)(GOLOCP2+1u);W[5]=w;return EXEC_OK;
}
/* NEWPHASE 4114: INHINT; TS L; NDX Q; CA 0 (the caller's group); INCR Q;
 * DOUBLE; TS TEMPG; CCS L. A phase of +0 or more stores directly; a
 * negative (or -0) phase first sets TBASE (NUFAZ+10: INCR A; TS L; CS TIME1;
 * NDX TEMPG; TS TBASE1 -2). Then CS L; NDX TEMPG; DXCH -PHASE1 -2; RELINT;
 * TC Q, which runs the TC held in Q (the caller's L+2), leaving Q = 3.
 * io is the cm_native_io record: RELINT leaves interrupts enabled. */
static enum exec_status newphase(const agc_native_program *p,apollo_overlay_state *s,void *io){
 agc_register a=W[0],l,ta,tl;agc_word q=W[2],group;enum agc_ccs_class kind;cm_native_io *x=io;
 /* TS L with overflow would skip NDX Q; no caller loads such an A. */
 if(x==0||x->interrupt_enabled>1u||agc_overflow(a)!=0||!agc_transfer_operand(p,s,0,q,&group)||group<1u||group>6u)return EXEC_OUT_OF_SCOPE;
 l=a;W[TEMPG]=(agc_word)(group*2u);
 kind=agc_classify(agc_correct(l));
 if(kind==AGC_NEGATIVE||kind==AGC_MINUS_ZERO){
  l=kind==AGC_NEGATIVE?(agc_register)(dabs(l)+1u):0;
  W[TBASE1M2+group*2u]=agc_negate(W[TIME1]);
 }
 a=com(l);
 ta=W[PHASE1M2+group*2u];tl=W[PHASE1M2+1u+group*2u];
 W[PHASE1M2+group*2u]=agc_correct(a);W[PHASE1M2+1u+group*2u]=agc_correct(l);
 W[0]=agc_extend((agc_word)ta);W[1]=agc_extend((agc_word)tl);
 W[2]=3;W[5]=(agc_word)(q+1u);x->interrupt_enabled=1;
 return EXEC_OK;
}
static const agc_routine_entry entries[7]={
 {B1(CM_RR_RESTARTS),restarts},{B1(CM_RR_GETPART2),getpart2},{B1(CM_RR_DISPLAY_RETURN),display_return},
 {B1(CM_RR_PHSPART2),phspart2},{FF(CM_RR_NEWPHASE),newphase},{E(GOLOCM1),goloc_call},{E(GOLOCP2),goloc_return}};
const agc_routine_table cm_restart_routines={entries,7};
const agc_site_operand cm_restart_operands[CM_RESTART_OPERAND_COUNT]={
 {CM_RESTART_INITDSP_SITE,CM_RESTART_INITDSP_0},{CM_RESTART_INITDSP_SITE+1u,CM_RESTART_INITDSP_1},
 CM_NEWPHASE_SITES_INIT};
