/* SPDX-License-Identifier: MIT */
/* Luminary099 FRESH_START_AND_RESTART, bank 05 and fixed-fixed: SLAP1 (V36
 * fresh start), SKIPSIM, DOFSTART/DOFSTRT1, ENDRSTRT, MR.KLEAN, P00KLEAN,
 * V37KLEAN, GOPROG (hardware restart) with BUTTONS and the ERESTORE repair,
 * DORSTART, SETINFL, ENEMA, GOPROG2/GOPROG2A/GOPROG3, PCLOOP, NXTRST,
 * PACTIVE, PINACT, PTBAD, LIGHTSET, NONAVKEY, STARTSUB/STARTSB1/STARTSB2,
 * DSPOFF and GOTOPOOH. One agc_image_ops call per original instruction,
 * under labels L<address>; the TC Q returns go through `jump`, which takes
 * only this bank's return points (or a fixed-fixed SWRETURN). STARTSIM
 * (2452-2455) is not translated: it runs only if STARTSW is patched. */
#include "lm_fresh_start.h"
#include "agc_image_ops.h"
#include "lm_fresh_start_operands.inc"

#define W s->words
#define B05(z) (uint16_t)(05u*1024u+((z)-02000u))
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define A 0u
#define L 1u
#define Q 2u
/* Entries outside the file. */
#define POSTJUMP 04635u
#define SWCALL 04622u
#define ALARM 05567u
#define MMDSPLAY 05315u
static const agc_word neg0[2]={LM_FRESH_K_NEG0};
static const agc_word lm_fresh_k_swinit_[2]={LM_FRESH_K_SWINIT};
static const agc_word lm_fresh_k_swinit_p4_[2]={LM_FRESH_K_SWINIT_P4};
static const agc_word lm_fresh_k_swinit_p6_[2]={LM_FRESH_K_SWINIT_P6};
static const agc_word lm_fresh_k_idleadr_[2]={LM_FRESH_K_IDLEADR};

static lm_fresh_io *ctx(const apollo_overlay_state *s,void *io){
 lm_fresh_io *x=io;
 if(s==0||x==0||x->control.interrupt_enabled>1u||(x->control.superbank&~0160u)||W[7]!=0)return 0;
 return x;
}
static agc_word indexed(agc_word word,agc_register idx){
 return (agc_word)(agc_correct(agc_add_register(agc_extend((agc_word)(agc_correct(idx)&077777u)),agc_extend(word)))&077777u);
}
#define TS(a) do{if(ai_ts(s,(a)))return EXEC_OUT_OF_SCOPE;}while(0)
/* TS after a load from memory or a channel (CA, CS, DCA, DXCH, XCH, CH_*),
 * then only MASKs and other such TSs: A holds no overflow, so TS never skips. */
#define TSU(a) (void)ai_ts(s,(a))
/* CAF k; TS a (a TSU: CAF loads a constant). */
static void cafts(apollo_overlay_state *s,agc_word k,unsigned a){ai_caf(s,k);(void)ai_ts(s,a);}
#define CAFTS(k,a) cafts(s,(k),(a))
/* DCA NEG0; DXCH -PHASEn. */
static void neg0x(apollo_overlay_state *s,unsigned a){ai_dcaf(s,neg0);ai_dxch(s,a);}
#define NEG0(a) neg0x(s,(a))
/* TC to an entry outside the file: Q = the word after the TC. */
#define OUT(ret,target) do{ai_tc(s,(ret));W[5]=(target);return EXEC_OK;}while(0)
/* The channels this bank reads or writes; channel 11 is program->hardware's. */
static agc_word *channel(lm_fresh_io *x,agc_hardware *hw,agc_word c){
 switch(c){
 case 05u:return &x->channel5; case 06u:return &x->channel6; case 07u:return &x->control.superbank;
 case 011u:return hw?&hw->channel11:0; case 012u:return &x->channel12; case 013u:return &x->channel13;
 case 014u:return &x->channel14; case 015u:return &x->channel15; case 016u:return &x->channel16;
 case 033u:return &x->channel33;
 default:return 0;}
}
/* yaAGC CpuWriteIO: channel 13 bits 12-14 arm the HANDRUPT traps and do not
 * stay in the channel. */
