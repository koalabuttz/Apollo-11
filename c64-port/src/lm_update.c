/* SPDX-License-Identifier: MIT */
/* Luminary099 UPDATE_PROGRAM: V70UPDAT-V73UPDAT/UPERROR (bank 43), UPPART2,
 * OHWELL1/OHWELL2, UPVERIFY, CK4V32, UPSTORE, UPEND73, UPFNDVAC, UPJOB,
 * UPWAKE, TIMEDIDL/TIMEDIDR/DELTATOK, UPEND71/STORLP71, UPEND72/LDLOOP72,
 * UPOUT, UPEND70, UPERROUT and UPACTOFF (bank 04). One agc_image_ops call
 * per original instruction, under labels L<address>; internal transfers
 * whose target is computed at run time (INDEX UPVERB; TCF, CK4V32's INDEX Q;
 * TC 0 -6, TC Q, TC UPTEMP, INDEX UPTEMP; TC 1) go through `jump`, which
 * takes only the targets the original can reach. */
#include "lm_update.h"
#include "agc_image_ops.h"
#include "lm_update_operands.inc"

#define W s->words
#define B04(z) (uint16_t)(04u*1024u+((z)-02000u))
#define B43(z) (uint16_t)(043u*1024u+((z)-02000u))
#define A 0u
#define L 1u
#define Q 2u
#define EBANK 3u
#define TIME2 024u
#define FLAGWRD7 0103u
#define MPAC 0154u
#define DNLSTCOD 0332u
#define MPHASE6 0764u      /* -PHASE6 */
#define MODREG 01011u
#define UPVERBSV 01166u
#define UPTEMP LM_UP_UPTEMP
#define COMPNUMB LM_UP_COMPNUMB
#define UPOLDMOD 01171u
#define UPVERB LM_UP_UPVERB
#define UPCOUNT LM_UP_UPCOUNT
#define UPBUFF LM_UP_UPBUFF
#define TETCSM 01570u      /* E3 */
#define TETLEM 01642u      /* E3 */
#define TEPHEM 01706u      /* E3 */
/* Entries outside the file. */
#define PHASCHNG 05353u
#define NEWMODEX 05311u
#define POSTJUMP 04635u
#define BANKCALL 04616u
#define FALTON 04364u
#define FINDVAC 05105u
#define ENDOFJOB 05155u
#define INTPRET 06037u
#define UPFLAG 05504u
#define TPAGREE 07257u
#define ENDEXT 05472u
#define INTWAKEU 03165u    /* bank 04 */
#define TESTXACT 02076u    /* bank 43 */
static const agc_word neg0[2]={LM_UPDATE_K_NEG0};

