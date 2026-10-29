/* SPDX-License-Identifier: MIT */
/* Luminary099 ALARM_AND_ABORT: ALARM/ALARM2/BORTENT/PRIOENT/LARMENT,
 * CHKFAIL1/CHKFAIL2/FAIL3/PROGLARM/MULTEXIT/MULTFAIL, PRIOLARM, BAILOUT,
 * WHIMPER, POODOO/ABORT2/GOPOODOO/STRTIDLE, CCSHOLE, CURTAINS, BAILOUT1,
 * BOTHABRT, POODOO1, ALARM1/ALMNCADR, VARALARM and FLAGS. One agc_image_ops
 * call per original instruction, under labels L<address>; the alarm code
 * read through `INDEX Q; CA 0` from the callers' compiled operands. */
#include "lm_alarm.h"
#include "agc_image_ops.h"
#include "lm_alarm_sites.inc"

#include "agc_image_w.h"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define B10(z) (uint16_t)(010u*1024u+((z)-02000u))
#define B13(z) (uint16_t)(013u*1024u+((z)-02000u))
#define L 1u
#define Q 2u
#define BBANK 6u
#define ZRUPT 015u
#define BRUPT 017u
#define ITEMP1 061u
#define FLAGWRD2 076u
#define FLAGWRD3 077u
#define FLAGWRD7 0103u
#define FLGWRD10 0106u
#define BUF2 0133u
#define FAILREG 0375u
#define MPHASE4 0760u      /* -PHASE4 */
#define DSPTAB11 01036u    /* DSPTAB +11D */
#define ALMCADR LM_AL_ALMCADR

const agc_site_operand *lm_alarm_operand_table(uint16_t *count){if(count)*count=LM_ALARM_OPERAND_COUNT;return lm_alarm_operands;}

static p66_bias_io *ctx(const apollo_overlay_state *s,void *io){
 p66_bias_io *x=io;
 if(s==0||x==0||x->interrupt_enabled>1u||(x->superbank&~0160u)||W[7]!=0)return 0;
 return x;
}
/* The instruction word an INDEX with value idx makes of `word` (yaAGC:
 * overflow-corrected 16-bit sum of the sign-extended words). */
static agc_word indexed(agc_word word,agc_register idx){
 return (agc_word)(agc_correct(agc_add_register(agc_extend((agc_word)(agc_correct(idx)&077777u)),agc_extend(word)))&077777u);
}
/* INDEX Q; CA 0: the word at Q, the caller's compiled alarm code (or an
 * erasable word). */
static int code(const agc_native_program *p,apollo_overlay_state *s,p66_bias_io *x){
 agc_word ins=indexed(030000u,W[Q]),address=(agc_word)(ins&07777u),w;
 if((ins>>12)!=3u||address<010u)return 0;
 if(address<02000u){ai_ca(s,address);return 1;}
 if(!agc_transfer_operand(p,s,x->superbank,address,&w))return 0;
 ai_caf(s,w);return 1;
}
/* Whether code() would succeed, without the read into A. */
static int code_ok(const agc_native_program *p,const apollo_overlay_state *s,const p66_bias_io *x){
 agc_word ins=indexed(030000u,W[Q]),address=(agc_word)(ins&07777u),w;
 if((ins>>12)!=3u||address<010u)return 0;
 return address<02000u||agc_transfer_operand(p,s,x->superbank,address,&w);
}
/* A 16-bit register whose two top bits differ (TS would skip). */
static int overflowed(agc_register r){return ((r>>14)&1u)!=((r>>15)&1u);}
#define TS(a) do{if(ai_ts(s,(a)))return EXEC_OUT_OF_SCOPE;}while(0)
#define CODE() do{if(!code(p,s,x))return EXEC_OUT_OF_SCOPE;}while(0)
#define INHINT() (x->interrupt_enabled=0u)
#define RELINT() (x->interrupt_enabled=1u)
/* TC to `target` from the word before `ret`. */
#define TC(ret,target) do{ai_tc(s,(ret));W[5]=(target);return EXEC_OK;}while(0)

