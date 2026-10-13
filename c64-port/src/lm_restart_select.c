/* SPDX-License-Identifier: MIT */
/* Luminary099 RESTARTS_ROUTINE: RESTARTS, GETPART2, PHSPART2, ITSAVAR,
 * ITSLIKEB, ITSATBL/ITSEVEN/CONTBL2, ITSAJOB/ITSAJOB2/CHKNOVAC/ITSNOVAC,
 * ITSAWAIT/ITSWTLST/TIMETEST/ITSINDIR/FINDTIME/IMEDIATE and
 * ITSLNGCL/ITSLGCL1/ITSLGCL2/LONGCLCL, to the TC GOLOC-1 (or display FINDVAC). */
#include "lm_restart_select.h"
#include "agc_multiword.h"
#include "agc_word.h"
#include "lm_restart_table_data.inc"

#define W s->words
#define TEMPPHS 0154u
#define TEMP2G 0155u
#define POINTER 0156u
#define TEMPSWCH 0157u
#define GROUPIX 0161u
#define CYR 020u
#define SR 021u
#define TIME2 024u
#define TIME1 025u
#define GOLOCM1 0704u
#define GOLOC 0705u
#define GOLOCP1 0706u
#define GOLOCP2 0707u
#define TBASE1 01053u
#define PHSPRDT1 01054u
#define PHSNAME1 01436u
#define LONGBASE 01151u
#define LONGTIME 01153u
#define RTRN 04631u      /* the word at RTRNCADR: TC SWRETURN */
#define PRIO14 014000u
#define SECS600_ADDRESS 02630u
#define SECS600_BANK 026000u /* BBCON STATEINT: FB 13 */
#define Q_FINDVAC 03645u
#define Q_NOVAC 03651u
#define Q_IMEDIATE 03622u
#define Q_LONGCLCL 03734u
#define Q_DISPLAY 03546u

static agc_register com(agc_register a){return (agc_register)(a^0177777u);}
static agc_word cyr(agc_word a){return (agc_word)((a>>1)|((a&1u)<<14));}
/* Erasable address under the task's EBANK (BBCON low bits). */
static agc_word phys(agc_word address,agc_word bank){return address<01400u?address:(agc_word)((bank&7u)*0400u+(address&0377u));}
/* Table word at PRDTTAB+index (01,2000+index); the table starts at 01,2002. */
static int table(unsigned index,agc_word *word){
 if(index<LM_RESTART_TABLE_FIRST||index-LM_RESTART_TABLE_FIRST>=LM_RESTART_TABLE_WORDS)return 0;
 *word=lm_restart_table_data[index-LM_RESTART_TABLE_FIRST];return 1;
}
static enum agc_ccs_class regclass(agc_register a){
 int ov=agc_overflow(a);
 return ov>0?AGC_POSITIVE:ov<0?AGC_NEGATIVE:agc_classify(agc_correct(a));
}
static agc_register dabs(agc_register a){agc_register m=(a&0100000u)?com(a):a;return m?(agc_register)(m-1u):0;}
static agc_register plus(agc_register a,agc_word b){return agc_add_register(a,agc_extend(b));}

