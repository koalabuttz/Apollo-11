/* SPDX-License-Identifier: MIT */
/* Comanche055 UPDATE_PROGRAM: V70UPDAT-V73UPDAT/CKMDMORE/UPERROR (bank 43),
 * UPPART2, OHWELL1/OHWELL2, UPVERIFY, CK4V32, UPSTORE, UPEND73, UPFNDVAC,
 * UPJOB, UPWAKE, TIMEDIDL/TIMEDIDR/DELTAOK, UPEND71/STORLP71,
 * UPEND72/LDLOOP72, UPOUT, UPEND70, UPERROUT and UPACTOFF (bank 27). The
 * bank 27 code is Luminary's bank 04 code (src/lm_update.c) six words lower,
 * with CAF BIT3 for VERIFBIT and the update words in unswitched erasable.
 * One agc_image_ops call per original instruction, under labels L<address>;
 * internal transfers whose target is computed at run time (INDEX UPVERB;
 * TCF, CK4V32's INDEX Q; TC 0 -6, TC Q, TC UPTEMP, INDEX UPTEMP; TC 1) go
 * through `jump`, which takes only the targets the original can reach. */
#include "cm_update.h"
#include "agc_image_ops.h"
#include "cm_update_operands.inc"

#define W s->words
#define B27(z) (uint16_t)(027u*1024u+((z)-02000u))
#define B43(z) (uint16_t)(043u*1024u+((z)-02000u))
#define A 0u
#define L 1u
#define Q 2u
#define EBANK 3u
#define TIME2 024u
#define FLAGWRD5 0101u
#define FLAGWRD7 0103u
#define MPAC 0154u
#define DNLSTCOD 0332u
#define MPHASE6 0764u      /* -PHASE6 */
#define MODREG 01011u
#define UPVERBSV 0331u
#define UPTEMP CM_UP_UPTEMP
#define COMPNUMB CM_UP_COMPNUMB
#define UPOLDMOD 0301u
#define UPVERB CM_UP_UPVERB
#define UPCOUNT CM_UP_UPCOUNT
#define UPBUFF CM_UP_UPBUFF
#define TETCSM 01570u      /* E3 */
#define TETLEM 01642u      /* E3 */
#define TEPHEM 01706u      /* E3 */
/* Entries outside the file. */
#define PHASCHNG 05412u
#define NEWMODEX 05354u
#define POSTJUMP 04701u
#define BANKCALL 04662u
#define FALTON 04400u
#define FINDVAC 05147u
#define ENDOFJOB 05217u
#define INTPRET 06006u
#define UPFLAG 05546u
#define TPAGREE 07226u
#define ENDEXT 05534u
#define INTWAKEU 02671u    /* bank 27 */
#define TESTXACT 02076u    /* bank 43 */
static const agc_word neg0[2]={CM_UPDATE_K_NEG0};

static cm_native_io *ctx(const apollo_overlay_state *s,void *io){
 cm_native_io *x=io;
 if(s==0||x==0||x->interrupt_enabled>1u||(x->superbank&~0160u)||W[7]!=0)return 0;
 return x;
}
static agc_word indexed(agc_word word,agc_register idx){
 return (agc_word)(agc_correct(agc_add_register(agc_extend((agc_word)(agc_correct(idx)&077777u)),agc_extend(word)))&077777u);
}
#define TS(a) do{if(ai_ts(s,(a)))return EXEC_OUT_OF_SCOPE;}while(0)
/* TC to an entry outside the file: Q = the word after the TC. */
#define OUT(ret,target) do{ai_tc(s,(ret));W[5]=(target);return EXEC_OK;}while(0)
/* INDEX A (or K); CA/LXCH K: the indexed word must keep its order and name an
 * erasable word; an update may not write the registers or counters. */
static int ca_indexed(apollo_overlay_state *s,agc_word word,agc_register idx){
 agc_word ins=indexed(word,idx);
 if((ins>>12)!=3u||(ins&07777u)>=02000u)return 0;
 ai_ca(s,ins&01777u);return 1;
}
static int lxch_indexed(apollo_overlay_state *s,agc_word word,agc_register idx){
 agc_word ins=indexed(word,idx);
 if((ins&076000u)!=022000u||ai_ea(s,ins&01777u)<061u)return 0;
 ai_lxch(s,ins&01777u);return 1;
}