enum entry { E_ALARM,E_ALARM2,E_BAILOUT,E_BAILOUT_RET,E_WHIMPER,E_POODOO,E_ABORT2,E_POODOO_RET,E_GOPOODOO,
 E_FLAGS_RET,E_V37KLEAN_RET,E_MRKLEAN_RET,E_SUPERSW_RET,E_CCSHOLE,E_CURTAINS,E_CURTAINS_RET,E_BAILOUT1,
 E_POODOO1,E_ALARM1,E_ALMNCADR,E_VARALARM,E_VARALARM_RET,E_PRIOLARM,E_PRIOLARM_RET };

static enum exec_status body(const agc_native_program *p,apollo_overlay_state *s,void *io,enum entry entry){
 p66_bias_io *x=ctx(s,io);uint8_t c;agc_word ins;
 if(!x)return EXEC_OUT_OF_SCOPE;
 /* Refuse before the first write (#318): these entries INHINT or store A
  * or Q (TS) before they read the caller's code at Q, and a TS that would
  * skip refuses after its write. CCSHOLE and CURTAINS read a code of their
  * own; the other entries reach no CODE() before a write. */
 switch(entry){
 case E_ALARM:case E_ALARM2:case E_BAILOUT:case E_POODOO:case E_ABORT2:
 case E_BAILOUT1:case E_POODOO1:case E_ALARM1:case E_ALMNCADR:
  if(!code_ok(p,s,x))return EXEC_OUT_OF_SCOPE;
  break;
 default:break;}
 if(overflowed(W[Q])&&(entry==E_ALARM||entry==E_BAILOUT||entry==E_POODOO||entry==E_CCSHOLE||entry==E_CURTAINS||entry==E_VARALARM))
  return EXEC_OUT_OF_SCOPE;
 if(overflowed(W[0])&&(entry==E_ALARM2||entry==E_ABORT2||entry==E_PRIOLARM||entry==E_VARALARM))return EXEC_OUT_OF_SCOPE;
 switch(entry){
 case E_ALARM:goto L5567; case E_ALARM2:goto L5571; case E_BAILOUT:goto L5634; case E_BAILOUT_RET:goto L5643;
 case E_WHIMPER:goto L5644; case E_POODOO:goto L5652; case E_ABORT2:goto L5654; case E_POODOO_RET:goto L5661;
 case E_GOPOODOO:goto L5665; case E_FLAGS_RET:goto L5670; case E_V37KLEAN_RET:goto L5676; case E_MRKLEAN_RET:goto L5700;
 case E_SUPERSW_RET:goto L5703; case E_CCSHOLE:goto L5705; case E_CURTAINS:goto L5711; case E_CURTAINS_RET:goto L5715;
 case E_BAILOUT1:goto L5716; case E_POODOO1:goto L5726; case E_ALARM1:goto L5732; case E_ALMNCADR:goto L5734;
 case E_VARALARM:goto L5744; case E_VARALARM_RET:goto L5752; case E_PRIOLARM:goto L3562; case E_PRIOLARM_RET:goto L3571;
 default:return EXEC_OUT_OF_SCOPE;}
L5567: /* ALARM */
 INHINT();
 ai_ca(s,Q);                                  /* 5570 */
L5571: /* ALARM2 */
 TS(ALMCADR);
 CODE();                                      /* 5572-5573 INDEX Q; CA 0 */
L5574: /* BORTENT */
 TS(L);
L5575: /* PRIOENT */
 ai_ca(s,BBANK);
L5576:
 W[0]=(agc_word)(W[0]|x->superbank);          /* 5576-5577 EXTEND; ROR SUPERBNK */
 TS(ALMCADR+1u);                              /* 5600 */
L5601: /* LARMENT */
 ai_ca(s,Q);TS(ITEMP1);                       /* 5601-5602 */
L5603: /* CHKFAIL1 */
 c=ai_ccs(s,FAILREG);
 if(c==0u)goto L5607;                         /* 5604 TCF CHKFAIL2 */
 if(c==1u)ai_lxch(s,FAILREG);                 /* 5605 */
 if(c<3u)goto L5621;                          /* 5606 TCF PROGLARM */
L5607: /* CHKFAIL2 */
 c=ai_ccs(s,FAILREG+1u);
 if(c==0u)goto L5613;                         /* 5610 TCF FAIL3 */
 if(c==1u)ai_lxch(s,FAILREG+1u);              /* 5611 */
 if(c<3u)goto L5624;                          /* 5612 TCF MULTEXIT */
L5613: /* FAIL3 */
 ai_ca(s,FAILREG+2u);
 ai_maskf(s,LM_ALARM_K_POSMAX);               /* 5614 */
 c=ai_ccs(s,0u);                              /* 5615 */
 if(c==0u)goto L5630;                         /* 5616 TCF MULTFAIL */
 if(c==1u)ai_lxch(s,FAILREG+2u);              /* 5617 */
 if(c<3u)goto L5624;                          /* 5620 TCF MULTEXIT */
L5621: /* PROGLARM */
 ai_cs(s,DSPTAB11);
 ai_maskf(s,LM_ALARM_K_OCT40400);             /* 5622 */
 ai_ads(s,DSPTAB11);                          /* 5623 */
L5624: /* MULTEXIT */
 ai_xch(s,ITEMP1);
 RELINT();                                    /* 5625 */
 ins=indexed(1u,W[0]);                        /* 5626-5627 INDEX A; TC 1 */
 if(ins<010000u){if(ins<010u)return EXEC_OUT_OF_SCOPE;TC(05630u,ins);}
 if(ins<020000u&&(ins&06000u)){W[5]=(agc_word)(ins&07777u);return EXEC_OK;} /* TCF */
 return EXEC_OUT_OF_SCOPE;
L5630: /* MULTFAIL */
 ai_ca(s,L);
 ai_adf(s,LM_ALARM_K_BIT15);                  /* 5631 */
 TS(FAILREG+2u);                              /* 5632 */
 goto L5624;                                  /* 5633 TCF MULTEXIT */
L3562: /* PRIOLARM (bank 10) */
 INHINT();
 TS(L);                                       /* 3563 */
 ai_ca(s,BUF2);TS(ALMCADR);                   /* 3564-3565 */
 ai_ca(s,BUF2+1u);                            /* 3566 */
 ai_tc(s,03570u);goto L5576;                  /* 3567 TC PRIOENT +1 */
L3571: /* PRIOLARM's return (bank 10) */
 ai_caf(s,LM_ALARM_K_V05N09);
 W[5]=LM_AL_PRIODSPR;return EXEC_OK;          /* 3572 TCF PRIODSPR */
L5634: /* BAILOUT */
 INHINT();
 ai_ca(s,Q);TS(ALMCADR);                      /* 5635-5636 */
 CODE();                                      /* 5637-5640 INDEX Q; CAF 0 */
 ai_tc(s,05642u);goto L5574;                  /* 5641 TC BORTENT */
L5643:
 INHINT();
L5644: /* WHIMPER = ABORT */
 if(!p->hardware)return EXEC_OUT_OF_SCOPE;
 ai_caf(s,LM_ALARM_K_TWO);
 ai_adf(s,05646u);                            /* 5645 AD Z */
 TS(BRUPT);                                   /* 5646 */
 p->hardware->in_interrupt=0u;                /* 5647 RESUME: Z = ZRUPT-1, executing BRUPT (TC 5650) */
 ai_tc(s,W[ZRUPT]);
 TC(05651u,LM_AL_POSTJUMP);                   /* 5650 TC POSTJUMP; 5651 CADR ENEMA */
L5652: /* POODOO */
 INHINT();
 ai_ca(s,Q);                                  /* 5653 */
L5654: /* ABORT2 */
 TS(ALMCADR);
 CODE();                                      /* 5655-5656 */
 ai_tc(s,05660u);goto L5574;                  /* 5657 TC BORTENT */
L5661:
 ai_caf(s,LM_ALARM_K_OCT35);
 TS(L);                                       /* 5662 */
 ai_com(s);                                   /* 5663 */
 ai_dxch(s,MPHASE4);                          /* 5664 */
L5665: /* GOPOODOO */
 INHINT();
 TC(05667u,LM_AL_BANKCALL);                   /* 5666 TC BANKCALL; CADR FLAGS */
L5670:
 ai_ca(s,FLAGWRD7);
 ai_maskf(s,LM_ALARM_K_V37FLBIT);             /* 5671 */
 c=ai_ccs(s,0u);                              /* 5672 */
 if(c==0u)goto L5701;                         /* 5673 TCF STRTIDLE */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;           /* a masked bit is never negative */
 TC(05675u,LM_AL_BANKCALL);                   /* 5674 TC BANKCALL; CADR V37KLEAN */
L5676:
 TC(05677u,LM_AL_BANKCALL);                   /* CADR MR.KLEAN */
L5700:
 goto L5644;                                  /* TCF WHIMPER */
L5701: /* STRTIDLE */
 ai_caf(s,LM_ALARM_K_BBSERVDL);
 TC(05703u,LM_AL_SUPERSW);                    /* 5702 */
L5703:
 TC(05704u,LM_AL_BANKCALL);                   /* CADR SERVIDLE */
L5705: /* CCSHOLE */
 INHINT();
 ai_ca(s,Q);                                  /* 5706 */
 ai_tc(s,05710u);goto L5654;                  /* 5707 TC ABORT2; 5710 OCT 1103 */
L5711: /* CURTAINS */
 INHINT();
 ai_ca(s,Q);                                  /* 5712 */
 ai_tc(s,05714u);goto L5571;                  /* 5713 TC ALARM2; 5714 OCT 217 */
L5715:
 TC(05716u,ALMCADR);                          /* TC ALMCADR */
L5716: /* BAILOUT1 */
 INHINT();
 ai_dxch(s,ALMCADR);                          /* 5717 */
 ai_caf(s,LM_ALARM_K_ADR40400);               /* 5720 */
L5721: /* BOTHABRT */
 TS(ITEMP1);
 CODE();                                      /* 5722-5723 */
 TS(L);                                       /* 5724 */
 goto L5603;                                  /* 5725 TCF CHKFAIL1 */
L5726: /* POODOO1 */
 INHINT();
 ai_dxch(s,ALMCADR);                          /* 5727 */
 ai_caf(s,LM_ALARM_K_ADR77770);               /* 5730 */
 goto L5721;                                  /* 5731 TCF BOTHABRT */
L5732: /* ALARM1 */
 INHINT();
 ai_dxch(s,ALMCADR);                          /* 5733 */
L5734: /* ALMNCADR */
 INHINT();
 CODE();                                      /* 5735-5736 */
 TS(L);                                       /* 5737 */
 goto L5601;                                  /* 5740 TCF LARMENT */
L5744: /* VARALARM */
 INHINT();
 TS(L);                                       /* 5745 */
 ai_ca(s,Q);TS(ALMCADR);                      /* 5746-5747 */
 ai_tc(s,05751u);goto L5575;                  /* 5750 TC PRIOENT; 5751 OCT 14 */
L5752:
 TC(05753u,ALMCADR);                          /* TC ALMCADR */
}
#define ENTRY(name,e) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io){return body(p,s,io,e);}
ENTRY(alarm,E_ALARM) ENTRY(alarm2,E_ALARM2) ENTRY(bailout,E_BAILOUT) ENTRY(bailout_ret,E_BAILOUT_RET)
ENTRY(whimper,E_WHIMPER) ENTRY(poodoo,E_POODOO) ENTRY(abort2,E_ABORT2) ENTRY(poodoo_ret,E_POODOO_RET)
ENTRY(gopoodoo,E_GOPOODOO) ENTRY(flags_ret,E_FLAGS_RET) ENTRY(v37klean_ret,E_V37KLEAN_RET) ENTRY(mrklean_ret,E_MRKLEAN_RET)
ENTRY(supersw_ret,E_SUPERSW_RET) ENTRY(ccshole,E_CCSHOLE) ENTRY(curtains,E_CURTAINS) ENTRY(curtains_ret,E_CURTAINS_RET)
ENTRY(bailout1,E_BAILOUT1) ENTRY(poodoo1,E_POODOO1) ENTRY(alarm1,E_ALARM1) ENTRY(almncadr,E_ALMNCADR)
ENTRY(varalarm,E_VARALARM) ENTRY(varalarm_ret,E_VARALARM_RET) ENTRY(priolarm,E_PRIOLARM) ENTRY(priolarm_ret,E_PRIOLARM_RET)