static p66_bias_io *ctx(const apollo_overlay_state *s,void *io){
 p66_bias_io *x=io;
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

/* Bank 43: V70UPDAT 3724 ... to TC TESTXACT, and from its return to POSTJUMP. */
static enum exec_status bank43(const agc_native_program *p,apollo_overlay_state *s,void *io,agc_word at){
 p66_bias_io *x=ctx(s,io);(void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
 switch(at){
 case 03724u:ai_caf(s,LM_UPDATE_K_UP70);break;  /* V70UPDAT; 3725 TCF V73UPDAT +1 */
 case 03726u:ai_caf(s,LM_UPDATE_K_UP71);break;  /* V71UPDAT */
 case 03730u:ai_caf(s,LM_UPDATE_K_UP72);break;  /* V72UPDAT */
 case 03732u:ai_caf(s,LM_UPDATE_K_UP73);break;  /* V73UPDAT */
 case 03735u:goto L3735;
 default:return EXEC_OUT_OF_SCOPE;}
 TS(UPVERBSV);                                  /* 3733 */
 OUT(03735u,TESTXACT);                          /* 3734 */
L3735:
 ai_ca(s,MODREG);
 if(!ai_bzf(s))OUT(03741u,POSTJUMP);            /* 3736-3737; UPERROR 3740 TC POSTJUMP; CADR UPERROUT +2 */
 ai_ca(s,MODREG);TS(UPOLDMOD);                  /* 3742-3743 */
 ai_ca(s,UPVERBSV);TS(UPVERB);                  /* 3744-3745 */
 ai_caf(s,LM_UPDATE_K_ONE);TS(UPCOUNT);         /* 3746-3747 */
 OUT(03751u,POSTJUMP);                          /* 3750 TC POSTJUMP; CADR UPPART2 */
}

/* Bank 04: UPPART2 3401 ... UPACTOFF 3770. */
static enum exec_status bank04(const agc_native_program *p,apollo_overlay_state *s,void *io,agc_word at){
 p66_bias_io *x=ctx(s,io);uint8_t c;agc_word ins;
 if(!x)return EXEC_OUT_OF_SCOPE;
jump:
 switch(at){
 case 03401u:goto L3401; case 03406u:goto L3406; case 03412u:goto L3412; case 03417u:goto L3417;
 case 03422u:goto L3422; case 03424u:goto L3424; case 03427u:goto L3427; case 03430u:goto L3430;
 case 03431u:goto L3431; case 03432u:goto L3432; case 03450u:goto L3450; case 03453u:goto L3453;
 case 03454u:goto L3454; case 03455u:goto L3455; case 03456u:goto L3456; case 03465u:goto L3465;
 case 03470u:goto L3470; case 03471u:goto L3471; case 03472u:goto L3472; case 03473u:goto L3473;
 case 03531u:goto L3531; case 03542u:goto L3542; case 03543u:goto L3543; case 03550u:goto L3550;
 case 03551u:goto L3551; case 03555u:goto L3555; case 03557u:goto L3557; case 03561u:goto L3561;
 case 03601u:goto L3601; case 03621u:goto L3621; case 03623u:goto L3623; case 03627u:goto L3627;
 case 03632u:goto L3632; case 03664u:goto L3664; case 03712u:goto L3712; case 03714u:goto L3714;
 case 03717u:goto L3717; case 03723u:goto L3723; case 03727u:goto L3727; case 03730u:goto L3730;
 case 03740u:goto L3740; case 03757u:goto L3757; case 03761u:goto L3761; case 03762u:goto L3762;
 case 03763u:goto L3763; case 03764u:goto L3764;
 default:return EXEC_OUT_OF_SCOPE;}
L3401: /* UPPART2 */
 OUT(03402u,PHASCHNG);                          /* OCT 07026; OCT 30000; 2CADR UPOUT +1 */
L3406:
 ai_caf(s,LM_UPDATE_K_ONE);TS(DNLSTCOD);        /* 3406-3407 */
 OUT(03411u,NEWMODEX);                          /* 3410; DEC 27 */
L3412:
 ins=indexed(013414u,W[UPVERB]);                /* 3412-3413 INDEX UPVERB; TCF +1 */
 if(ins==013414u)goto L3417;                    /* 3414 TCF +3 */
 if(ins==013415u||ins==013416u)goto L3422;      /* 3415-3416 TCF OHWELL1 */
 if(ins!=013417u)return EXEC_OUT_OF_SCOPE;
L3417:
 ai_caf(s,LM_UPDATE_K_TWO);TS(COMPNUMB);        /* 3417-3420 */
 goto L3445;                                    /* 3421 TCF OHWELL2 */
L3422: /* OHWELL1 */
 ai_caf(s,LM_UPDATE_K_ADUPBUFF);TS(MPAC+2u);
L3424:
 ai_caf(s,LM_UPDATE_K_UPLOADNV);
 OUT(03426u,BANKCALL);                          /* 3425 TC BANKCALL; CADR GOXDSPF */
L3427:
 goto L3712;                                    /* TCF UPOUT4: V34 */
L3430:
 goto L3424;                                    /* TCF OHWELL1 +2: V33 */
L3431:
 ai_tc(s,03432u);goto L3506;                    /* TC CK4V32: data */
L3432:
 ai_csf(s,LM_UPDATE_K_BIT2);ai_ad(s,UPBUFF);     /* 3432-3433 */
 if(ai_bzmf(s))goto L3424;                      /* 3434-3435 */
 ai_cs(s,UPBUFF);ai_adf(s,LM_UPDATE_K_UP21);     /* 3436-3437 */
 if(ai_bzmf(s))goto L3424;                      /* 3440-3441 */
 ai_ca(s,UPBUFF);TS(COMPNUMB);                  /* 3442-3443 */
L3444:
 ai_incr(s,UPCOUNT);
L3445: /* OHWELL2 */
 ai_caf(s,LM_UPDATE_K_ADUPBFM1);ai_ad(s,UPCOUNT);
L3447:
 TS(MPAC+2u);
L3450:
 ai_caf(s,LM_UPDATE_K_UPLOADNV);
 OUT(03452u,BANKCALL);                          /* 3451 TC BANKCALL; CADR GOXDSPF */
L3453:
 goto L3712;                                    /* TCF UPOUT4 */
L3454:
 goto L3450;                                    /* TCF OHWELL2 +3 */
L3455:
 ai_tc(s,03456u);goto L3506;                    /* TC CK4V32 */
L3456:
 ai_cs(s,UPCOUNT);ai_ad(s,COMPNUMB);            /* 3456-3457 */
 if(ai_bzmf(s))goto L3463;                      /* 3460-3461 BZMF UPVERIFY */
 goto L3444;                                    /* 3462 TCF OHWELL2 -1 */
L3463: /* UPVERIFY */
 ai_caf(s,LM_UPDATE_K_ADUPTEMP);TS(MPAC+2u);
L3465:
 ai_caf(s,LM_UPDATE_K_UPVRFYNV);
 OUT(03467u,BANKCALL);                          /* 3466 TC BANKCALL; CADR GOXDSPF */
L3470:
 goto L3712;                                    /* TCF UPOUT4 */
L3471:
 goto L3520;                                    /* TCF UPSTORE */
L3472:
 ai_tc(s,03473u);goto L3506;                    /* TC CK4V32 */
L3473:
 ai_ca(s,UPTEMP);
 if(ai_bzmf(s))goto L3463;                      /* 3474-3475 */
 ai_cs(s,UPTEMP);ai_ad(s,COMPNUMB);ai_adf(s,LM_UPDATE_K_BIT1); /* 3476-3500 */
 if(ai_bzmf(s))goto L3463;                      /* 3501-3502 */
 ai_caf(s,LM_UPDATE_K_ADUPBFM1);ai_ad(s,UPTEMP); /* 3503-3504 */
 goto L3447;                                    /* 3505 TCF OHWELL2 +2 */
L3506: /* CK4V32 */
 ai_cs(s,MPAC);ai_maskf(s,LM_UPDATE_K_BIT6);    /* 3506-3507 */
 c=ai_ccs(s,A);                                 /* 3510 */
 if(c==0u){at=(agc_word)(agc_correct(W[Q])&077777u);ai_tc(s,3u);goto jump;} /* 3511 TC Q */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;             /* a masked bit is never negative */
 ins=indexed(077771u,W[Q]);                     /* 3512-3513 INDEX Q; TC 0 -6 */
 if(ins>=010000u)return EXEC_OUT_OF_SCOPE;
 ai_tc(s,03514u);at=ins;goto jump;
L3520: /* UPSTORE */
 x->interrupt_enabled=0u;                       /* INHINT */
 ai_ca(s,FLAGWRD7);ai_xch(s,L);                 /* 3521-3522 */
 ai_caf(s,LM_UPDATE_K_VERIFBIT);                /* 3523 */
 W[0]=(agc_register)(W[0]^W[1]);                /* 3524-3525 EXTEND; RXOR LCHAN */
 TS(FLAGWRD7);                                  /* 3526 */
 OUT(03530u,PHASCHNG);                          /* 3527; OCT 04026 */
L3531:
 x->interrupt_enabled=0u;                       /* INHINT */
 ai_csf(s,LM_UPDATE_K_TWO);ai_ad(s,UPVERB);     /* 3532-3533 */
 if(ai_bzmf(s))goto L3544;                      /* 3534-3535 BZMF UPFNDVAC */
 ai_dca(s,UPBUFF);ai_dxch(s,UPBUFF+8u);         /* UPEND73 3536-3540 */
 ai_tc(s,03542u);goto L3567;                    /* 3541 TC TIMEDIDL */
L3542:
 OUT(03543u,FALTON);
L3543:
 ai_tc(s,03544u);goto L3712;                    /* TC UPOUT +1 */
L3544: /* UPFNDVAC */
 ai_caf(s,LM_UPDATE_K_CHRPRIO);
 OUT(03546u,FINDVAC);                           /* 3545; 2CADR UPJOB */
L3550:
 OUT(03551u,ENDOFJOB);
L3551: /* UPJOB */
 OUT(03552u,INTPRET);                           /* CALL INTSTALL; UPWAKE EXIT */
L3555:
 OUT(03556u,PHASCHNG);                          /* OCT 04026 */
L3557:
 OUT(03560u,UPFLAG);                            /* ADRES REINTFLG */
L3561:
 x->interrupt_enabled=0u;                       /* INHINT */
 ins=indexed(013564u,W[UPVERB]);                /* 3562-3563 INDEX UPVERB; TCF +1 */
 if(ins==013564u)goto L3723;                    /* 3564 TCF UPEND70 */
 if(ins==013565u)goto L3632;                    /* 3565 TCF UPEND71 */
 if(ins==013566u)goto L3664;                    /* 3566 TCF UPEND72 */
 if(ins==013567u)goto L3567;                    /* V73 never gets here: TIMEDIDL, entered by TCF */
 return EXEC_OUT_OF_SCOPE;
L3567: /* TIMEDIDL */
 ai_qxch(s,UPTEMP);                             /* 3567-3570 */
 ai_caf(s,LM_UPDATE_K_ZERO);ai_zl(s);           /* 3571-3572 */
 ai_dxch(s,TIME2);ai_dxch(s,UPBUFF+18u);        /* 3573-3574 */
 ai_caf(s,LM_UPDATE_K_UPDTPHAS);TS(L);ai_com(s);ai_dxch(s,MPHASE6); /* 3575-3600 */
L3601: /* TIMEDIDR */
 x->interrupt_enabled=0u;                       /* INHINT */
 ai_caf(s,LM_UPDATE_K_ZERO);ai_zl(s);           /* 3602-3603 */
 TS(MPAC+2u);                                   /* 3604 */
 ai_dxch(s,UPBUFF+8u);ai_dxch(s,MPAC);          /* 3605-3606 */
 ai_dca(s,UPBUFF+18u);ai_das(s,MPAC);           /* 3607-3611 */
 if(ai_bzf(s))goto L3622;                       /* 3612-3613 BZF DELTATOK */
 ai_caf(s,LM_UPDATE_K_ZERO);ai_dxch(s,UPBUFF+18u);ai_das(s,TIME2); /* 3614-3616 */
 OUT(03620u,PHASCHNG);                          /* 3617; OCT 04026 */
L3621:
 /* TC UPTEMP: the word TIMEDIDL's QXCH stored there executes as TC. */
 ai_tc(s,03622u);
 at=ai_word(s,UPTEMP);
 if(at!=03542u&&at!=03727u)return EXEC_OUT_OF_SCOPE;
 ai_tc(s,(agc_word)(UPTEMP+1u));goto jump;
L3622: /* DELTATOK */
 OUT(03623u,TPAGREE);
L3623:
 ai_dxch(s,MPAC);ai_das(s,TIME2);               /* 3623-3624 */
 OUT(03626u,PHASCHNG);                          /* 3625; OCT 04026 */
L3627:
 x->interrupt_enabled=0u;                       /* INHINT */
 ins=indexed(1u,W[UPTEMP]);                     /* 3630-3631 INDEX UPTEMP; TC 1 */
 if(ins!=03543u&&ins!=03730u)return EXEC_OUT_OF_SCOPE;
 ai_tc(s,03632u);at=ins;goto jump;
L3632: /* UPEND71 */
 ai_ca(s,UPBUFF+1u);TS(EBANK);                  /* 3632-3633 */
 ai_maskf(s,LM_UPDATE_K_LOW8);TS(UPTEMP);       /* 3634-3635 */
 ai_adf(s,LM_UPDATE_K_NEG3);ai_ad(s,COMPNUMB);  /* 3636-3637 */
 if(ai_bzf(s))goto L3647;                       /* 3640-3641 BZF STORLP71 */
 ai_maskf(s,LM_UPDATE_K_BIT9);                  /* 3642 */
 c=ai_ccs(s,A);                                 /* 3643 */
 if(c==0u)goto L3760;                           /* 3644 TCF UPERROUT */
 if(c==1u)ai_caf(s,LM_UPDATE_K_NEG3);           /* 3645 */
 if(c<3u)ai_ad(s,COMPNUMB);                     /* 3646 */
L3647: /* STORLP71 */
 TS(MPAC);
 if(!ca_indexed(s,031176u,W[A]))return EXEC_OUT_OF_SCOPE; /* 3650-3651 INDEX A; CA UPBUFF +2 */
 TS(L);                                         /* 3652 */
 ai_ca(s,MPAC);ai_ad(s,UPTEMP);                 /* 3653-3654 */
 if(!lxch_indexed(s,023400u,W[A]))return EXEC_OUT_OF_SCOPE; /* 3655-3656 INDEX A; LXCH 1400 */
 c=ai_ccs(s,MPAC);                              /* 3657 */
 if(c==0u)goto L3647;                           /* 3660 TCF STORLP71 */
 if(c!=2u)goto L3711;                           /* 3661 / 3663 TCF UPOUT */
 return EXEC_OUT_OF_SCOPE;                      /* 3662 ADRES UPBUFF -1 would run as TC 1173 */
L3664: /* UPEND72 */
 ai_caf(s,LM_UPDATE_K_BIT1);ai_mask(s,COMPNUMB);/* 3664-3665 */
 c=ai_ccs(s,A);                                 /* 3666 */
 if(c==1u)goto L3760;                           /* 3670 TCF UPERROUT */
 if(c<3u)ai_csf(s,LM_UPDATE_K_BIT2);            /* 3667 TCF +2; 3671 */
 ai_ad(s,COMPNUMB);                             /* 3672 */
L3673: /* LDLOOP72 */
 TS(MPAC);
 if(!ca_indexed(s,031175u,W[A]))return EXEC_OUT_OF_SCOPE; /* 3674-3675 INDEX A; CAE UPBUFF +1 */
 ai_lxch(s,A);                                  /* 3676 */
 c=ai_ccs(s,MPAC);                              /* 3677 */
 if(c==0u)TS(MPAC);                             /* 3700 */
 if(c<=1u){if(!ca_indexed(s,031175u,W[A]))return EXEC_OUT_OF_SCOPE;} /* 3701-3702 INDEX A; CAE UPBUFF +1 */
 else if(c==2u)ai_ca(s,UPBUFF+1u);              /* 3702 */
 TS(EBANK);                                     /* 3703 */
 ai_maskf(s,LM_UPDATE_K_LOW8);                  /* 3704 */
 if(!lxch_indexed(s,023400u,W[A]))return EXEC_OUT_OF_SCOPE; /* 3705-3706 INDEX A; LXCH 1400 */
 c=ai_ccs(s,MPAC);                              /* 3707 */
 if(c==0u)goto L3673;                           /* 3710 TCF LDLOOP72 */
 if(c==2u)goto L3712;
 if(c==3u)goto L3713;                           /* not reached: MPAC counts down from positive */
L3711: /* UPOUT */
 OUT(03712u,INTWAKEU);
L3712:
 ai_ca(s,UPOLDMOD);
L3713:
 OUT(03714u,NEWMODEX+3u);
L3714:
 ai_caf(s,LM_UPDATE_K_ZERO);TS(DNLSTCOD);       /* 3714-3715 */
 ai_tc(s,03717u);goto L3765;                    /* 3716 TC UPACTOFF */
L3717:
 ai_dcaf(s,neg0);ai_dxch(s,MPHASE6);            /* 3717-3721 */
 OUT(03723u,ENDEXT);
L3723: /* UPEND70 */
 ai_dcs(s,UPBUFF);ai_dxch(s,UPBUFF+8u);         /* 3723-3725 */
 ai_tc(s,03727u);goto L3567;                    /* 3726 TC TIMEDIDL */
L3727:
 ai_tc(s,03730u);goto L3760;                    /* TC UPERROUT */
L3730:
 ai_dcs(s,UPBUFF);ai_dxch(s,UPBUFF+10u);        /* 3730-3732 */
 ai_dcs(s,UPBUFF);ai_dxch(s,UPBUFF+12u);        /* 3733-3735 */
 OUT(03737u,PHASCHNG);                          /* 3736; OCT 04026 */
L3740:
 ai_caf(s,LM_UPDATE_K_ZERO);ai_zl(s);ai_dxch(s,UPBUFF+10u);ai_das(s,TETCSM); /* 3740-3743 */
 ai_caf(s,LM_UPDATE_K_ZERO);ai_zl(s);ai_dxch(s,UPBUFF+12u);ai_das(s,TETLEM); /* 3744-3747 */
 ai_caf(s,LM_UPDATE_K_ZERO);ai_zl(s);ai_dxch(s,UPBUFF);ai_das(s,TEPHEM+1u);  /* 3750-3753 */
 ai_ads(s,TEPHEM);                              /* 3754 */
 OUT(03756u,PHASCHNG);                          /* 3755; OCT 04026 */
L3757:
 ai_tc(s,03760u);goto L3711;                    /* TC UPOUT */
L3760: /* UPERROUT */
 OUT(03761u,FALTON);
L3761:
 goto L3711;                                    /* TCF UPOUT */
L3762: /* UPERROUT +2 */
 OUT(03763u,FALTON);
L3763:
 ai_tc(s,03764u);goto L3765;                    /* TC UPACTOFF */
L3764:
 OUT(03765u,ENDEXT);
L3765: /* UPACTOFF */
 if(!p->hardware)return EXEC_OUT_OF_SCOPE;
 ai_csf(s,LM_UPDATE_K_BIT3);
 {agc_word v=(agc_word)(agc_correct(W[0])&p->hardware->channel11);p->hardware->channel11=v;W[0]=agc_extend(v);} /* 3766-3767 EXTEND; WAND DSALMOUT */
 at=(agc_word)(agc_correct(W[Q])&077777u);ai_tc(s,3u);goto jump; /* 3770 TC Q */
}

const lm_up_entry lm_up_entries[LM_UP_ROUTINES]={
 {04,03401},{04,03406},{04,03412},{04,03427},{04,03430},{04,03431},{04,03453},{04,03454},{04,03455},
 {04,03470},{04,03471},{04,03472},{04,03531},{04,03543},{04,03550},{04,03551},{04,03555},{04,03557},
 {04,03561},{04,03601},{04,03621},{04,03623},{04,03627},{04,03712},{04,03714},{04,03740},{04,03757},
 {04,03761},{04,03762},{04,03763},
 {043,03724},{043,03726},{043,03730},{043,03732},{043,03735}};
#define E04(name,z) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io){return bank04(p,s,io,(z));}
#define E43(name,z) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io){return bank43(p,s,io,(z));}
E04(e3401,03401u) E04(e3406,03406u) E04(e3412,03412u) E04(e3427,03427u) E04(e3430,03430u) E04(e3431,03431u)
E04(e3453,03453u) E04(e3454,03454u) E04(e3455,03455u) E04(e3470,03470u) E04(e3471,03471u) E04(e3472,03472u)
E04(e3531,03531u) E04(e3543,03543u) E04(e3550,03550u) E04(e3551,03551u) E04(e3555,03555u) E04(e3557,03557u)
E04(e3561,03561u) E04(e3601,03601u) E04(e3621,03621u) E04(e3623,03623u) E04(e3627,03627u) E04(e3712,03712u)
E04(e3714,03714u) E04(e3740,03740u) E04(e3757,03757u) E04(e3761,03761u) E04(e3762,03762u) E04(e3763,03763u)
E43(v70updat,03724u) E43(v71updat,03726u) E43(v72updat,03730u) E43(v73updat,03732u) E43(testxact_ret,03735u)
static const agc_routine_entry entries[LM_UP_ROUTINES]={
 {B04(03401),e3401},{B04(03406),e3406},{B04(03412),e3412},{B04(03427),e3427},{B04(03430),e3430},{B04(03431),e3431},
 {B04(03453),e3453},{B04(03454),e3454},{B04(03455),e3455},{B04(03470),e3470},{B04(03471),e3471},{B04(03472),e3472},
 {B04(03531),e3531},{B04(03543),e3543},{B04(03550),e3550},{B04(03551),e3551},{B04(03555),e3555},{B04(03557),e3557},
 {B04(03561),e3561},{B04(03601),e3601},{B04(03621),e3621},{B04(03623),e3623},{B04(03627),e3627},{B04(03712),e3712},
 {B04(03714),e3714},{B04(03740),e3740},{B04(03757),e3757},{B04(03761),e3761},{B04(03762),e3762},{B04(03763),e3763},
 {B43(03724),v70updat},{B43(03726),v71updat},{B43(03730),v72updat},{B43(03732),v73updat},{B43(03735),testxact_ret}};
const agc_routine_table lm_update_routines={entries,LM_UP_ROUTINES};