/* CHKNOVAC/ITSNOVAC: priority in GOLOC-1 decides FINDVAC or NOVAC. */
static void job(apollo_overlay_state *s,agc_word priority){
 if(priority==0||(priority&AGC_SIGN)!=0){W[GOLOCM1]=LM_RS_NOVAC;W[0]=com(agc_extend(priority));W[2]=Q_NOVAC;}
 else{W[GOLOCM1]=LM_RS_FINDVAC;W[0]=agc_extend(priority);W[2]=Q_FINDVAC;}
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
static enum lm_restart_select_exit long_delay(apollo_overlay_state *s){
 agc_register_pair r;agc_word high,low;enum agc_ccs_class kind;
 if(W[1]!=agc_extend(agc_correct(W[1])))return LM_RS_SELECT_INVALID;
 W[LONGTIME]=agc_correct(W[0]);W[LONGTIME+1u]=agc_correct(W[1]);
 r.a=agc_extend(agc_negate(W[TIME2]));r.l=agc_extend(agc_negate(W[TIME1]));pairadd(s,&r);
 r.a=agc_extend(W[LONGBASE]);r.l=agc_extend(W[LONGBASE+1u]);pairadd(s,&r);
 high=W[LONGTIME];low=W[LONGTIME+1u];kind=agc_classify(high);
 if(kind==AGC_POSITIVE||((kind==AGC_PLUS_ZERO||kind==AGC_MINUS_ZERO)&&agc_classify(low)==AGC_POSITIVE)){
  W[GOLOCM1]=LM_RS_LONGCALL;W[0]=agc_extend(high);W[1]=agc_extend(low);
  W[2]=Q_LONGCLCL;W[5]=GOLOCM1;return LM_RS_SELECTED_LONGCALL;
 }
 /* Negative and signed-zero times share the immediate branches; at -0/-0
  * IMEDIATE adds one to the preceding CCS diminished zero. */
 W[0]=1;W[1]=0;W[2]=Q_IMEDIATE;W[5]=GOLOCM1;
 return LM_RS_SELECTED_TASK;
}
/* TIMETEST (waitlist, delta direct/indirect/immediate) or ITSLGCL1 (longcall,
 * DP delta at an address under the task's banks). */
static enum lm_restart_select_exit timed(apollo_overlay_state *s,uint8_t group,agc_word delta,int lng){
 agc_word bank=W[GOLOCP1];
 if(lng){
  if(delta==SECS600_ADDRESS&&(bank&076000u)==SECS600_BANK){W[0]=3;W[1]=025140;}
  else{agc_word address=phys(delta,bank);if(delta>=02000u||address>=03777u)return LM_RS_SELECT_INVALID;W[0]=agc_extend(W[address]);W[1]=agc_extend(W[address+1u]);}
  W[GOLOCP1]=(agc_word)(bank&076007u);
  return long_delay(s);
 }
 if(delta==077777u){W[0]=1;W[2]=Q_IMEDIATE;W[5]=GOLOCM1;return LM_RS_SELECTED_TASK;}
 if((delta&AGC_SIGN)!=0){agc_word address=agc_negate(delta);if(address>=02000u)return LM_RS_SELECT_INVALID;W[0]=agc_extend(W[phys(address,bank)]);W[GOLOCP1]=(agc_word)(bank&076007u);}
 else W[0]=agc_extend(delta);
 wait_delay(s,group);return LM_RS_SELECTED_TASK;
}

enum lm_restart_select_exit lm_restart_select(apollo_overlay_state *s,enum lm_restart_select_entry entry){
 uint8_t group;agc_word phase,priority,pointer,code,bank,oldloc,oldbank;
 if(s==0||(unsigned)entry>LM_RS_SELECT_TYPEB_TABLE||W[6]!=02003u||W[3]!=01400u||W[4]!=02000u)return LM_RS_SELECT_INVALID;
 /* RESTARTS doubles MPAC+5 into TEMP2G; the second parts index by TEMP2G. */
 if(entry==LM_RS_SELECT_GROUP){if(W[GROUPIX]>=6u)return LM_RS_SELECT_INVALID;group=(uint8_t)W[GROUPIX];}
 else{if(W[TEMP2G]>=12u||(W[TEMP2G]&1u))return LM_RS_SELECT_INVALID;group=(uint8_t)(W[TEMP2G]>>1);}
 phase=W[TEMPPHS];
 if(entry==LM_RS_SELECT_PART2){
  /* PHSPART2: second entry of an even table phase. */
  pointer=(agc_word)(W[POINTER]+3u);
  W[POINTER]=pointer;W[GOLOCP2]=RTRN;
 }else{
  if(phase==0||phase==077777u||phase>01777u)return LM_RS_SELECT_INVALID;
  if(entry==LM_RS_SELECT_GROUP){W[TEMP2G]=(agc_word)(group*2u);W[TEMPSWCH]=LM_RS_PHSPART2;W[GOLOCP2]=RTRN;}
  if((phase&01400u)!=0&&entry==LM_RS_SELECT_GROUP){
   /* ITSAVAR: GOLOC takes the group's stored 2CADR; A,L the old GOLOC. */
   oldloc=W[GOLOC];oldbank=W[GOLOCP1];W[GOLOC]=W[PHSNAME1+group*2u];W[GOLOCP1]=W[PHSNAME1+1u+group*2u];
   W[0]=agc_extend(oldloc);W[1]=agc_extend(oldbank);priority=W[PHSPRDT1+group*2u];
   if((phase&01000u)!=0){W[TEMPSWCH]=RTRN;W[GOLOCP2]=LM_RS_GETPART2;W[TEMPPHS]=(agc_word)(phase&0177u);job(s,priority);return LM_RS_SELECTED_JOB;}
   if((phase&7u)==2u){job(s,priority);return LM_RS_SELECTED_JOB;}
   W[GOLOCM1]=LM_RS_WAITLIST;
   return timed(s,group,(phase&7u)>=3u?agc_negate(priority):priority,(phase&7u)>=3u);
  }
  if(phase==1u){W[0]=PRIO14;W[2]=Q_DISPLAY;W[5]=LM_RS_FINDVAC;return LM_RS_SELECTED_DISPLAY;}
  if(!table(2u+group*2u+(phase&1u),&code))return LM_RS_SELECT_INVALID;
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
 if(!table(pointer,&priority)||!table(pointer+1u,&code)||!table(pointer+2u,&bank))return LM_RS_SELECT_INVALID;
 oldbank=W[GOLOCP1];W[GOLOCP1]=bank;W[1]=agc_extend(oldbank);
 if((code&AGC_SIGN)==0){W[0]=agc_extend(W[GOLOC]);W[GOLOC]=code;job(s,priority);return LM_RS_SELECTED_JOB;}
 W[GOLOC]=agc_negate(code);W[GOLOCM1]=LM_RS_WAITLIST;
 if((bank&01000u)!=0){W[GOLOCP1]=agc_negate(bank);return timed(s,group,priority,0);}
 return timed(s,group,priority,1);
}
