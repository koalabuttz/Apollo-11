/* SPDX-License-Identifier: MIT */
/* Comanche055 FRESH_START_AND_RESTART, bank 05 and fixed-fixed: SLAP1 (V36
 * fresh start), SKIPSIM, DOFSTART, ENDRSTRT, MR.KLEAN, P00KLEAN, V37KLEAN,
 * GOPROG (hardware restart) with BUTTONS and the ERESTORE repair, ELRSKIP,
 * NOCOARSE, ENEMA, GOPROG3, PCLOOP, NXTRST, PACTIVE, PINACT, PTBAD,
 * LIGHTSET, NONAVKEY, STARTSUB/STARTSB2, DSPOFF, T5IDLOC and GOTOPOOH. One
 * agc_image_ops call per original instruction, under labels L<address>; the
 * TC Q returns go through `jump`, which takes only this bank's return points
 * (or a fixed-fixed return). STARTSIM (2345-2350) is not translated: it runs
 * only if STARTSW is patched. */
#include "cm_fresh_start.h"
#include "agc_image_ops.h"
#include "cm_fresh_start_operands.inc"

#define W s->words
#define B05(z) (uint16_t)(05u*1024u+((z)-02000u))
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define A 0u
#define L 1u
#define Q 2u
/* Entries outside the file. */
#define BANKCALL 04662u
#define SWCALL 04666u
#define POSTJUMP 04701u
#define IBNKCALL 04740u
#define STOPRATE 04526u
#define WAITLIST 05245u
#define NOQRSM_P1 05336u
#define MMDSPLAY 05360u
#define PHASCHNG 05412u
#define ALARM 05650u
static const agc_word neg0[2]={CM_FRESH_K_NEG0};
static const agc_word k_t5idler[2]={CM_FRESH_K_T5IDLER};
static const agc_word k_swinit_p4[2]={CM_FRESH_K_SWINIT_P4};
static const agc_word k_swinit_p6[2]={CM_FRESH_K_SWINIT_P6};
static const agc_word k_swinit_p10[2]={CM_FRESH_K_SWINIT_P10};

static cm_fresh_io *ctx(const apollo_overlay_state *s,void *io){
 cm_fresh_io *x=io;
 if(s==0||x==0||x->control.interrupt_enabled>1u||(x->control.superbank&~0160u)||W[7]!=0)return 0;
 return x;
}
static agc_word indexed(agc_word word,agc_register idx){
 return (agc_word)(agc_correct(agc_add_register(agc_extend((agc_word)(agc_correct(idx)&077777u)),agc_extend(word)))&077777u);
}
#define TS(a) do{if(ai_ts(s,(a)))return EXEC_OUT_OF_SCOPE;}while(0)
/* TC to an entry outside the file: Q = the word after the TC. */
#define OUT(ret,target) do{ai_tc(s,(ret));W[5]=(target);return EXEC_OK;}while(0)
/* The channels this bank reads or writes. */
static agc_word *channel(cm_fresh_io *x,agc_word c){
 switch(c){
 case 05u:return &x->channel5; case 06u:return &x->channel6; case 07u:return &x->control.superbank;
 case 011u:return &x->control.channel11; case 012u:return &x->control.channel12; case 013u:return &x->control.channel13;
 case 014u:return &x->control.channel14; case 015u:return &x->control.main_keys; case 016u:return &x->control.navigation_keys;
 case 033u:return &x->channel33;
 default:return 0;}
}
/* yaAGC CpuWriteIO: channel 13 bits 12-14 arm the HANDRUPT traps and do not
 * stay in the channel; a write to channel 33 only resets its latched bits
 * 11-15 to 1, but the AGC warning latch (bit 14) sets again at once while
 * the warning condition persists. */
