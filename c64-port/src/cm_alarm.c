/* SPDX-License-Identifier: MIT */
/* Comanche055 ALARM_AND_ABORT: ALARM/ALARM2/BORTENT/PRIOENT/LARMENT,
 * CHKFAIL1/CHKFAIL2/FAIL3/PROGLARM/MULTEXIT/MULTFAIL, PRIOLARM, BAILOUT
 * (= ABORT), WHIMPER, POODOO/ABORT2, CCSHOLE, CURTAINS and VARALARM. One
 * agc_image_ops call per original instruction, under labels L<address>; the
 * alarm code read through `INDEX Q; CA 0` or `INDEX ALMCADR; CAF 0` from the
 * callers' compiled operands. */
#include "cm_alarm.h"
#include "agc_image_ops.h"
#include "cm_alarm_operands.inc"

#include "agc_image_w.h"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define B10(z) (uint16_t)(010u*1024u+((z)-02000u))
#define L 1u
#define Q 2u
#define BBANK 6u
#define ZRUPT 015u
#define BRUPT 017u
#define LOCALARM 057u
#define BANKALRM 060u
#define ITEMP1 061u
#define FLAGWRD7 0103u
#define BUF2 0133u
#define LOC 0164u
#define BANKSET 0165u
#define FAILREG 0375u
#define DSPTAB11 01036u    /* DSPTAB +11D */
#define ALMCADR CM_AL_ALMCADR

const agc_site_operand *cm_alarm_operand_table(uint16_t *count){if(count)*count=CM_ALARM_OPERAND_COUNT;return cm_alarm_operands;}

static cm_native_io *ctx(const apollo_overlay_state *s,void *io){
 cm_native_io *x=io;
 if(s==0||x==0||x->interrupt_enabled>1u||(x->superbank&~0160u)||W[7]!=0)return 0;
 return x;
}
/* The instruction word an INDEX with value idx makes of `word` (yaAGC:
 * overflow-corrected 16-bit sum of the sign-extended words). */
static agc_word indexed(agc_word word,agc_register idx){
 return (agc_word)(agc_correct(agc_add_register(agc_extend((agc_word)(agc_correct(idx)&077777u)),agc_extend(word)))&077777u);
}
/* INDEX idx; CA 0 (ALARM2's idx is Q, BAILOUT's and POODOO's ALMCADR): the
 * word at the caller's return address, its compiled alarm code. A return
 * address in erasable (no caller runs from erasable) is refused. */
static int lookup(const agc_native_program *p,apollo_overlay_state *s,const cm_native_io *x,agc_register idx,agc_word *w){
 agc_word ins=indexed(030000u,idx),address=(agc_word)(ins&07777u);
 if((ins>>12)!=3u||address<010u)return 0;
 if(address<02000u)return 0;
 return agc_transfer_operand(p,s,x->superbank,address,w);
}
static int code(const agc_native_program *p,apollo_overlay_state *s,cm_native_io *x,agc_register idx){
 agc_word w;
 if(!lookup(p,s,x,idx,&w))return 0;
 ai_caf(s,w);return 1;
}
/* A 16-bit register whose two top bits differ (TS would skip). */
static int overflowed(agc_register r){return ((r>>14)&1u)!=((r>>15)&1u);}
#define TS(a) do{if(ai_ts(s,(a)))return EXEC_OUT_OF_SCOPE;}while(0)
#define CODE(i) do{if(!code(p,s,x,(i)))return EXEC_OUT_OF_SCOPE;}while(0)
#define INHINT() (x->interrupt_enabled=0u)
#define RELINT() (x->interrupt_enabled=1u)
/* TC to `target` from the word before `ret`. */
#define TC(ret,target) do{ai_tc(s,(ret));W[5]=(target);return EXEC_OK;}while(0)

enum entry { E_ALARM,E_ALARM2,E_POODOO,E_POODOO_RET,E_POODOO_ABORT,E_DOWNFLAG1_RET,E_DOWNFLAG2_RET,
 E_DOWNFLAG3_RET,E_MRKLEAN_RET,E_CCSHOLE,E_CCSHOLE_RET,E_CURTAINS,E_CURTAINS_RET,E_VARALARM,E_VARALARM_RET,
 E_BAILOUT,E_BAILOUT_RET,E_WHIMPER_M1,E_PRIOLARM,E_PRIOLARM_RET };