/* Bank 43: V70UPDAT 3726 ... to TC TESTXACT, and from its return through
 * CKMDMORE (which takes a nonzero MODREG only when FLAGWRD5 bit 8 is set
 * and MODREG bit 2 alone may be set) to POSTJUMP. */
static enum exec_status bank43(const agc_native_program *p,apollo_overlay_state *s,void *io,agc_word at){
 cm_native_io *x=ctx(s,io);uint8_t c;(void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
 switch(at){
 case 03726u:ai_caf(s,CM_UPDATE_K_UP70);break;  /* V70UPDAT; 3727 TCF V73UPDAT +1 */
 case 03730u:ai_caf(s,CM_UPDATE_K_UP71);break;  /* V71UPDAT */
 case 03732u:ai_caf(s,CM_UPDATE_K_UP72);break;  /* V72UPDAT */
 case 03734u:ai_caf(s,CM_UPDATE_K_UP73);break;  /* V73UPDAT */
 case 03737u:goto L3737;
 default:return EXEC_OUT_OF_SCOPE;}
 TS(UPVERBSV);                                  /* 3735 */
 OUT(03737u,TESTXACT);                          /* 3736 */
L3737:
 ai_ca(s,MODREG);
 if(!ai_bzf(s)){                                /* 3740-3741 BZF +2 */
  ai_tc(s,03743u);                              /* 3742 TC CKMDMORE */
  ai_cs(s,FLAGWRD5);ai_maskf(s,CM_UPDATE_K_BIT8);  /* 3753-3754 */
  c=ai_ccs(s,A);                                /* 3755 */
  if(c==0u)goto L3764;                          /* 3756 UPERLEM TCF UPERROR */
  if(c!=1u)return EXEC_OUT_OF_SCOPE;            /* a masked bit is never negative */
  ai_csf(s,CM_UPDATE_K_TWO);ai_mask(s,MODREG);  /* 3757-3760 */
  c=ai_ccs(s,A);                                /* 3761 */
  if(c==0u||c==2u)goto L3764;                   /* 3762 UPERCMC TCF UPERROR; 3764 */
  if(c!=1u)return EXEC_OUT_OF_SCOPE;            /* -0 would run CADR UPERROUT +2; bit 2 clear rules it out */
  if(agc_correct(W[Q])!=03743u)return EXEC_OUT_OF_SCOPE;
  ai_tc(s,3u);                                  /* 3763 TC Q */
 }
 ai_ca(s,MODREG);TS(UPOLDMOD);                  /* 3743-3744 */
 ai_ca(s,UPVERBSV);TS(UPVERB);                  /* 3745-3746 */
 ai_caf(s,CM_UPDATE_K_ONE);TS(UPCOUNT);         /* 3747-3750 */
 OUT(03752u,POSTJUMP);                          /* 3751 TC POSTJUMP; CADR UPPART2 */
L3764: /* UPERROR */
 OUT(03765u,POSTJUMP);                          /* TC POSTJUMP; CADR UPERROUT +2 */
}

/* Bank 27: UPPART2 3373 ... UPACTOFF 3762. */
static enum exec_status bank27(const agc_native_program *p,apollo_overlay_state *s,void *io,agc_word at){
 cm_native_io *x=ctx(s,io);uint8_t c;agc_word ins;
 if(!x)return EXEC_OUT_OF_SCOPE;
jump:
 switch(at){
 case 03373u:goto L3373; case 03400u:goto L3400; case 03404u:goto L3404; case 03411u:goto L3411;
 case 03414u:goto L3414; case 03416u:goto L3416; case 03421u:goto L3421; case 03422u:goto L3422;
 case 03423u:goto L3423; case 03424u:goto L3424; case 03442u:goto L3442; case 03445u:goto L3445;
 case 03446u:goto L3446; case 03447u:goto L3447; case 03450u:goto L3450; case 03457u:goto L3457;
 case 03462u:goto L3462; case 03463u:goto L3463; case 03464u:goto L3464; case 03465u:goto L3465;
 case 03523u:goto L3523; case 03534u:goto L3534; case 03535u:goto L3535; case 03542u:goto L3542;
 case 03543u:goto L3543; case 03547u:goto L3547; case 03551u:goto L3551; case 03553u:goto L3553;
 case 03573u:goto L3573; case 03613u:goto L3613; case 03615u:goto L3615; case 03621u:goto L3621;
 case 03624u:goto L3624; case 03656u:goto L3656; case 03704u:goto L3704; case 03706u:goto L3706;
 case 03711u:goto L3711; case 03715u:goto L3715; case 03721u:goto L3721; case 03722u:goto L3722;
 case 03732u:goto L3732; case 03751u:goto L3751; case 03753u:goto L3753; case 03754u:goto L3754;
 case 03755u:goto L3755; case 03756u:goto L3756; case 03757u:goto L3757;
 default:return EXEC_OUT_OF_SCOPE;}
L3373: /* UPPART2 */
 OUT(03374u,PHASCHNG);                          /* OCT 07026; OCT 30000; 2CADR UPOUT +1 */
L3400:
 ai_caf(s,CM_UPDATE_K_ONE);TS(DNLSTCOD);        /* 3400-3401 */
 OUT(03403u,NEWMODEX);                          /* 3402; DEC 27 */
L3404:
 ins=indexed(013406u,W[UPVERB]);                /* 3404-3405 INDEX UPVERB; TCF +1 */
 if(ins==013406u)goto L3411;                    /* 3406 TCF +3 */
 if(ins==013407u||ins==013410u)goto L3414;      /* 3407-3410 TCF OHWELL1 */
 if(ins!=013411u)return EXEC_OUT_OF_SCOPE;
L3411:
 ai_caf(s,CM_UPDATE_K_TWO);TS(COMPNUMB);        /* 3411-3412 */
 goto L3437;                                    /* 3413 TCF OHWELL2 */
L3414: /* OHWELL1 */
 ai_caf(s,CM_UPDATE_K_ADUPBUFF);TS(MPAC+2u);
L3416:
 ai_caf(s,CM_UPDATE_K_UPLOADNV);
 OUT(03420u,BANKCALL);                          /* 3417 TC BANKCALL; CADR GOXDSPF */
L3421:
 goto L3704;                                    /* TCF UPOUT4: V34 */
L3422:
 goto L3416;                                    /* TCF OHWELL1 +2: V33 */
L3423:
 ai_tc(s,03424u);goto L3500;                    /* TC CK4V32: data */
L3424:
 ai_csf(s,CM_UPDATE_K_BIT2);ai_ad(s,UPBUFF);     /* 3424-3425 */
 if(ai_bzmf(s))goto L3416;                      /* 3426-3427 */
 ai_cs(s,UPBUFF);ai_adf(s,CM_UPDATE_K_UP21);     /* 3430-3431 */
 if(ai_bzmf(s))goto L3416;                      /* 3432-3433 */
 ai_ca(s,UPBUFF);TS(COMPNUMB);                  /* 3434-3435 */
L3436:
 ai_incr(s,UPCOUNT);
L3437: /* OHWELL2 */
 ai_caf(s,CM_UPDATE_K_ADUPBFM1);ai_ad(s,UPCOUNT);
L3441:
 TS(MPAC+2u);
L3442:
 ai_caf(s,CM_UPDATE_K_UPLOADNV);
 OUT(03444u,BANKCALL);                          /* 3443 TC BANKCALL; CADR GOXDSPF */
L3445:
 goto L3704;                                    /* TCF UPOUT4 */
L3446:
 goto L3442;                                    /* TCF OHWELL2 +3 */
L3447:
 ai_tc(s,03450u);goto L3500;                    /* TC CK4V32 */
L3450:
 ai_cs(s,UPCOUNT);ai_ad(s,COMPNUMB);            /* 3450-3451 */
 if(ai_bzmf(s))goto L3455;                      /* 3452-3453 BZMF UPVERIFY */
 goto L3436;                                    /* 3454 TCF OHWELL2 -1 */
L3455: /* UPVERIFY */
 ai_caf(s,CM_UPDATE_K_ADUPTEMP);TS(MPAC+2u);
L3457:
 ai_caf(s,CM_UPDATE_K_UPVRFYNV);
 OUT(03461u,BANKCALL);                          /* 3460 TC BANKCALL; CADR GOXDSPF */
L3462:
 goto L3704;                                    /* TCF UPOUT4 */
L3463:
 goto L3512;                                    /* TCF UPSTORE */
L3464:
 ai_tc(s,03465u);goto L3500;                    /* TC CK4V32 */
L3465:
 ai_ca(s,UPTEMP);
 if(ai_bzmf(s))goto L3455;                      /* 3466-3467 */
 ai_cs(s,UPTEMP);ai_ad(s,COMPNUMB);ai_adf(s,CM_UPDATE_K_BIT1); /* 3470-3472 */
 if(ai_bzmf(s))goto L3455;                      /* 3473-3474 */
 ai_caf(s,CM_UPDATE_K_ADUPBFM1);ai_ad(s,UPTEMP); /* 3475-3476 */
 goto L3441;                                    /* 3477 TCF OHWELL2 +2 */
L3500: /* CK4V32 */
 ai_cs(s,MPAC);ai_maskf(s,CM_UPDATE_K_BIT6);    /* 3500-3501 */
 c=ai_ccs(s,A);                                 /* 3502 */
 if(c==0u){at=(agc_word)(agc_correct(W[Q])&077777u);ai_tc(s,3u);goto jump;} /* 3503 TC Q */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;             /* a masked bit is never negative */
 ins=indexed(077771u,W[Q]);                     /* 3504-3505 INDEX Q; TC 0 -6 */
 if(ins>=010000u)return EXEC_OUT_OF_SCOPE;
 ai_tc(s,03506u);at=ins;goto jump;
L3512: /* UPSTORE */
 x->interrupt_enabled=0u;                       /* INHINT */
 ai_ca(s,FLAGWRD7);ai_xch(s,L);                 /* 3513-3514 */
 ai_caf(s,CM_UPDATE_K_BIT3);                /* 3515 */
 W[0]=(agc_register)(W[0]^W[1]);                /* 3516-3517 EXTEND; RXOR LCHAN */
 TS(FLAGWRD7);                                  /* 3520 */
 OUT(03522u,PHASCHNG);                          /* 3521; OCT 04026 */
L3523:
 x->interrupt_enabled=0u;                       /* INHINT */
 ai_csf(s,CM_UPDATE_K_TWO);ai_ad(s,UPVERB);     /* 3524-3525 */
 if(ai_bzmf(s))goto L3536;                      /* 3526-3527 BZMF UPFNDVAC */
 ai_dca(s,UPBUFF);ai_dxch(s,UPBUFF+8u);         /* UPEND73 3530-3532 */
 ai_tc(s,03534u);goto L3561;                    /* 3533 TC TIMEDIDL */
L3534:
 OUT(03535u,FALTON);
L3535:
 ai_tc(s,03536u);goto L3704;                    /* TC UPOUT +1 */
L3536: /* UPFNDVAC */
 ai_caf(s,CM_UPDATE_K_CHRPRIO);
 OUT(03540u,FINDVAC);                           /* 3537; 2CADR UPJOB */
L3542:
 OUT(03543u,ENDOFJOB);
L3543: /* UPJOB */
 OUT(03544u,INTPRET);                           /* CALL INTSTALL; UPWAKE EXIT */
L3547:
 OUT(03550u,PHASCHNG);                          /* OCT 04026 */
L3551:
 OUT(03552u,UPFLAG);                            /* ADRES REINTFLG */
L3553:
 x->interrupt_enabled=0u;                       /* INHINT */
 ins=indexed(013556u,W[UPVERB]);                /* 3554-3555 INDEX UPVERB; TCF +1 */
 if(ins==013556u)goto L3715;                    /* 3556 TCF UPEND70 */
 if(ins==013557u)goto L3624;                    /* 3557 TCF UPEND71 */
 if(ins==013560u)goto L3656;                    /* 3560 TCF UPEND72 */
 if(ins==013561u)goto L3561;                    /* V73 never gets here: TIMEDIDL, entered by TCF */
 return EXEC_OUT_OF_SCOPE;
L3561: /* TIMEDIDL */
 ai_qxch(s,UPTEMP);                             /* 3561-3562 */
 ai_caf(s,CM_UPDATE_K_ZERO);ai_zl(s);           /* 3563-3564 */
 ai_dxch(s,TIME2);ai_dxch(s,UPBUFF+18u);        /* 3565-3566 */
 ai_caf(s,CM_UPDATE_K_UPDTPHAS);TS(L);ai_com(s);ai_dxch(s,MPHASE6); /* 3567-3572 */
L3573: /* TIMEDIDR */
 x->interrupt_enabled=0u;                       /* INHINT */
 ai_caf(s,CM_UPDATE_K_ZERO);ai_zl(s);           /* 3574-3575 */
 TS(MPAC+2u);                                   /* 3576 */
 ai_dxch(s,UPBUFF+8u);ai_dxch(s,MPAC);          /* 3577-3600 */
 ai_dca(s,UPBUFF+18u);ai_das(s,MPAC);           /* 3601-3603 */
 if(ai_bzf(s))goto L3614;                       /* 3604-3605 BZF DELTAOK */
 ai_caf(s,CM_UPDATE_K_ZERO);ai_dxch(s,UPBUFF+18u);ai_das(s,TIME2); /* 3606-3610 */
 OUT(03612u,PHASCHNG);                          /* 3611; OCT 04026 */
L3613:
 /* TC UPTEMP: the word TIMEDIDL's QXCH stored there executes as TC. */
 ai_tc(s,03614u);
 at=ai_word(s,UPTEMP);
 if(at!=03534u&&at!=03721u)return EXEC_OUT_OF_SCOPE;
 ai_tc(s,(agc_word)(UPTEMP+1u));goto jump;
L3614: /* DELTAOK */
 OUT(03615u,TPAGREE);
L3615:
 ai_dxch(s,MPAC);ai_das(s,TIME2);               /* 3615-3616 */
 OUT(03620u,PHASCHNG);                          /* 3617; OCT 04026 */
L3621:
 x->interrupt_enabled=0u;                       /* INHINT */
 ins=indexed(1u,W[UPTEMP]);                     /* 3622-3623 INDEX UPTEMP; TC 1 */
 if(ins!=03535u&&ins!=03722u)return EXEC_OUT_OF_SCOPE;
 ai_tc(s,03624u);at=ins;goto jump;
L3624: /* UPEND71 */
 ai_ca(s,UPBUFF+1u);TS(EBANK);                  /* 3624-3625 */
 ai_maskf(s,CM_UPDATE_K_LOW8);TS(UPTEMP);       /* 3626-3627 */
 ai_adf(s,CM_UPDATE_K_NEG3);ai_ad(s,COMPNUMB);  /* 3630-3631 */
 if(ai_bzf(s))goto L3641;                       /* 3632-3633 BZF STORLP71 */
 ai_maskf(s,CM_UPDATE_K_BIT9);                  /* 3634 */
 c=ai_ccs(s,A);                                 /* 3635 */
 if(c==0u)goto L3752;                           /* 3636 TCF UPERROUT */
 if(c==1u)ai_caf(s,CM_UPDATE_K_NEG3);           /* 3637 */
 if(c<3u)ai_ad(s,COMPNUMB);                     /* 3640 */
L3641: /* STORLP71 */
 TS(MPAC);
 if(!ca_indexed(s,030306u,W[A]))return EXEC_OUT_OF_SCOPE; /* 3642-3643 INDEX A; CA UPBUFF +2 */
 TS(L);                                         /* 3644 */
 ai_ca(s,MPAC);ai_ad(s,UPTEMP);                 /* 3645-3646 */
 if(!lxch_indexed(s,023400u,W[A]))return EXEC_OUT_OF_SCOPE; /* 3647-3650 INDEX A; LXCH 1400 */
 c=ai_ccs(s,MPAC);                              /* 3651 */
 if(c==0u)goto L3641;                           /* 3652 TCF STORLP71 */
 if(c!=2u)goto L3703;                           /* 3653 / 3655 TCF UPOUT */
 return EXEC_OUT_OF_SCOPE;                      /* 3654 ADRES UPBUFF -1 would run as TC 0303 */
L3656: /* UPEND72 */
 ai_caf(s,CM_UPDATE_K_BIT1);ai_mask(s,COMPNUMB);/* 3656-3657 */
 c=ai_ccs(s,A);                                 /* 3660 */
 if(c==1u)goto L3752;                           /* 3662 TCF UPERROUT */
 if(c<3u)ai_csf(s,CM_UPDATE_K_BIT2);            /* 3661 TCF +2; 3663 */
 ai_ad(s,COMPNUMB);                             /* 3664 */
L3665: /* LDLOOP72 */
 TS(MPAC);
 if(!ca_indexed(s,030305u,W[A]))return EXEC_OUT_OF_SCOPE; /* 3666-3667 INDEX A; CAE UPBUFF +1 */
 ai_lxch(s,A);                                  /* 3670 */
 c=ai_ccs(s,MPAC);                              /* 3671 */
 if(c==0u)TS(MPAC);                             /* 3672 */
 if(c<=1u){if(!ca_indexed(s,030305u,W[A]))return EXEC_OUT_OF_SCOPE;} /* 3673-3674 INDEX A; CAE UPBUFF +1 */
 else if(c==2u)ai_ca(s,UPBUFF+1u);              /* 3674 */
 TS(EBANK);                                     /* 3675 */
 ai_maskf(s,CM_UPDATE_K_LOW8);                  /* 3676 */
 if(!lxch_indexed(s,023400u,W[A]))return EXEC_OUT_OF_SCOPE; /* 3677-3700 INDEX A; LXCH 1400 */
 c=ai_ccs(s,MPAC);                              /* 3701 */
 if(c==0u)goto L3665;                           /* 3702 TCF LDLOOP72 */
 if(c==2u)goto L3704;
 if(c==3u)goto L3705;                           /* not reached: MPAC counts down from positive */
L3703: /* UPOUT */
 OUT(03704u,INTWAKEU);
L3704:
 ai_ca(s,UPOLDMOD);
L3705:
 OUT(03706u,NEWMODEX+3u);
L3706:
 ai_caf(s,CM_UPDATE_K_ZERO);TS(DNLSTCOD);       /* 3706-3707 */
 ai_tc(s,03711u);goto L3757;                    /* 3710 TC UPACTOFF */
L3711:
 ai_dcaf(s,neg0);ai_dxch(s,MPHASE6);            /* 3711-3713 */
 OUT(03715u,ENDEXT);
L3715: /* UPEND70 */
 ai_dcs(s,UPBUFF);ai_dxch(s,UPBUFF+8u);         /* 3715-3717 */
 ai_tc(s,03721u);goto L3561;                    /* 3720 TC TIMEDIDL */
L3721:
 ai_tc(s,03722u);goto L3752;                    /* TC UPERROUT */
L3722:
 ai_dcs(s,UPBUFF);ai_dxch(s,UPBUFF+10u);        /* 3722-3724 */
 ai_dcs(s,UPBUFF);ai_dxch(s,UPBUFF+12u);        /* 3725-3727 */
 OUT(03731u,PHASCHNG);                          /* 3730; OCT 04026 */
L3732:
 ai_caf(s,CM_UPDATE_K_ZERO);ai_zl(s);ai_dxch(s,UPBUFF+10u);ai_das(s,TETCSM); /* 3732-3735 */
 ai_caf(s,CM_UPDATE_K_ZERO);ai_zl(s);ai_dxch(s,UPBUFF+12u);ai_das(s,TETLEM); /* 3736-3741 */
 ai_caf(s,CM_UPDATE_K_ZERO);ai_zl(s);ai_dxch(s,UPBUFF);ai_das(s,TEPHEM+1u);  /* 3742-3745 */
 ai_ads(s,TEPHEM);                              /* 3746 */
 OUT(03750u,PHASCHNG);                          /* 3747; OCT 04026 */
L3751:
 ai_tc(s,03752u);goto L3703;                    /* TC UPOUT */
L3752: /* UPERROUT */
 OUT(03753u,FALTON);
L3753:
 goto L3703;                                    /* TCF UPOUT */
L3754: /* UPERROUT +2 */
 OUT(03755u,FALTON);
L3755:
 ai_tc(s,03756u);goto L3757;                    /* TC UPACTOFF */
L3756:
 OUT(03757u,ENDEXT);
L3757: /* UPACTOFF */
 if(!p->hardware)return EXEC_OUT_OF_SCOPE;
 ai_csf(s,CM_UPDATE_K_BIT3);
 {agc_word v=(agc_word)(agc_correct(W[0])&p->hardware->channel11);p->hardware->channel11=v;W[0]=agc_extend(v);} /* 3760-3761 EXTEND; WAND DSALMOUT */
 /* 3762 TC Q: back inside the file after its own TC UPACTOFF; from a BANKCALL
  * (FRESH_START, P20-P25) to the word in Q (SWRETURN), with Q = 3. */
 at=(agc_word)(agc_correct(W[Q])&077777u);ai_tc(s,3u);
 if(at==03711u||at==03756u)goto jump;
 W[5]=at;return EXEC_OK;
}

const cm_up_entry cm_up_entries[CM_UP_ROUTINES]={
 {027,03373},{027,03400},{027,03404},{027,03421},{027,03422},{027,03423},{027,03445},{027,03446},{027,03447},
 {027,03462},{027,03463},{027,03464},{027,03523},{027,03535},{027,03542},{027,03543},{027,03547},{027,03551},
 {027,03553},{027,03573},{027,03613},{027,03615},{027,03621},{027,03704},{027,03706},{027,03732},{027,03751},
 {027,03753},{027,03754},{027,03755},{027,03757},{043,03726},{043,03730},{043,03732},{043,03734},{043,03737}};
#define E27(name,z) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io){return bank27(p,s,io,(z));}
#define E43(name,z) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io){return bank43(p,s,io,(z));}
 E27(e3373,03373u) E27(e3400,03400u) E27(e3404,03404u) E27(e3421,03421u) E27(e3422,03422u) E27(e3423,03423u)
 E27(e3445,03445u) E27(e3446,03446u) E27(e3447,03447u) E27(e3462,03462u) E27(e3463,03463u) E27(e3464,03464u)
 E27(e3523,03523u) E27(e3535,03535u) E27(e3542,03542u) E27(e3543,03543u) E27(e3547,03547u) E27(e3551,03551u)
 E27(e3553,03553u) E27(e3573,03573u) E27(e3613,03613u) E27(e3615,03615u) E27(e3621,03621u) E27(e3704,03704u)
 E27(e3706,03706u) E27(e3732,03732u) E27(e3751,03751u) E27(e3753,03753u) E27(e3754,03754u) E27(e3755,03755u) E27(upactoff,03757u)
E43(v70updat,03726u) E43(v71updat,03730u) E43(v72updat,03732u) E43(v73updat,03734u) E43(testxact_ret,03737u)
static const agc_routine_entry entries[CM_UP_ROUTINES]={
 {B27(03373),e3373},{B27(03400),e3400},{B27(03404),e3404},{B27(03421),e3421},{B27(03422),e3422},{B27(03423),e3423},
 {B27(03445),e3445},{B27(03446),e3446},{B27(03447),e3447},{B27(03462),e3462},{B27(03463),e3463},{B27(03464),e3464},
 {B27(03523),e3523},{B27(03535),e3535},{B27(03542),e3542},{B27(03543),e3543},{B27(03547),e3547},{B27(03551),e3551},
 {B27(03553),e3553},{B27(03573),e3573},{B27(03613),e3613},{B27(03615),e3615},{B27(03621),e3621},{B27(03704),e3704},
 {B27(03706),e3706},{B27(03732),e3732},{B27(03751),e3751},{B27(03753),e3753},{B27(03754),e3754},{B27(03755),e3755},{B27(03757),upactoff},
 {B43(03726),v70updat},{B43(03730),v71updat},{B43(03732),v72updat},{B43(03734),v73updat},{B43(03737),testxact_ret}};
const agc_routine_table cm_update_routines={entries,CM_UP_ROUTINES};