static int write_channel(cm_fresh_io *x,agc_word c,agc_word v){
 agc_word *ch=channel(x,c);
 if(!ch)return 0;
 if(c==013u){if(v&004000u)x->control.trap31a=1u;if(v&010000u)x->control.trap31b=1u;if(v&020000u)x->control.trap32=1u;v&=043777u;}
 if(c==033u){v=(agc_word)(*ch|076000u);if(x->warning)v&=(agc_word)~020000u;}
 *ch=v;return 1;
}
/* EXTEND; WRITE/WOR/WAND/RAND/ROR/READ k. */
static int chan_op(apollo_overlay_state *s,cm_fresh_io *x,agc_word c,char op){
 agc_word *ch=channel(x,c),v;
 if(!ch)return 0;
 v=(agc_word)agc_correct(W[0]);
 switch(op){
 case 'W':return write_channel(x,c,v);
 case 'O':v=(agc_word)((v|*ch)&077777u);if(!write_channel(x,c,v))return 0;break;
 case 'A':v&=*ch;if(!write_channel(x,c,v))return 0;break;
 case 'a':v&=*ch;break;
 case 'o':v=(agc_word)((v|*ch)&077777u);break;
 default:v=*ch;break;}
 W[0]=agc_extend(v);return 1;
}
#define CH(c,op) do{if(!chan_op(s,x,(c),(op)))return EXEC_OUT_OF_SCOPE;}while(0)
#define CH_WRITE(c) CH((c),'W')
#define CH_WOR(c) CH((c),'O')
#define CH_WAND(c) CH((c),'A')
#define CH_RAND(c) CH((c),'a')
#define CH_ROR(c) CH((c),'o')
#define CH_READ(c) CH((c),'R')