static enum exec_status body(const agc_native_program *p,apollo_overlay_state *s,void *io,enum entry entry){
 cm_native_io *x=ctx(s,io);uint8_t c;agc_word ins;
 if(!x)return EXEC_OUT_OF_SCOPE;
 /* Refuse before the first write (#318): ALARM and ALARM2 store A in
  * ALMCADR (TS) and only then read the caller's code at Q. A TS that
  * would skip refuses after its write, so an entry that stores Q (CA Q;
  * TS) or A (ALARM2, PRIOLARM, VARALARM's TS L) checks that first. */
 {agc_word w;
  if((entry==E_ALARM||entry==E_ALARM2)&&!lookup(p,s,x,W[Q],&w))return EXEC_OUT_OF_SCOPE;
  if(overflowed(W[Q])&&(entry==E_ALARM||entry==E_CURTAINS||entry==E_BAILOUT||entry==E_POODOO||entry==E_CCSHOLE||entry==E_VARALARM))
   return EXEC_OUT_OF_SCOPE;
  if(overflowed(W[0])&&(entry==E_ALARM2||entry==E_PRIOLARM||entry==E_VARALARM))return EXEC_OUT_OF_SCOPE;}
 switch(entry){
 case E_ALARM:goto L5650; case E_ALARM2:goto L5652; case E_POODOO:goto L5721; case E_POODOO_RET:goto L5726;
 case E_POODOO_ABORT:goto L5732; case E_DOWNFLAG1_RET:goto L5740; case E_DOWNFLAG2_RET:goto L5742;
 case E_DOWNFLAG3_RET:goto L5744; case E_MRKLEAN_RET:goto L5746; case E_CCSHOLE:goto L5747;
 case E_CCSHOLE_RET:goto L5754; case E_CURTAINS:goto L5757; case E_CURTAINS_RET:goto L5763;
 case E_VARALARM:goto L5764; case E_VARALARM_RET:goto L5772; case E_BAILOUT:goto L7755;
 case E_BAILOUT_RET:goto L7762; case E_WHIMPER_M1:goto L7766; case E_PRIOLARM:goto L3766;
 case E_PRIOLARM_RET:goto L3775;
 default:return EXEC_OUT_OF_SCOPE;}
L5650: /* ALARM */
 INHINT();
 ai_ca(s,Q);                                  /* 5651 */
L5652: /* ALARM2 */
 TS(ALMCADR);
 CODE(W[Q]);                                  /* 5653-5654 INDEX Q; CA 0 */
L5655: /* BORTENT */
 TS(L);
L5656: /* PRIOENT */
 ai_ca(s,BBANK);
L5657:
 W[0]=(agc_word)(W[0]|x->superbank);          /* 5657-5660 EXTEND; ROR SUPERBNK */
 TS(ALMCADR+1u);                              /* 5661 */
 ai_ca(s,Q);TS(ITEMP1);                       /* 5662-5663 LARMENT */
 ai_ca(s,LOC);TS(LOCALARM);                   /* 5664-5665 */
 ai_ca(s,BANKSET);TS(BANKALRM);               /* 5666-5667 */
/* 5670 CHKFAIL1 */
 c=ai_ccs(s,FAILREG);
 if(c==0u)goto L5674;                         /* 5671 TCF CHKFAIL2 */
 if(c==1u)ai_lxch(s,FAILREG);                 /* 5672 */
 if(c<3u)goto L5706;                          /* 5673 TCF PROGLARM */
L5674: /* CHKFAIL2 */
 c=ai_ccs(s,FAILREG+1u);
 if(c==0u)goto L5700;                         /* 5675 TCF FAIL3 */
 if(c==1u)ai_lxch(s,FAILREG+1u);              /* 5676 */
 if(c<3u)goto L5711;                          /* 5677 TCF MULTEXIT */
L5700: /* FAIL3 */
 ai_ca(s,FAILREG+2u);
 ai_maskf(s,CM_ALARM_K_POSMAX);               /* 5701 */
 c=ai_ccs(s,0u);                              /* 5702 */
 if(c==0u)goto L5715;                         /* 5703 TCF MULTFAIL */
 if(c==1u)ai_lxch(s,FAILREG+2u);              /* 5704 */
 if(c<3u)goto L5711;                          /* 5705 TCF MULTEXIT */
L5706: /* PROGLARM */
 ai_cs(s,DSPTAB11);
 ai_maskf(s,CM_ALARM_K_OCT40400);             /* 5707 */
 ai_ads(s,DSPTAB11);                          /* 5710 */
L5711: /* MULTEXIT */
 ai_xch(s,ITEMP1);
 RELINT();                                    /* 5712 */
 ins=indexed(1u,W[0]);                        /* 5713-5714 INDEX A; TC 1 */
 if(ins<010000u){if(ins<010u)return EXEC_OUT_OF_SCOPE;TC(05715u,ins);}
 if(ins<020000u&&(ins&06000u)){W[5]=(agc_word)(ins&07777u);return EXEC_OK;} /* TCF */
 return EXEC_OUT_OF_SCOPE;
L5715: /* MULTFAIL */
 ai_ca(s,L);
 ai_adf(s,CM_ALARM_K_BIT15);                  /* 5716 */
 TS(FAILREG+2u);                              /* 5717 */
 goto L5711;                                  /* 5720 TCF MULTEXIT */
L3766: /* PRIOLARM (bank 10) */
 INHINT();
 TS(L);                                       /* 3767 */
 ai_ca(s,BUF2);TS(ALMCADR);                   /* 3770-3771 */
 ai_ca(s,BUF2+1u);                            /* 3772 */
 ai_tc(s,03774u);goto L5657;                  /* 3773 TC PRIOENT +1 */
L3775: /* PRIOLARM's return (bank 10) */
 ai_caf(s,CM_ALARM_K_V05N09);
 W[5]=CM_AL_PRIODSPR;return EXEC_OK;          /* 3776 TCF PRIODSPR */
L7755: /* BAILOUT = ABORT */
 INHINT();
 ai_ca(s,Q);TS(ALMCADR);                      /* 7756-7757 */
 TC(07761u,CM_AL_BANKCALL);                   /* 7760 TC BANKCALL; CADR VAC5STOR */
L7762:
 CODE(W[ALMCADR]);                            /* 7762-7763 INDEX ALMCADR; CAF 0 */
 ai_tc(s,07765u);goto L5655;                  /* 7764 TC BORTENT; 7765 OCT40400 */
L7766:
 INHINT();
L7767: /* WHIMPER */
 if(!p->hardware)return EXEC_OUT_OF_SCOPE;
 ai_caf(s,CM_ALARM_K_TWO);
 ai_adf(s,07771u);                            /* 7770 AD Z */
 TS(BRUPT);                                   /* 7771 */
 p->hardware->in_interrupt=0u;                /* 7772 RESUME: Z = ZRUPT-1, executing BRUPT (TC 7773) */
 ai_tc(s,W[ZRUPT]);
 TC(07774u,CM_AL_POSTJUMP);                   /* 7773 TC POSTJUMP; 7774 CADR ENEMA */
L5721: /* POODOO */
 INHINT();
 ai_ca(s,Q);TS(ALMCADR);                      /* 5722-5723 */
 TC(05725u,CM_AL_BANKCALL);                   /* 5724 TC BANKCALL; CADR VAC5STOR */
L5726:
 CODE(W[ALMCADR]);                            /* 5726-5727 INDEX ALMCADR; CAF 0 */
L5730: /* ABORT2 */
 ai_tc(s,05731u);goto L5655;                  /* TC BORTENT; 5731 OCT77770 */
L5732:
 ai_caf(s,CM_ALARM_K_V37FLBIT);
 ai_mask(s,FLAGWRD7);                         /* 5733 */
 c=ai_ccs(s,0u);                              /* 5734 */
 if(c==0u){ai_tc(s,05736u);goto L7766;}       /* 5735 TC WHIMPER -1 */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;           /* a masked bit is never negative */
 TC(05737u,CM_AL_DOWNFLAG);                   /* 5736 TC DOWNFLAG; ADRES STATEFLG */
L5740:
 TC(05741u,CM_AL_DOWNFLAG);                   /* ADRES REINTFLG */
L5742:
 TC(05743u,CM_AL_DOWNFLAG);                   /* ADRES NODOFLAG */
L5744:
 TC(05745u,CM_AL_BANKCALL);                   /* CADR MR.KLEAN */
L5746:
 ai_tc(s,05747u);goto L7767;                  /* TC WHIMPER */
L5747: /* CCSHOLE */
 INHINT();
 ai_ca(s,Q);TS(ALMCADR);                      /* 5750-5751 */
 TC(05753u,CM_AL_BANKCALL);                   /* 5752 TC BANKCALL; CADR VAC5STOR */
L5754:
 ai_caf(s,CM_ALARM_K_OCT1103);
 ai_tc(s,05756u);goto L5730;                  /* 5755 TC ABORT2; 5756 OCT 1103 */
L5757: /* CURTAINS */
 INHINT();
 ai_ca(s,Q);                                  /* 5760 */
 ai_tc(s,05762u);goto L5652;                  /* 5761 TC ALARM2; 5762 OCT 217 */
L5763:
 TC(05764u,ALMCADR);                          /* TC ALMCADR */
L5764: /* VARALARM */
 INHINT();
 TS(L);                                       /* 5765 */
 ai_ca(s,Q);TS(ALMCADR);                      /* 5766-5767 */
 ai_tc(s,05771u);goto L5656;                  /* 5770 TC PRIOENT; 5771 OCT 14 */
L5772:
 TC(05773u,ALMCADR);                          /* TC ALMCADR */
}
#define ENTRY(name,e) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io){return body(p,s,io,e);}
ENTRY(alarm,E_ALARM) ENTRY(alarm2,E_ALARM2) ENTRY(poodoo,E_POODOO) ENTRY(poodoo_ret,E_POODOO_RET)
ENTRY(poodoo_abort,E_POODOO_ABORT) ENTRY(downflag1_ret,E_DOWNFLAG1_RET) ENTRY(downflag2_ret,E_DOWNFLAG2_RET)
ENTRY(downflag3_ret,E_DOWNFLAG3_RET) ENTRY(mrklean_ret,E_MRKLEAN_RET) ENTRY(ccshole,E_CCSHOLE)
ENTRY(ccshole_ret,E_CCSHOLE_RET) ENTRY(curtains,E_CURTAINS) ENTRY(curtains_ret,E_CURTAINS_RET)
ENTRY(varalarm,E_VARALARM) ENTRY(varalarm_ret,E_VARALARM_RET) ENTRY(bailout,E_BAILOUT)
ENTRY(bailout_ret,E_BAILOUT_RET) ENTRY(whimper_m1,E_WHIMPER_M1) ENTRY(priolarm,E_PRIOLARM)
ENTRY(priolarm_ret,E_PRIOLARM_RET)