static int write_channel(lm_fresh_io *x,agc_hardware *hw,agc_word c,agc_word v){
 agc_word *ch=channel(x,hw,c);
 if(!ch)return 0;
 if(c==013u){if(v&004000u)x->trap31a=1u;if(v&010000u)x->trap31b=1u;if(v&020000u)x->trap32=1u;v&=043777u;}
 *ch=v;return 1;
}
/* EXTEND; WRITE/WOR/WAND/RAND/ROR/READ k. */
static int chan_op(apollo_overlay_state *s,lm_fresh_io *x,agc_hardware *hw,agc_word c,char op){
 agc_word *ch=channel(x,hw,c),v;
 if(!ch)return 0;
 v=(agc_word)agc_correct(W[0]);
 switch(op){
 case 'W':return write_channel(x,hw,c,v);
 case 'O':v=(agc_word)((v|*ch)&077777u);if(!write_channel(x,hw,c,v))return 0;break;
 case 'A':v&=*ch;if(!write_channel(x,hw,c,v))return 0;break;
 case 'a':v&=*ch;break;
 case 'o':v=(agc_word)((v|*ch)&077777u);break;
 default:v=*ch;break;}
 W[0]=agc_extend(v);return 1;
}
#define CH(c,op) do{if(!chan_op(s,x,hw,(c),(op)))return EXEC_OUT_OF_SCOPE;}while(0)
#define CH_WRITE(c) CH((c),'W')
#define CH_WOR(c) CH((c),'O')
#define CH_WAND(c) CH((c),'A')
#define CH_RAND(c) CH((c),'a')
#define CH_ROR(c) CH((c),'o')
#define CH_READ(c) CH((c),'R')