/* FLAGS (bank 13), entered by BANKCALL from GOPOODOO: TC Q returns. */
static enum exec_status flags(const agc_native_program *p,apollo_overlay_state *s,void *io){
 p66_bias_io *x=ctx(s,io);agc_word ret;(void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
 ai_csf(s,LM_ALARM_K_STATEBIT);ai_mask(s,FLAGWRD3);TS(FLAGWRD3);   /* 3755-3757 */
 ai_csf(s,LM_ALARM_K_REINTBIT);ai_mask(s,FLGWRD10);TS(FLGWRD10);   /* 3760-3762 */
 ai_csf(s,LM_ALARM_K_NODOBIT);ai_mask(s,FLAGWRD2);TS(FLAGWRD2);    /* 3763-3765 */
 /* 3766 TC Q: Z = 2, which executes the word in Q (a TC) with Q = 3. */
 ret=(agc_word)(agc_correct(W[Q])&077777u);
 if(ret>=010000u||ret<010u)return EXEC_OUT_OF_SCOPE;
 TC(3u,ret);
}
/* ALMCADR (erasable), from CURTAINS's or VARALARM's TC ALMCADR: the
 * caller's return address stored there executes as TC (Q = ALMCADR+1). */
static enum exec_status almcadr(const agc_native_program *p,apollo_overlay_state *s,void *io){
 p66_bias_io *x=ctx(s,io);agc_word w;(void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
 w=ai_word(s,ALMCADR);
 if(w<02000u||w>07777u)return EXEC_OUT_OF_SCOPE;
 TC((agc_word)(ALMCADR+1u),w);
}
static const agc_routine_entry entries[LM_AL_ROUTINES]={
 {FF(LM_AL_ALARM),alarm},{FF(LM_AL_ALARM2),alarm2},{FF(LM_AL_BAILOUT),bailout},{FF(LM_AL_BAILOUT_RET),bailout_ret},
 {FF(LM_AL_WHIMPER),whimper},{FF(LM_AL_POODOO),poodoo},{FF(LM_AL_ABORT2),abort2},{FF(LM_AL_POODOO_RET),poodoo_ret},
 {FF(LM_AL_GOPOODOO),gopoodoo},{FF(LM_AL_FLAGS_RET),flags_ret},{FF(LM_AL_V37KLEAN_RET),v37klean_ret},
 {FF(LM_AL_MRKLEAN_RET),mrklean_ret},{FF(LM_AL_SUPERSW_RET),supersw_ret},{FF(LM_AL_CCSHOLE),ccshole},
 {FF(LM_AL_CURTAINS),curtains},{FF(LM_AL_CURTAINS_RET),curtains_ret},{FF(LM_AL_BAILOUT1),bailout1},
 {FF(LM_AL_POODOO1),poodoo1},{FF(LM_AL_ALARM1),alarm1},{FF(LM_AL_ALMNCADR),almncadr},{FF(LM_AL_VARALARM),varalarm},
 {FF(LM_AL_VARALARM_RET),varalarm_ret},{B10(LM_AL_PRIOLARM),priolarm},{B10(LM_AL_PRIOLARM_RET),priolarm_ret},
 {B13(LM_AL_FLAGS),flags},{AGC_TRANSFER_ERASABLE_BASE+LM_AL_ALMCADR,almcadr}};
const agc_routine_table lm_alarm_routines={entries,LM_AL_ROUTINES};