/* ALMCADR (erasable), from CURTAINS's or VARALARM's TC ALMCADR: the
 * caller's return address stored there executes as TC (Q = ALMCADR+1). */
static enum exec_status almcadr(const agc_native_program *p,apollo_overlay_state *s,void *io){
 cm_native_io *x=ctx(s,io);agc_word w;(void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
 w=ai_word(s,ALMCADR);
 if(w<02000u||w>07777u)return EXEC_OUT_OF_SCOPE;
 TC((agc_word)(ALMCADR+1u),w);
}
static const agc_routine_entry entries[CM_AL_ROUTINES]={
 {FF(CM_AL_ALARM),alarm},{FF(CM_AL_ALARM2),alarm2},{FF(CM_AL_POODOO),poodoo},{FF(CM_AL_POODOO_RET),poodoo_ret},
 {FF(CM_AL_POODOO_ABORT),poodoo_abort},{FF(CM_AL_DOWNFLAG1_RET),downflag1_ret},{FF(CM_AL_DOWNFLAG2_RET),downflag2_ret},
 {FF(CM_AL_DOWNFLAG3_RET),downflag3_ret},{FF(CM_AL_MRKLEAN_RET),mrklean_ret},{FF(CM_AL_CCSHOLE),ccshole},
 {FF(CM_AL_CCSHOLE_RET),ccshole_ret},{FF(CM_AL_CURTAINS),curtains},{FF(CM_AL_CURTAINS_RET),curtains_ret},
 {FF(CM_AL_VARALARM),varalarm},{FF(CM_AL_VARALARM_RET),varalarm_ret},{FF(CM_AL_BAILOUT),bailout},
 {FF(CM_AL_BAILOUT_RET),bailout_ret},{FF(CM_AL_WHIMPER_M1),whimper_m1},{B10(CM_AL_PRIOLARM),priolarm},
 {B10(CM_AL_PRIOLARM_RET),priolarm_ret},{AGC_TRANSFER_ERASABLE_BASE+CM_AL_ALMCADR,almcadr}};
const agc_routine_table cm_alarm_routines={entries,CM_AL_ROUTINES};