/* Bank 05: SLAP1 2447 ... STARTSB2/DSPOFF 3345, and GOTOPOOH (fixed-fixed 6001). */
static enum exec_status bank05(const agc_native_program *p,apollo_overlay_state *s,void *io,agc_word at){
 lm_fresh_io *x=ctx(s,io);agc_hardware *hw=p->hardware;uint8_t c;agc_word ins;
 if(!x)return EXEC_OUT_OF_SCOPE;
jump:
 switch(at){
 case 02447u:goto L2447; case 02451u:goto L2451; case 02543u:goto L2543; case 02643u:goto L2643;
 case 02647u:goto L2647; case 02652u:goto L2652; case 02667u:goto L2667; case 02704u:goto L2704;
 case 02731u:goto L2731; case 02765u:goto L2765; case 02767u:goto L2771; case 02770u:goto L2770;
 case 02771u:goto L2771; case 02772u:goto L2772; case 03015u:goto L3015; case 03043u:goto L3043;
 case 03056u:goto L2501; case 03104u:goto L2474; case 03106u:goto L2501; case 06001u:goto L6001;
 default:return EXEC_OUT_OF_SCOPE;}
L2447: /* SLAP1 */
 x->control.interrupt_enabled=0u;                          /* INHINT */
 ai_tc(s,02451u);goto L3107;                                /* 2450 TC STARTSUB */
L2451: /* STARTSW TCF SKIPSIM; STARTSIM 2452-2455 runs only if this word is patched */
 ai_ca(s,01036u); ai_maskf(s,LM_FRESH_K_BITS4_6); ai_adf(s,LM_FRESH_K_BIT15);/* 2456-2460 */
 TS(01036u); CAFTS(LM_FRESH_K_BIT12,0333u); ai_caf(s,LM_FRESH_K_ZERO);/* 2461-2464 */
 TSU(01365u); TSU(0375u); TSU(0376u); TSU(0377u); TSU(0320u); ai_csf(s,LM_FRESH_K_PRIO12);/* 2465-2472 */
 TSU(01313u);                                               /* 2473 */
L2474: /* DOFSTART */
 ai_caf(s,LM_FRESH_K_BIT14); CH_WRITE(011u); ai_csf(s,LM_FRESH_K_ZERO);/* 2474-2477 */
 TSU(055u);                                                 /* 2500 */
L2501: /* DOFSTRT1 */
 CAFTS(LM_FRESH_K_FOUR,01273u); ai_caf(s,LM_FRESH_K_PRIO30);/* 2501-2503 */
 TSU(0366u); CAFTS(LM_FRESH_K_ZERO,01246u); TSU(0371u);/* 2504-2507 */
 TSU(01072u); TSU(01262u); TSU(01263u); TSU(01276u); TSU(01360u);/* 2510-2514 */
 TSU(01362u); TSU(0332u); TSU(01324u); TSU(01501u); CH_WRITE(05u);/* 2515-2522 */
 CH_WRITE(06u); CH_WRITE(012u); CH_WRITE(013u); CH_WRITE(014u);/* 2524-2532 */
 ai_cs(s,01036u); ai_maskf(s,LM_FRESH_K_BITS4_6);          /* 2533-2534 */
 c=ai_ccs(s,A);                                             /* 2535 */
 if(c==0u){ai_tc(s,02537u);goto L2542;}                     /* 2536 TC +4 */
 if(c==1u)ai_caf(s,LM_FRESH_K_BITS4_6);                     /* 2537 */
 if(c<3u)CH_WOR(012u);                                      /* 2540-2541 */
L2542:
 ai_tc(s,02543u);goto L2643;                                /* TC MR.KLEAN */
L2543:
 ai_csf(s,LM_FRESH_K_ZERO); TSU(01011u); ai_caf(s,LM_FRESH_K_IM30INIF);/* 2543-2545 */
 TSU(01302u); CAFTS(LM_FRESH_K_MAXDB,01346u); ai_caf(s,LM_FRESH_K_FOUR);/* 2546-2551 */
 TSU(01325u); CAFTS(LM_FRESH_K_BOOLSTRT,0111u); ai_caf(s,LM_FRESH_K_EBANK6);/* 2552-2555 */
 TSU(03u); CAFTS(LM_FRESH_K_STIKSTRT,01444u); ai_caf(s,LM_FRESH_K_RATESTRT);/* 2556-2561 */
 TSU(01476u); CAFTS(LM_FRESH_K_FULLAPS,01400u); ai_caf(s,LM_FRESH_K_77001OCT);/* 2562-2565 */
 TSU(01403u); TSU(01406u); CAFTS(LM_FRESH_K_60DEC,01405u);/* 2566-2571 */
 TSU(01410u); CAFTS(LM_FRESH_K_ZERO,01407u); ai_caf(s,LM_FRESH_K_TEN);/* 2572-2575 */
 TSU(01404u); CAFTS(LM_FRESH_K_BIT8,01411u); ai_caf(s,LM_FRESH_K_IM33INIT);/* 2576-2601 */
 ai_adf(s,LM_FRESH_K_BIT6); TS(01303u); ai_dcaf(s,lm_fresh_k_swinit_);/* 2602-2605 */
 ai_dxch(s,074u); CAFTS(LM_FRESH_K_SWINIT_P2,076u); ai_caf(s,LM_FRESH_K_REFSMBIT);/* 2606-2611 */
 ai_mask(s,077u); ai_adf(s,LM_FRESH_K_SWINIT_P3); TS(077u); ai_dcaf(s,lm_fresh_k_swinit_p4_);/* 2612-2616 */
 ai_dxch(s,0100u); ai_dcaf(s,lm_fresh_k_swinit_p6_); ai_dxch(s,0102u);/* 2617-2622 */
 ai_caf(s,LM_FRESH_K_SURFFBIT); ai_adf(s,LM_FRESH_K_CMOONBIT);/* 2623-2624 */
 ai_adf(s,LM_FRESH_K_LMOONBIT); ai_mask(s,0104u); ai_adf(s,LM_FRESH_K_SWINIT_P8);/* 2625-2627 */
 TS(0104u); CAFTS(LM_FRESH_K_SWINIT_P9,0105u); ai_caf(s,LM_FRESH_K_APSFLBIT);/* 2630-2633 */
 ai_mask(s,0106u); ai_adf(s,LM_FRESH_K_SWINIT_P10); TS(0106u);/* 2634-2636 */
 CAFTS(LM_FRESH_K_SWINIT_P11,0107u);               /* 2637-2640 */
L2641: /* ENDRSTRT */
 OUT(02642u,POSTJUMP);                                      /* CADR DUMMYJOB +2 */
L2643: /* MR.KLEAN */
 x->control.interrupt_enabled=0u;NEG0(0754u);      /* 2643-2646 -PHASE2 */
L2647: /* P00KLEAN */
 NEG0(0760u);                          /* 2647-2651 -PHASE4 */
L2652: /* V37KLEAN */
 NEG0(0752u);                          /* 2652-2654 -PHASE1 */
 NEG0(0756u);                          /* 2655-2657 -PHASE3 */
 NEG0(0762u);                          /* 2660-2662 -PHASE5 */
 NEG0(0764u);                          /* 2663-2665 -PHASE6 */
 goto retq;                                                 /* 2666 TC Q */
L2667: /* GOPROG */
 ai_incr(s,0320u); ai_lxch(s,02u); CH_ROR(07u); ai_dxch(s,01432u);/* 2667-2673 */
 ai_ca(s,01036u); ai_maskf(s,LM_FRESH_K_BIT4);             /* 2674-2675 */
 if(!ai_bzf(s)){                                             /* 2676-2677 BZF +4 */
 ai_adf(s,LM_FRESH_K_BIT6); CH_WOR(012u);                  /* 2700-2702 */
 }
 ai_tc(s,02704u);goto L3070;                                /* 2703 BUTTONS TC LIGHTSET */
L2704:
 ai_caf(s,LM_FRESH_K_HI5); ai_mask(s,01360u);              /* 2704-2705 */
 if(!ai_bzf(s))goto L3105;                                  /* 2706-2710 TCF NONAVKEY +3 */
 ai_cs(s,01360u);                                           /* 2711 */
 if(ai_bzf(s))goto L2730;                                   /* 2712-2713 BZF DORSTART */
 ai_ad(s,01377u);                                           /* 2714 */
 if(!ai_bzf(s))goto L3105;                                  /* 2715-2717 TCF NONAVKEY +3 */
 ai_ca(s,01374u); TSU(03u); ai_dca(s,01375u);               /* 2720-2723 */
 /* 2724-2725 INDEX SKEEP7; DXCH 0000: restore the two words ERASCHK was testing. */
 ins=indexed(052001u,agc_extend(ai_word(s,01377u)));
 if((ins&076000u)!=052000u||(ins&01777u)<062u)return EXEC_OUT_OF_SCOPE;
 ai_dxch(s,(ins&01777u)-1u);
 CAFTS(LM_FRESH_K_ZERO,01360u);                    /* 2726-2727 */
L2730: /* DORSTART */
 ai_tc(s,02731u);goto L3107;                                /* TC STARTSUB */
L2731: /* SETINFL */
 ai_csf(s,LM_FRESH_K_INTFLBIT); ai_mask(s,0106u); TSU(0106u);/* 2731-2733 */
 ai_caf(s,LM_FRESH_K_9_6_4); ai_mask(s,01036u); ai_adf(s,LM_FRESH_K_BIT15);/* 2734-2736 */
 ai_xch(s,01036u); ai_caf(s,LM_FRESH_K_IFAILINH); ai_mask(s,01302u);/* 2737-2741 */
 ai_adf(s,LM_FRESH_K_IM30INIR); TS(01302u); ai_ca(s,01324u);/* 2742-2744 */
 TSU(0332u); ai_caf(s,LM_FRESH_K_BIT4); CH_WOR(014u); ai_cs(s,0101u);/* 2745-2751 */
 ai_maskf(s,LM_FRESH_K_ENGONBIT);                          /* 2752 */
 c=ai_ccs(s,A);                                             /* 2753 */
 if(c==0u)goto L2761;                                       /* 2754 TCF +5 */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a masked bit is never negative */
 ai_caf(s,LM_FRESH_K_BIT13); CH_WOR(011u);                 /* 2755-2757 */
 goto L2775;                                                /* 2760 TCF GOPROG3 */
L2761:
 ai_caf(s,LM_FRESH_K_BIT14); CH_WOR(011u);                 /* 2761-2763 */
 goto L2775;                                                /* 2764 TCF GOPROG3 */
L2765: /* ENEMA */
 x->control.interrupt_enabled=0u;                          /* INHINT */
 ai_tc(s,02767u);goto L3116;                                /* 2766 TC STARTSB1; 2767 TCF GOPROG2A */
L2770: /* GOPROG2 */
 ai_tc(s,02771u);goto L3146;                                /* TC STARTSB2 */
L2771: /* GOPROG2A */
 ai_tc(s,02772u);goto L3070;                                /* TC LIGHTSET */
L2772:
 ai_csf(s,LM_FRESH_K_RSFLGBTS); ai_mask(s,0106u); TSU(0106u);/* 2772-2774 */
L2775: /* GOPROG3 */
 ai_caf(s,LM_FRESH_K_NUMGRPS);
L2776: /* PCLOOP: each restart group's phase and its complement must agree */
 TS(0161u);ai_ad(s,A);                                      /* 2776-2777 TS MPAC +5; DOUBLE */
 ins=indexed(030753u,W[A]);                                 /* 3000-3002 EXTEND; INDEX A; DCA -PHASE1 */
 if((ins&070000u)!=030000u||(ins&07777u)<062u||(ins&07777u)>01777u)return EXEC_OUT_OF_SCOPE;
 ai_dca(s,(ins&07777u)-1u);
 W[A]=(agc_register)(W[A]^W[L]);                            /* 3003-3004 EXTEND; RXOR LCHAN */
 if(ai_ccs(s,A)!=3u)goto L3054;                             /* 3005-3010 TCF PTBAD */
 c=ai_ccs(s,0161u);                                         /* 3011 */
 if(c==0u)goto L2776;                                       /* 3012 TCF PCLOOP */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* MPAC +5 counts down from NUMGRPS */
 TS(0162u);                                                 /* 3013 */
 OUT(03015u,MMDSPLAY);                                      /* 3014 */
L3015:
 x->control.interrupt_enabled=0u; ai_csf(s,LM_FRESH_K_DIDFLBIT);/* 3015-3016 */
 ai_mask(s,075u); TSU(075u); ai_csf(s,LM_FRESH_K_RODFLBIT); ai_mask(s,075u);/* 3017-3022 */
 TSU(075u); ai_csf(s,LM_FRESH_K_P21FLBIT); ai_mask(s,074u); TSU(074u);/* 3023-3026 */
 ai_caf(s,LM_FRESH_K_NUMGRPS);                             /* 3027 */
L3030: /* NXTRST */
 TS(0161u);ai_ad(s,A);                                      /* 3030-3031 TS MPAC +5; DOUBLE */
 ins=indexed(010753u,W[A]);                                 /* 3032-3033 INDEX A; CCS PHASE1 */
 if((ins&076000u)!=010000u||(ins&01777u)<061u)return EXEC_OUT_OF_SCOPE;
 c=ai_ccs(s,ins&01777u);
 if(c==1u)goto L3043;                                       /* 3035 TCF PINACT */
 if(c==3u)goto L3037;                                       /* 3034 TCF PACTIVE; -0 falls to 3037 */
/* 3036 PACTIVE */
 TS(0154u);
L3037:
 ai_incr(s,0154u); ai_incr(s,0162u); ai_caf(s,LM_FRESH_K_RACTCADR);/* 3037-3041 */
 OUT(03043u,SWCALL);                                        /* 3042 */
L3043: /* PINACT */
 c=ai_ccs(s,0161u);
 if(c==0u)goto L3030;                                       /* 3044 TCF NXTRST */
 if(c==2u)goto L2641;                                       /* 3046 TCF ENDRSTRT */
 if(c==1u){
  c=ai_ccs(s,0162u);                                        /* 3045 */
  if(c==0u)goto L2641;                                      /* 3046 TCF ENDRSTRT */
  if(c==2u)goto L3050;
  if(c==3u)goto L3051;
 }
 ai_caf(s,LM_FRESH_K_BIT15);                                /* 3047 */
L3050:
 ai_mask(s,01011u);
L3051:
 if(ai_bzf(s))goto L6001;                                   /* 3051-3052 BZF GOTOPOOH */
 goto L2641;                                                /* 3053 TCF ENDRSTRT */
L3054: /* PTBAD */
 OUT(03055u,ALARM);                                         /* OCT 1107; 3056 TCF DOFSTRT1 */
L3070: /* LIGHTSET */
 ai_caf(s,LM_FRESH_K_BIT5); CH_RAND(016u);                 /* 3070-3072 */
 if(ai_bzf(s))goto retq;                                    /* 3073-3074 BZF NONAVKEY */
 CH_READ(015u); ai_adf(s,LM_FRESH_K__ELR);                 /* 3076-3077 */
 if(!ai_bzf(s))goto retq;                                   /* 3100-3102 BZF +2; NONAVKEY TC Q */
 ai_tc(s,03104u);goto L3107;                                /* 3103 TC STARTSUB; 3104 TCF DOFSTART */
L3105: /* NONAVKEY +3 */
 ai_tc(s,03106u);goto L3107;                                /* TC STARTSUB; 3106 TCF DOFSTRT1 */
L3107: /* STARTSUB */
 CAFTS(LM_FRESH_K_LDNPHAS1,0335u); ai_caf(s,LM_FRESH_K_BIT6);/* 3107-3111 */
 CH_RAND(033u); ai_adf(s,LM_FRESH_K_RMODINIT); TS(0110u);  /* 3113-3115 */
L3116: /* STARTSB1 */
 CAFTS(LM_FRESH_K_POSMAX,026u); ai_adf(s,LM_FRESH_K_MINUS2);/* 3116-3120 */
 TS(027u); ai_adf(s,LM_FRESH_K_NEGONE); TS(030u); ai_caf(s,LM_FRESH_K_EBANK6);/* 3121-3124 */
 TSU(03u); ai_csf(s,LM_FRESH_K_BIT13); ai_mask(s,01273u); TSU(01273u);/* 3125-3130 */
 CAFTS(LM_FRESH_K_POSMAX,01466u); CH_WAND(013u); ai_caf(s,LM_FRESH_K_ZERO);/* 3131-3135 */
 TSU(01465u); TSU(01472u); ai_csf(s,LM_FRESH_K_ACCSOKAY); ai_mask(s,0111u);/* 3136-3141 */
 TSU(0111u); ai_dcaf(s,lm_fresh_k_idleadr_); ai_dxch(s,01274u);/* 3142-3145 */
L3146: /* STARTSB2 */
 ai_caf(s,LM_FRESH_K_OCT30001); CH_WAND(011u); ai_csf(s,LM_FRESH_K_READRBIT);/* 3146-3151 */
 ai_mask(s,077u); TSU(077u); ai_cs(s,077u); ai_maskf(s,LM_FRESH_K_NR29FBIT);/* 3152-3155 */
 if(!ai_bzf(s))ai_caf(s,LM_FRESH_K_BIT10);                  /* 3156-3160 BZF +2 */
 ai_adf(s,LM_FRESH_K_OCT32001); ai_cs(s,A); ai_mask(s,0110u);/* 3161-3163 */
 TSU(0110u); ai_caf(s,LM_FRESH_K_OCT27470); CH_WAND(012u); ai_csf(s,LM_FRESH_K_NORRMBIT);/* 3164-3170 */
 ai_mask(s,0101u); TSU(0101u); ai_csf(s,LM_FRESH_K_R77FLBIT);/* 3171-3173 */
 ai_mask(s,0101u); TSU(0101u); ai_caf(s,LM_FRESH_K_OCT74160);/* 3174-3176 */
 CH_WAND(013u); ai_caf(s,LM_FRESH_K_BIT12); CH_WOR(013u); ai_caf(s,LM_FRESH_K_BIT6);/* 3200-3204 */
 CH_WAND(014u); CAFTS(LM_FRESH_K_STARTEB,03u); ai_caf(s,LM_FRESH_K_NEG1_2);/* 3206-3211 */
 TSU(01407u); TSU(01406u); TSU(01405u); TSU(01404u); TSU(01403u);/* 3212-3216 */
 TSU(01402u); TSU(01401u); TSU(01400u); ai_csf(s,LM_FRESH_K_ENDTASK);/* 3217-3222 */
 TSU(01410u); TSU(01412u); TSU(01414u); TSU(01416u); TSU(01420u);/* 3223-3227 */
 TSU(01422u); TSU(01424u); TSU(01426u); TSU(01430u); ai_csf(s,LM_FRESH_K_ENDTASK_P1);/* 3230-3234 */
 TSU(01411u); TSU(01413u); TSU(01415u); TSU(01417u); TSU(01421u);/* 3235-3241 */
 TSU(01423u); TSU(01425u); TSU(01427u); TSU(01431u); ai_csf(s,LM_FRESH_K_ZERO);/* 3242-3246 */
 TSU(0167u); TSU(0203u); TSU(0217u); TSU(0233u); TSU(0247u); TSU(0263u);/* 3247-3254 */
 TSU(0277u); TSU(0313u); TSU(01313u); TSU(067u); ai_caf(s,LM_FRESH_K_VAC1ADRC);/* 3255-3261 */
 TSU(0400u); ai_adf(s,LM_FRESH_K_LTHVACA); TS(0454u); ai_adf(s,LM_FRESH_K_LTHVACA);/* 3262-3265 */
 TS(0530u); ai_adf(s,LM_FRESH_K_LTHVACA); TS(0604u); ai_adf(s,LM_FRESH_K_LTHVACA);/* 3266-3271 */
 TS(0660u); ai_caf(s,LM_FRESH_K_TEN);                      /* 3272-3273 */
L3274: /* DSPOFF */
 TS(0154u);ai_csf(s,LM_FRESH_K_BIT12);                      /* 3274-3275 */
 ins=indexed(055023u,agc_extend(ai_word(s,0154u)));         /* 3276-3277 INDEX MPAC; TS DSPTAB */
 if((ins&076000u)!=054000u)return EXEC_OUT_OF_SCOPE;
 TS(ins&01777u);
 c=ai_ccs(s,0154u);                                         /* 3300 */
 if(c==0u)goto L3274;                                       /* 3301 TCF DSPOFF */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* MPAC counts down from TEN */
 TS(01326u); TS(01327u); TS(01330u); TS(01074u); TS(045u); TS(0776u);/* 3302-3307 */
 TS(01042u); TS(01013u); TS(01015u); TS(01012u); TS(01020u);/* 3310-3314 */
 TS(01021u); TS(01001u); TS(01002u); TS(01043u); TS(01312u);/* 3315-3321 */
 TS(01044u); TS(01304u); TS(01305u); TS(01306u); TS(01307u);/* 3322-3326 */
 TS(01314u); TS(0100u); CAFTS(LM_FRESH_K_NOUTCON,01016u);/* 3327-3332 */
 ai_csf(s,LM_FRESH_K_ONE); TSU(01100u); ai_caf(s,LM_FRESH_K_BIT6);/* 3333-3335 */
 ai_mask(s,01303u); ai_adf(s,LM_FRESH_K_IM33INIT); TS(01303u);/* 3336-3340 */
 CAFTS(LM_FRESH_K_LESCHK,01361u); ai_csf(s,LM_FRESH_K_VD1);/* 3341-3343 */
 TSU(0777u);                                                /* 3344 */
 goto retq;                                                 /* 3345 TC Q */
L6001: /* GOTOPOOH */
 CAFTS(LM_FRESH_K_OCT33,01u); ai_cs(s,A); ai_dxch(s,0760u);/* 6001-6004 */
 OUT(06006u,POSTJUMP);                                      /* 6005; CADR GOP00FIX */
retq: /* TC Q: the word in Q executes as TC. A caller in this bank returns
       * to one of its return points; MR.KLEAN, P00KLEAN and V37KLEAN under
       * BANKCALL/IBNKCALL return to fixed-fixed. */
 at=(agc_word)(agc_correct(W[Q])&077777u);ai_tc(s,3u);
 if(at>=04000u&&at<010000u){W[5]=at;return EXEC_OK;}
 switch(at){case 02451u:case 02543u:case 02704u:case 02731u:case 02767u:case 02771u:case 02772u:case 03104u:case 03106u:goto jump;
 default:return EXEC_OUT_OF_SCOPE;}
}

const uint16_t lm_fresh_entries[LM_FS_ROUTINES]={02447u,02643u,02647u,02652u,02667u,02765u,02770u,03015u,03043u,03056u,06001u};
#define E05(name,z) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io){return bank05(p,s,io,(z));}
E05(slap1,02447u) E05(mrklean,02643u) E05(p00klean,02647u) E05(v37klean,02652u) E05(goprog,02667u) E05(enema,02765u)
E05(goprog2,02770u) E05(mmdsplay_ret,03015u) E05(swcall_ret,03043u) E05(alarm_ret,03056u) E05(gotopooh,06001u)
static const agc_routine_entry entries[LM_FS_ROUTINES]={
 {FF(06001),gotopooh},
 {B05(02447),slap1},{B05(02643),mrklean},{B05(02647),p00klean},{B05(02652),v37klean},{B05(02667),goprog},
 {B05(02765),enema},{B05(02770),goprog2},{B05(03015),mmdsplay_ret},{B05(03043),swcall_ret},{B05(03056),alarm_ret}};
const agc_routine_table lm_fresh_start_routines={entries,LM_FS_ROUTINES};