/* Bank 05: SLAP1 2342 ... T5IDLOC 3154, and GOTOPOOH (fixed-fixed 4106-4111). */
static enum exec_status bank05(const agc_native_program *p,apollo_overlay_state *s,void *io,agc_word at){
 cm_fresh_io *x=ctx(s,io);uint8_t c;agc_word ins;unsigned idx;
 (void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
jump:
 switch(at){
 case 02342u:goto L2342; case 02344u:goto L2351; case 02425u:goto L2425; case 02501u:goto L2501;
 case 02505u:goto L2505; case 02513u:goto L2513; case 02526u:goto L2526; case 02535u:goto L2535;
 case 02550u:goto L2550; case 02575u:goto L2575; case 02631u:goto L2631; case 02635u:goto L2635;
 case 02651u:goto L2651; case 02653u:goto L2653; case 02654u:goto L2654; case 02667u:goto L2667;
 case 02707u:goto L2707; case 02717u:goto L2717; case 02735u:goto L2735; case 02750u:goto L2365;
 case 02775u:goto L2365; case 03153u:goto L3153; case 04106u:goto L4106; case 04110u:goto L4110;
 default:return EXEC_OUT_OF_SCOPE;}
L2342: /* SLAP1 */
 x->control.interrupt_enabled=0u;                          /* INHINT */
 ai_tc(s,02344u);goto L2776;                                /* 2343 TC STARTSUB */
L2351: /* STARTSW TCF SKIPSIM; STARTSIM 2345-2350 runs only if this word is patched */
 ai_ca(s,01036u); ai_maskf(s,CM_FRESH_K_BITS4_6); ai_adf(s,CM_FRESH_K_BIT15);/* 2351-2353 */
 TS(01036u); ai_caf(s,CM_FRESH_K_ZERO); TS(01365u); TS(0375u);/* 2354-2357 */
 TS(0376u); TS(0377u); TS(01154u); ai_csf(s,CM_FRESH_K_PRIO12);/* 2360-2363 */
 TS(01302u);                                               /* 2364 */
L2365: /* DOFSTART */
 ai_caf(s,CM_FRESH_K_ZERO); TS(01360u); TS(01362u); TS(01501u);/* 2365-2370 */
 CH_WRITE(05u); CH_WRITE(06u); CH_WRITE(011u); CH_WRITE(012u);/* 2371-2400 */
 CH_WRITE(013u); CH_WRITE(014u); TS(01316u); TS(0332u);     /* 2401-2406 */
 TS(0371u); TS(01071u); TS(01130u); TS(01126u); TS(01125u); /* 2407-2413 */
 TS(01044u); ai_cs(s,01036u); ai_maskf(s,CM_FRESH_K_BITS4_6);/* 2414-2416 */
 c=ai_ccs(s,A);                                             /* 2417 */
 if(c==0u){ai_tc(s,02421u);goto L2424;}                     /* 2420 TC +4 */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a masked word is never negative */
 ai_caf(s,CM_FRESH_K_BITS4_6); CH_WOR(012u);                /* 2421-2423 */
L2424:
 ai_tc(s,02425u);goto L2501;                                /* TC MR.KLEAN */
L2425:
 ai_csf(s,CM_FRESH_K_ZERO); TS(01011u); ai_caf(s,CM_FRESH_K_PRIO30);/* 2425-2427 */
 TS(0366u); ai_caf(s,CM_FRESH_K_IM30INIF); TS(01320u);      /* 2430-2432 */
 ai_caf(s,CM_FRESH_K_NEGONE); TS(01303u); ai_caf(s,CM_FRESH_K_OPTINITF);/* 2433-2435 */
 TS(01331u); ai_caf(s,CM_FRESH_K_IM33INIT); TS(01321u);     /* 2436-2440 */
 ai_dcaf(s,k_t5idler); ai_dxch(s,01312u);                   /* 2441-2443 DCA T5IDLER; DXCH T5LOC */
 ai_caf(s,CM_FRESH_K_SWINIT); TS(074u); ai_ca(s,075u);      /* 2444-2446 */
 ai_maskf(s,CM_FRESH_K_NOP01BIT); ai_adf(s,CM_FRESH_K_SWINIT_P1); TS(075u);/* 2447-2451 */
 ai_caf(s,CM_FRESH_K_SWINIT_P2); TS(076u); ai_ca(s,077u);   /* 2452-2454 */
 ai_maskf(s,CM_FRESH_K_BIT13); ai_adf(s,CM_FRESH_K_SWINIT_P3); TS(077u);/* 2455-2457 */
 ai_dcaf(s,k_swinit_p4); ai_dxch(s,0100u);                  /* 2460-2462 */
 ai_dcaf(s,k_swinit_p6); ai_dxch(s,0102u);                  /* 2463-2465 */
 ai_ca(s,0104u); ai_maskf(s,CM_FRESH_K_OCT6200); ai_adf(s,CM_FRESH_K_SWINIT_P8);/* 2466-2470 */
 TS(0104u); ai_caf(s,CM_FRESH_K_SWINIT_P9); TS(0105u);      /* 2471-2473 */
 ai_dcaf(s,k_swinit_p10); ai_dxch(s,0106u);                 /* 2474-2476 */
L2477: /* ENDRSTRT */
 OUT(02500u,POSTJUMP);                                      /* CADR DUMMYJOB +2 */
L2501: /* MR.KLEAN */
 x->control.interrupt_enabled=0u;ai_dcaf(s,neg0);ai_dxch(s,0754u);      /* 2501-2504 -PHASE2 */
L2505: /* P00KLEAN */
 ai_dcaf(s,neg0);ai_dxch(s,0760u);                          /* 2505-2507 -PHASE4 */
 ai_dcaf(s,neg0);ai_dxch(s,0752u);                          /* 2510-2512 -PHASE1 */
L2513: /* V37KLEAN */
 ai_dcaf(s,neg0);ai_dxch(s,0756u);                          /* 2513-2515 -PHASE3 */
 ai_dcaf(s,neg0);ai_dxch(s,0762u);                          /* 2516-2520 -PHASE5 */
 ai_dcaf(s,neg0);ai_dxch(s,0764u);                          /* 2521-2523 -PHASE6 */
 goto retq;                                                 /* 2524 TC Q */
L2526: /* GOPROG */
 ai_incr(s,01154u); ai_lxch(s,02u); CH_ROR(07u); ai_dxch(s,01432u);/* 2526-2532 */
 OUT(02534u,BANKCALL);                                      /* 2533 TC BANKCALL; CADR VAC5STOR */
L2535:
 ai_caf(s,CM_FRESH_K_BIT15); CH_WAND(033u);                 /* 2535-2537 */
 if(ai_bzf(s))goto L2547;                                   /* 2540-2541 BZF BUTTONS */
 ai_caf(s,CM_FRESH_K_BIT14); CH_RAND(033u);                 /* 2542-2544 */
 if(ai_bzf(s))goto L2774;                                   /* 2545-2546 BZF NONAVKEY +1 */
L2547: /* BUTTONS */
 ai_tc(s,02550u);goto L2753;                                /* TC LIGHTSET */
L2550:
 ai_caf(s,CM_FRESH_K_HI5); ai_mask(s,01360u);               /* 2550-2551 */
 if(!ai_bzf(s))goto L2774;                                  /* 2552-2554 BZF +2; TCF NONAVKEY +1 */
 ai_cs(s,01360u);                                           /* 2555 */
 if(ai_bzf(s))goto L2574;                                   /* 2556-2557 BZF ELRSKIP -1 */
 ai_ad(s,01377u);                                           /* 2560 */
 if(!ai_bzf(s))goto L2774;                                  /* 2561-2563 BZF +2; TCF NONAVKEY +1 */
 ai_ca(s,01374u); TS(03u); ai_dca(s,01375u);                /* 2564-2567 */
 /* 2570-2571 INDEX SKEEP7; DXCH 0000: restore the two words ERASCHK was testing. */
 ins=indexed(052001u,agc_extend(ai_word(s,01377u)));
 if((ins&076000u)!=052000u||(ins&01777u)<062u)return EXEC_OUT_OF_SCOPE;
 ai_dxch(s,(ins&01777u)-1u);
 ai_caf(s,CM_FRESH_K_ZERO); TS(01360u);                     /* 2572-2573 */
L2574:
 ai_tc(s,02575u);goto L2776;                                /* TC STARTSUB */
L2575: /* ELRSKIP */
 ai_ca(s,0102u); ai_mpf(s,CM_FRESH_K_BIT3); ai_maskf(s,CM_FRESH_K_SIX);/* 2575-2600 */
 idx=(unsigned)(agc_correct(W[A])&077777u);                 /* 2601-2603 EXTEND; INDEX A; DCA T5IDLER */
 if(idx>6u)return EXEC_OUT_OF_SCOPE;
 ai_dcaf(s,&cm_fresh_t5idler[idx]); ai_dxch(s,01312u);      /* 2604 DXCH T5LOC */
 ai_csf(s,CM_FRESH_K_INTFLBIT); ai_mask(s,0106u); TS(0106u);/* 2605-2607 */
 ai_ca(s,01331u); ai_maskf(s,CM_FRESH_K_OPTINITR); ai_adf(s,CM_FRESH_K_BIT7);/* 2610-2612 */
 TS(01331u); ai_caf(s,CM_FRESH_K_BIT6); ai_mask(s,01321u);  /* 2613-2615 */
 ai_adf(s,CM_FRESH_K_IM33INIT); TS(01321u); ai_caf(s,CM_FRESH_K_9_6_4);/* 2616-2620 */
 ai_mask(s,01036u); ai_adf(s,CM_FRESH_K_BIT15); ai_xch(s,01036u);/* 2621-2623 */
 ai_maskf(s,CM_FRESH_K_BIT4);                               /* 2624 */
 if(ai_bzf(s))goto L2635;                                   /* 2625-2626 BZF NOCOARSE */
 OUT(02630u,IBNKCALL);                                      /* 2627 TC IBNKCALL; CADR SETCOARS */
L2631:
 ai_caf(s,CM_FRESH_K_SIX);
 OUT(02633u,WAITLIST);                                      /* 2632 TC WAITLIST; 2CADR CA+ECE */
L2635: /* NOCOARSE */
 ai_caf(s,CM_FRESH_K_IFAILINH); ai_mask(s,01320u); ai_adf(s,CM_FRESH_K_IM30INIR);/* 2635-2637 */
 TS(01320u); ai_cs(s,0101u); ai_maskf(s,CM_FRESH_K_ENGONBIT);/* 2640-2642 */
 c=ai_ccs(s,A);                                             /* 2643 */
 if(c==0u)goto L2667;                                       /* 2644 TCF GOPROG3 */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a masked word is never negative */
 ai_caf(s,CM_FRESH_K_BIT13); CH_WOR(011u);                  /* 2645-2647 */
 goto L2667;                                                /* 2650 TCF GOPROG3 */
L2651: /* ENEMA (GOPROG2) */
 x->control.interrupt_enabled=0u;                           /* INHINT */
 ai_tc(s,02653u);goto L2753;                                /* 2652 TC LIGHTSET */
L2653:
 ai_tc(s,02654u);goto L3006;                                /* TC STARTSB2 */
L2654:
 ai_csf(s,CM_FRESH_K_INTMASK); ai_mask(s,0106u); TS(0106u); /* 2654-2656 */
 ai_cs(s,0102u); ai_maskf(s,CM_FRESH_K_OCT60000);           /* 2657-2660 */
 if(ai_bzmf(s))goto L2667;                                  /* 2661-2662 BZMF GOPROG3 */
 ai_caf(s,CM_FRESH_K__5SEC);
 OUT(02665u,WAITLIST);                                      /* 2664 TC WAITLIST; 2CADR TVCEXEC */
L2667: /* GOPROG3 */
 ai_caf(s,CM_FRESH_K_NUMGRPS);
L2670: /* PCLOOP: each restart group's phase and its complement must agree */
 TS(0161u);ai_ad(s,A);                                      /* 2670-2671 TS MPAC +5; DOUBLE */
 ins=indexed(030753u,W[A]);                                 /* 2672-2674 EXTEND; INDEX A; DCA -PHASE1 */
 if((ins&070000u)!=030000u||(ins&07777u)<062u||(ins&07777u)>01777u)return EXEC_OUT_OF_SCOPE;
 ai_dca(s,(ins&07777u)-1u);
 W[A]=(agc_register)(W[A]^W[L]);                            /* 2675-2676 EXTEND; RXOR LCHAN */
 if(ai_ccs(s,A)!=3u)goto L2746;                             /* 2677-2702 TCF PTBAD */
 c=ai_ccs(s,0161u);                                         /* 2703 */
 if(c==0u)goto L2670;                                       /* 2704 TCF PCLOOP */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* MPAC +5 counts down from NUMGRPS */
 TS(0162u);                                                 /* 2705 */
 OUT(02707u,MMDSPLAY);                                      /* 2706 */
L2707:
 x->control.interrupt_enabled=0u; ai_ca(s,0102u); ai_maskf(s,CM_FRESH_K_OCT60000);/* 2707-2711 */
 if(ai_bzmf(s))goto L2721;                                  /* 2712-2713 BZMF NXTRST -1 */
 ai_caf(s,CM_FRESH_K_EBANK6); TS(03u);                      /* 2714-2715 */
 OUT(02717u,STOPRATE);                                      /* 2716 */
L2717:
 ai_caf(s,CM_FRESH_K_EBANK3); TS(03u);                      /* 2717-2720 */
L2721:
 ai_caf(s,CM_FRESH_K_NUMGRPS);
L2722: /* NXTRST */
 TS(0161u);ai_ad(s,A);                                      /* 2722-2723 TS MPAC +5; DOUBLE */
 ins=indexed(010753u,W[A]);                                 /* 2724-2725 INDEX A; CCS PHASE1 */
 if((ins&076000u)!=010000u||(ins&01777u)<061u)return EXEC_OUT_OF_SCOPE;
 c=ai_ccs(s,ins&01777u);
 if(c==1u)goto L2735;                                       /* 2727 TCF PINACT */
 if(c==3u)goto L2731;                                       /* 2726 TCF PACTIVE; -0 falls to 2731 */
/* 2730 PACTIVE */
 TS(0154u);
L2731:
 ai_incr(s,0154u); ai_incr(s,0162u); ai_caf(s,CM_FRESH_K_RACTCADR);/* 2731-2733 */
 OUT(02735u,SWCALL);                                        /* 2734 */
L2735: /* PINACT */
 c=ai_ccs(s,0161u);
 if(c==0u)goto L2722;                                       /* 2736 TCF NXTRST */
 if(c==2u)goto L2477;                                       /* 2740 TCF ENDRSTRT */
 if(c==1u){
  c=ai_ccs(s,0162u);                                        /* 2737 */
  if(c==0u)goto L2477;                                      /* 2740 TCF ENDRSTRT */
  if(c==2u)goto L2742;
  if(c==3u)goto L2743;
 }
 ai_caf(s,CM_FRESH_K_BIT15);                                /* 2741 */
L2742:
 ai_mask(s,01011u);
L2743:
 if(ai_bzf(s))goto L4106;                                   /* 2743-2744 BZF GOTOPOOH */
 goto L2477;                                                /* 2745 TCF ENDRSTRT */
L2746: /* PTBAD */
 OUT(02747u,ALARM);                                         /* OCT 1107; 2750 TCF DOFSTART */
L2753: /* LIGHTSET */
 ai_caf(s,CM_FRESH_K_BIT7); CH_RAND(016u);                  /* 2753-2755 */
 if(ai_bzf(s))goto retq;                                    /* 2756-2757 BZF NONAVKEY */
 ai_caf(s,CM_FRESH_K_OCT37); CH_RAND(016u); ai_adf(s,CM_FRESH_K__ELR);/* 2760-2763 */
 if(ai_bzf(s))goto L2774;                                   /* 2764-2765 BZF NONAVKEY +1 */
 CH_READ(015u); ai_adf(s,CM_FRESH_K__ELR);                  /* 2766-2770 */
 if(!ai_bzf(s))goto retq;                                   /* 2771-2772 BZF +2; NONAVKEY TC Q */
L2774: /* NONAVKEY +1 */
 ai_tc(s,02775u);goto L2776;                                /* TC STARTSUB; 2775 TCF DOFSTART */
L2776: /* STARTSUB */
 ai_caf(s,CM_FRESH_K_LDNPHAS1); TS(0335u); ai_caf(s,CM_FRESH_K_POSMAX);/* 2776-3000 */
 TS(026u); ai_adf(s,CM_FRESH_K_MINUS2); TS(027u); ai_adf(s,CM_FRESH_K_NEGONE);/* 3001-3004 */
 TS(030u);                                                  /* 3005 */
L3006: /* STARTSB2 */
 ai_caf(s,CM_FRESH_K_OCT77603); CH_WAND(011u); ai_caf(s,CM_FRESH_K_OCT74777);/* 3006-3011 */
 CH_WAND(013u); ai_csf(s,CM_FRESH_K_PRIO25); ai_mask(s,076u);/* 3012-3015 */
 ai_adf(s,CM_FRESH_K_SKIPVBIT); TS(076u); ai_caf(s,CM_FRESH_K_STARTEB);/* 3016-3020 */
 TS(03u); ai_caf(s,CM_FRESH_K_NEG1_2);                      /* 3021-3022 */
 TS(01407u); TS(01406u); TS(01405u); TS(01404u); TS(01403u);/* 3023-3027 */
 TS(01402u); TS(01401u); TS(01400u); ai_csf(s,CM_FRESH_K_ENDTASK);/* 3030-3033 */
 TS(01410u); TS(01412u); TS(01414u); TS(01416u); TS(01420u);/* 3034-3040 */
 TS(01422u); TS(01424u); TS(01426u); TS(01430u); ai_csf(s,CM_FRESH_K_ENDTASK_P1);/* 3041-3045 */
 TS(01411u); TS(01413u); TS(01415u); TS(01417u); TS(01421u);/* 3046-3052 */
 TS(01423u); TS(01425u); TS(01427u); TS(01431u); ai_csf(s,CM_FRESH_K_ZERO);/* 3053-3057 */
 TS(0167u); TS(0203u); TS(0217u); TS(0233u); TS(0247u); TS(0263u);/* 3060-3065 */
 TS(0277u); TS(01302u); TS(067u); ai_caf(s,CM_FRESH_K_VAC1ADRC);/* 3066-3071 */
 TS(0400u); ai_adf(s,CM_FRESH_K_LTHVACA); TS(0454u); ai_adf(s,CM_FRESH_K_LTHVACA);/* 3072-3075 */
 TS(0530u); ai_adf(s,CM_FRESH_K_LTHVACA); TS(0604u); ai_adf(s,CM_FRESH_K_LTHVACA);/* 3076-3101 */
 TS(0660u); ai_caf(s,CM_FRESH_K_TEN);                       /* 3102-3103 */
L3104: /* DSPOFF */
 TS(0154u);ai_csf(s,CM_FRESH_K_BIT12);                      /* 3104-3105 */
 ins=indexed(055023u,agc_extend(ai_word(s,0154u)));         /* 3106-3107 INDEX MPAC; TS DSPTAB */
 if((ins&076000u)!=054000u)return EXEC_OUT_OF_SCOPE;
 TS(ins&01777u);
 c=ai_ccs(s,0154u);                                         /* 3110 */
 if(c==0u)goto L3104;                                       /* 3111 TCF DSPOFF */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* MPAC counts down from TEN */
 TS(01141u); TS(01142u); TS(01143u); TS(01144u); TS(01073u);/* 3112-3116 */
 TS(045u); TS(0776u); TS(01042u); TS(01013u); TS(01015u);   /* 3117-3123 */
 TS(01012u); TS(01020u); TS(01021u); TS(01001u); TS(01002u);/* 3124-3130 */
 TS(01043u); TS(01330u); TS(01322u); TS(01323u); TS(01324u);/* 3131-3135 */
 TS(01325u); TS(01304u); TS(0100u); ai_caf(s,CM_FRESH_K_NOUTCON);/* 3136-3141 */
 TS(01016u); ai_caf(s,CM_FRESH_K_BIT14); ai_mask(s,01044u); TS(01044u);/* 3142-3145 */
 ai_caf(s,CM_FRESH_K_LESCHK); TS(01361u); ai_csf(s,CM_FRESH_K_VD1);/* 3146-3150 */
 TS(0777u);                                                 /* 3151 */
 goto retq;                                                 /* 3152 TC Q */
L3153: /* T5IDLOC: T5RUPT comes here when nothing uses TIME5 */
 ai_ca(s,L);W[5]=NOQRSM_P1;return EXEC_OK;                  /* 3153-3154 TCF NOQRSM +1 */
L4106: /* GOTOPOOH */
 OUT(04107u,PHASCHNG);                                      /* OCT 14 */
L4110:
 OUT(04111u,POSTJUMP);                                      /* CADR GOP00FIX */
retq: /* TC Q: the word in Q executes as TC. A caller in this bank returns
       * to one of its return points; MR.KLEAN, P00KLEAN and V37KLEAN under
       * BANKCALL return through SWRETURN, or to another fixed-fixed word. */
 at=(agc_word)(agc_correct(W[Q])&077777u);ai_tc(s,3u);
 if(at>=04000u&&at<010000u){W[5]=at;return EXEC_OK;}
 switch(at){case 02344u:case 02425u:case 02550u:case 02575u:case 02653u:case 02654u:case 02775u:goto jump;
 default:return EXEC_OUT_OF_SCOPE;}
}

const uint16_t cm_fresh_entries[CM_FS_ROUTINES]={04106u,04110u,02342u,02501u,02505u,02513u,02526u,02535u,02631u,
 02635u,02651u,02667u,02707u,02717u,02735u,02750u,03153u};
#define E05(name,z) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io){return bank05(p,s,io,(z));}
E05(gotopooh,04106u) E05(phaschng_ret,04110u) E05(slap1,02342u) E05(mrklean,02501u) E05(p00klean,02505u)
E05(v37klean,02513u) E05(goprog,02526u) E05(bankcall_ret,02535u) E05(ibnkcall_ret,02631u) E05(nocoarse,02635u)
E05(enema,02651u) E05(goprog3,02667u) E05(mmdsplay_ret,02707u) E05(stoprate_ret,02717u) E05(swcall_ret,02735u)
E05(alarm_ret,02750u) E05(t5idloc,03153u)
static const agc_routine_entry entries[CM_FS_ROUTINES]={
 {FF(04106),gotopooh},{FF(04110),phaschng_ret},
 {B05(02342),slap1},{B05(02501),mrklean},{B05(02505),p00klean},{B05(02513),v37klean},{B05(02526),goprog},
 {B05(02535),bankcall_ret},{B05(02631),ibnkcall_ret},{B05(02635),nocoarse},{B05(02651),enema},{B05(02667),goprog3},
 {B05(02707),mmdsplay_ret},{B05(02717),stoprate_ret},{B05(02735),swcall_ret},{B05(02750),alarm_ret},{B05(03153),t5idloc}};
const agc_routine_table cm_fresh_start_routines={entries,CM_FS_ROUTINES};
