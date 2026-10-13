/* SPDX-License-Identifier: MIT */
/* AGC_BLOCK_TWO_SELF-CHECK (bank 43), shared by src/lm_self_check.c and
 * src/cm_self_check.c: the two missions assemble the same source, Luminary099
 * two words lower (43,3232-3723) than Comanche055 (43,3234-3725). Labels are
 * named by their Comanche055 address; K(z) is the mission's address of the
 * Comanche055 word z. The includer defines SC_D (0 or 2), SCK(name) (its
 * compiled constants), SC_MISSION (0 or 1, for include/rope_image.h), SC_IO
 * (its device record: p66_bias_io or cm_native_io), the outside entries
 * SC_ALARM2, SC_CHANG1, SC_LEFT5, SC_SUPDACAL, SC_POSTJUMP, SC_SDISPLAY and
 * SC_ENDSUMS, its operand table SC_OPERANDS (SC_OPERAND_COUNT words), and
 * the names SC_TABLE, SC_ENTRIES, SC_SITE_TABLE and SC_ROPE_DATA.
 *
 * One agc_image_ops call per original instruction. Every TC out of the file
 * leaves through OUT; the routine resumes at the return point as its own
 * entry. TC Q (-1CHK, ADSUM) and TC SFAIL / TC SKEEP1 execute the word in
 * that register as TC, which leaves Q at the register +1; the word must be a
 * TC to an address this file continues at, or out of the file. FXADRS
 * (INDEX SKEEP3; CA 0, ROPECHK's read of the fixed-fixed rope) reads the
 * rope image (include/rope_image.h), which SC_ROPE_DATA also serves to
 * SUPDACAL as the program's fixed_data hook. */
#include "agc_image_ops.h"
#include "rope_image.h"

#include "agc_image_w.h"
#define K(z) ((agc_word)((z)-SC_D))
#define A 0u
#define L 1u
#define Q 2u
#define EBANK 3u
#define CYR 020u
#define SR 021u
#define CYL 022u
#define EDOP 023u
#define NEWJOB 067u
#define SFAIL 01357u
#define ERESTORE 01360u
#define SELFRET 01361u
#define SMODE 01362u
#define ALMCADR 01363u
#define ERCOUNT 01365u
#define SCOUNT 01366u
#define SKEEP1 01371u
#define SKEEP2 01372u
#define SKEEP3 01373u
#define SKEEP4 01374u
#define SKEEP5 01375u
#define SKEEP6 01376u
#define SKEEP7 01377u
#define B43 (033u<<10)

static SC_IO *ctx(const apollo_overlay_state *s,void *io){
 SC_IO *x=io;
 if(s==0||x==0||x->interrupt_enabled>1u||(x->superbank&~0160u)||!(x->superbank&0100u)||W[7]!=0)return 0;
 if((W[4]&076000u)!=B43)return 0;
 return x;
}
#define TS(a) do{if(ai_ts(s,(a)))return EXEC_OUT_OF_SCOPE;}while(0)
/* TC to an entry outside the file: Q = the word after the TC. */
#define OUT(ret,target) do{ai_tc(s,(ret));W[5]=(target);return EXEC_OK;}while(0)
/* TC to a word of the file: Q = the word after the TC. */
#define TCL(ret,label) do{ai_tc(s,K(ret));goto label;}while(0)
/* The erasable address an INDEX register names for a two-word access: an
 * erasable word whose pair stays in erasable memory, at or above 061 after
 * the EB window (the program's domain: ERASCHK and the ERESTORE repair). */
static int pair_ok(const apollo_overlay_state *s,agc_word x){
 return x<01777u&&ai_ea(s,x)>=061u&&ai_ea(s,(unsigned)x+1u)>=061u;
}
/* An INDEX value (A or an erasable word, overflow-corrected): yaAGC adds it
 * in ones' complement, so -0 indexes as +0. */
static agc_word index_value(agc_word v){v=(agc_word)agc_correct(v);return v==077777u?0u:v;}

static enum exec_status body(const agc_native_program *p,apollo_overlay_state *s,void *io,agc_word at){
 SC_IO *x=ctx(s,io);agc_word w;uint16_t physical;(void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
jump:
 /* The words a TC Q or a TC through SFAIL/SKEEP1 can continue at: the
  * entries, the returns of TC -1CHK and TC ADSUM, ERASCHK and ROPECHK. */
 switch(at){
 case K(03270u):goto L3270; case K(03306u):goto L3306; case K(03340u):goto L3340; case K(03341u):goto L3341;
 case K(03413u):goto L3413; case K(03430u):goto L3430; case K(03443u):goto L3443; case K(03512u):goto L3512;
 case K(03520u):goto L3520; case K(03522u):goto L3522; case K(03524u):goto L3524; case K(03542u):goto L3542;
 case K(03543u):goto L3543; case K(03564u):goto L3564; case K(03565u):goto L3565; case K(03625u):goto L3625;
 case K(03627u):goto L3627; case K(03635u):goto L3635; case K(03675u):goto L3675; case K(03713u):goto L3713;
 case K(03724u):goto L3724;
 default:break;}
 /* Any other word of the file is not a continuation; outside it, the run goes on there. */
 if(at>=K(03234u)&&at<=K(03725u))return EXEC_OUT_OF_SCOPE;
 W[5]=at;return EXEC_OK;
/* TC to the word held in erasable register r (TC Q, TC SFAIL, TC SKEEP1):
 * it runs as an instruction, which must be a TC to switched or fixed-fixed
 * memory; Q is left at r +1. */
#define TCREG(r) do{w=(agc_word)(W[(r)]&077777u);if(w<02000u||w>=010000u)return EXEC_OUT_OF_SCOPE;ai_tc(s,(agc_word)((r)+1u));at=w;goto jump;}while(0)

 /* PRERRORS: restore the two words ERASCHK was testing, if a restart left them. */
L3250: ai_ca(s,ERESTORE);
 if(ai_bzf(s))goto L3261;                                  /* 3251-3252 EXTEND; BZF ERRORS */
 ai_dca(s,SKEEP5);                                         /* 3253-3254 */
 w=W[SKEEP7];if(!pair_ok(s,w))return EXEC_OUT_OF_SCOPE;
 ai_dxch(s,w);                                             /* 3255-3256 INDEX SKEEP7; DXCH 0 */
 ai_caf(s,SCK(SPZERO)); TS(ERESTORE);                      /* 3257-3260 */
L3261: x->interrupt_enabled=0;                            /* ERRORS INHINT */
 ai_ca(s,Q); TS(SFAIL); TS(ALMCADR); ai_incr(s,ERCOUNT);   /* 3262-3265 */
 OUT(K(03267u),SC_ALARM2);                                 /* 3266 TC ALARM2; 3267 OCT 01102 */
L3270: switch(ai_ccs(s,SMODE)){case 0:goto L3271;case 1:goto L3272;case 2:goto L3273;default:goto L3274;}
L3271: ai_caf(s,SCK(SPZERO));                              /* SIDLOOP */
L3272: TS(SMODE);
L3273: TCL(03274u,L3340);                                  /* TC SELFCHK */
L3274: TCREG(SFAIL);                                       /* TC SFAIL */
 /* -1CHK: A must be -1; otherwise PRERRORS. */
L3275: switch(ai_ccs(s,A)){case 0:case 1:case 3:goto L3250;default:break;}
 switch(ai_ccs(s,A)){case 0:goto L3250;case 1:TCREG(Q);default:return EXEC_OUT_OF_SCOPE;}  /* 3300-3302; |A|-1 is never negative */
 /* SMODECHK: save the caller in SKEEP1, let the Executive run a new job, then
  * act on SMODE. */
L3303: ai_qxch(s,SKEEP1);                                  /* 3303-3304 EXTEND; QXCH SKEEP1 */
L3305: TCL(03306u,L3334);                                  /* TC CHECKNJ */
L3306: switch(ai_ccs(s,SMODE)){
 case 0:TCL(03310u,L3314);                                 /* 3307 TC SOPTIONS */
 case 1:TCL(03311u,L3305);                                 /* 3310 TC SMODECHK +2 */
 case 2:TCL(03312u,L3314);                                 /* 3311 TC SOPTIONS */
 default:break;}
 ai_incr(s,SCOUNT);                                        /* 3312 */
 TCREG(SKEEP1);                                            /* 3313 TC SKEEP1 */
L3314: ai_adf(s,SCK(SM7));                                 /* SOPTIONS */
 if(!ai_bzmf(s))TCL(03320u,L3271);                         /* 3315-3316 EXTEND; BZMF +2; 3317 BNKOPTN TC SIDLOOP */
 ai_incr(s,SCOUNT); ai_adf(s,SCK(SP7));                    /* 3320-3321 */
 w=index_value(W[A]);if(w>7u)return EXEC_OUT_OF_SCOPE;    /* 3322 INDEX A */
 ai_tc(s,K(03324u));                                       /* 3323 TC SOPTION1 */
 switch(w){                                                /* SOPTION1-SOPTON10, each a TC */
 case 3u:TCL(03330u,L3341);                                /* SOPTION4 TC ERASCHK */
 case 4u:TCL(03331u,L3522);                                /* SOPTION5 TC ROPECHK */
 default:ai_tc(s,(agc_word)(K(03325u)+w));TCREG(SKEEP1);}  /* the others TC SKEEP1 */
 /* CHECKNJ: SELFRET = the return; POSTJUMP ADVAN (the Executive comes back
  * through SELFRET if no new job is waiting). */
L3334: ai_qxch(s,SELFRET);                                 /* 3334-3335 EXTEND; QXCH SELFRET */
 OUT(K(03337u),SC_POSTJUMP);                               /* 3336 TC POSTJUMP; 3337 CADR ADVAN */
L3340: TCL(03341u,L3303);                                  /* SELFCHK TC SMODECHK */
 /* ERASCHK: every erasable word of every bank, two at a time. */
L3341: ai_caf(s,SCK(SP1)); TS(SKEEP2);
 ai_caf(s,SCK(SPZERO)); TS(EBANK);                         /* 3343-3344 0EBANK */
 ai_caf(s,SCK(ERASCON3)); TS(SKEEP7); ai_caf(s,SCK(S10BITS)); TS(SKEEP3);
 TCL(03352u,L3371);                                        /* 3351 TC ERASLOOP */
L3352: ai_caf(s,SCK(ERASCON6)); TS(SKEEP7); ai_caf(s,SCK(S10BITS)); TS(SKEEP3);  /* E134567B */
 TCL(03357u,L3371);
L3357: ai_caf(s,SCK(ERASCON6)); TS(SKEEP7); ai_caf(s,SCK(ERASCON4)); TS(SKEEP3);  /* 2EBANK */
 TCL(03364u,L3371);
L3364: TS(SKEEP2);                                         /* NOEBANK */
 ai_caf(s,SCK(ERASCON1)); TS(SKEEP7); ai_caf(s,SCK(ERASCON2)); TS(SKEEP3);
L3371: x->interrupt_enabled=0;                            /* ERASLOOP INHINT */
 ai_ca(s,EBANK); TS(SKEEP4);                               /* 3372-3373 */
 w=W[SKEEP7];if(!pair_ok(s,w))return EXEC_OUT_OF_SCOPE;
 ai_dca(s,w);                                              /* 3374-3376 EXTEND; NDX SKEEP7; DCA 0 */
 ai_dxch(s,SKEEP5);                                        /* 3377 */
 ai_ca(s,SKEEP7); TS(ERESTORE); TS(L); ai_incr(s,L);       /* 3400-3403 */
 w=index_value(W[A]);if(!pair_ok(s,w))return EXEC_OUT_OF_SCOPE;
 ai_dxch(s,w);                                             /* 3404-3405 NDX A; DXCH 0: each word holds its own address */
 w=W[SKEEP7];if(w>=01777u)return EXEC_OUT_OF_SCOPE;
 ai_cs(s,(unsigned)w+1u); ai_ad(s,w);                      /* 3406-3411 NDX SKEEP7; CS 1; NDX SKEEP7; AD 0 */
 TCL(03413u,L3275);                                        /* 3412 TC -1CHK */
L3413: ai_ca(s,ERESTORE);
 if(ai_bzf(s))goto L3441;                                  /* 3414-3415 */
 w=W[SKEEP7];if(!pair_ok(s,w))return EXEC_OUT_OF_SCOPE;
 ai_dcs(s,w); ai_dxch(s,w);                                /* 3416-3422 complement of the addresses */
 w=W[SKEEP7];if(w>=01777u)return EXEC_OUT_OF_SCOPE;
 ai_cs(s,w); ai_ad(s,(unsigned)w+1u);                      /* 3423-3426 NDX SKEEP7; CS 0; NDX SKEEP7; AD 1 */
 TCL(03430u,L3275);                                        /* 3427 TC -1CHK */
L3430: ai_ca(s,ERESTORE);
 if(ai_bzf(s))goto L3441;                                  /* 3431-3432 */
 ai_dca(s,SKEEP5);                                         /* 3433-3434 */
 w=W[SKEEP7];if(!pair_ok(s,w))return EXEC_OUT_OF_SCOPE;
 ai_dxch(s,w);                                             /* 3435-3436 put the words back */
 ai_caf(s,SCK(SPZERO)); TS(ERESTORE);                      /* 3437-3440 */
L3441: x->interrupt_enabled=1;                            /* ELOOPFIN RELINT */
 TCL(03443u,L3334);                                        /* 3442 TC CHECKNJ */
L3443: ai_ca(s,SKEEP4); TS(EBANK); ai_incr(s,SKEEP7);      /* 3443-3445 */
 ai_cs(s,SKEEP7); ai_ad(s,SKEEP3);                         /* 3446-3447 */
 if(!ai_bzf(s))TCL(03453u,L3371);                          /* 3450-3451 EXTEND; BZF +2; 3452 TC ERASLOOP */
 switch(ai_ccs(s,SKEEP2)){                                 /* 3453 */
 case 0:TCL(03455u,L3364);                                 /* 3454 TC NOEBANK */
 case 1:goto L3455;case 2:goto L3456;default:goto L3457;}
L3455: ai_incr(s,SKEEP2);
L3456: ai_ca(s,EBANK);
L3457: ai_adf(s,SCK(SBIT9)); TS(EBANK);                    /* 3457-3460 next EBANK */
 ai_adf(s,SCK(ERASCON5));                                  /* 3461 */
 if(ai_bzf(s))goto L3357;                                  /* 3462-3463 EXTEND; BZF 2EBANK */
 switch(ai_ccs(s,EBANK)){                                  /* 3464 */
 case 0:TCL(03466u,L3352);                                 /* 3465 TC E134567B */
 case 1:goto L3466;case 2:goto L3467;default:goto L3470;}
L3466: ai_caf(s,SCK(ERASCON6));
L3467: TS(EBANK);
 /* CNTRCHK: read every counter, 060 down to 010. */
L3470: ai_caf(s,SCK(CNTRCON));
L3471: TS(SKEEP2);                                         /* CNTRLOOP */
 ai_adf(s,SCK(SBIT4));                                     /* 3472 */
 w=index_value(W[A]);if(w<3u||w>01777u)return EXEC_OUT_OF_SCOPE;
 ai_cs(s,w);                                               /* 3473-3474 INDEX A; CS 0 */
 switch(ai_ccs(s,SKEEP2)){                                 /* 3475 */
 case 0:TCL(03477u,L3471);                                 /* 3476 TC CNTRLOOP */
 case 1:goto L3477;case 2:goto L3500;default:goto L3501;}
 /* CYCLSHFT: the edited registers. */
L3477: ai_caf(s,SCK(CONCPS1));
L3500: TS(CYR);
L3501: TS(CYL); TS(SR); TS(EDOP);                          /* 3501-3503 */
 ai_ad(s,CYR); ai_ad(s,CYL); ai_ad(s,SR); ai_ad(s,EDOP); ai_adf(s,SCK(CONCPS2));  /* 3504-3510 */
 TCL(03512u,L3275);                                        /* 3511 TC -1CHK */
L3512: ai_ad(s,CYR); ai_ad(s,CYL); ai_ad(s,SR); ai_ad(s,EDOP); ai_adf(s,SCK(SP1));  /* 3512-3516 */
 TCL(03520u,L3275);                                        /* 3517 TC -1CHK */
L3520: ai_incr(s,SCOUNT+1u);
 TCL(03522u,L3303);                                        /* 3521 TC SMODECHK */
 /* ROPECHK (SKEEP6 -0) and STSHOSUM (show-banksum, SKEEP6 as SHOWSUM sets
  * it): sum every word of every bank. */
L3522: ai_caf(s,SCK(SMZERO)); TS(SKEEP6);
L3524: ai_caf(s,SCK(SPZERO)); TS(SKEEP4);                  /* STSHOSUM */
 ai_caf(s,SCK(SP1));                                       /* 3526 */
L3527: TS(SKEEP7);                                         /* COMMFX */
 ai_caf(s,SCK(SPZERO)); TS(SKEEP1); TS(SKEEP3); ai_caf(s,SCK(SP1)); TS(SKEEP5);  /* 3530-3534 */
L3535: ai_ca(s,SKEEP4); TS(L); ai_maskf(s,SCK(HI5)); ai_ad(s,SKEEP3);  /* COMADRS 3535-3540 */
 OUT(K(03542u),SC_SUPDACAL);                               /* 3541 TC SUPDACAL */
L3542: TCL(03543u,L3565);                                  /* TC ADSUM */
L3543: ai_adf(s,SCK(SBIT11));
 TCL(03545u,L3576);                                        /* 3544 TC ADRSCHK */
L3545: ai_cs(s,A); TS(SKEEP7);                             /* FXFX */
 if(ai_bzf(s))goto L3553;                                  /* 3547-3550 EXTEND; BZF +3 */
 ai_caf(s,SCK(SBIT12));                                    /* 3551 */
 TCL(03553u,L3554);                                        /* 3552 TC +2 */
L3553: ai_caf(s,SCK(SBNK03));
L3554: TS(SKEEP3); ai_caf(s,SCK(SPZERO)); TS(SKEEP1); ai_caf(s,SCK(SP1)); TS(SKEEP5);  /* 3554-3560 */
L3561: w=index_value(W[SKEEP3]);if(w>=010000u)return EXEC_OUT_OF_SCOPE;  /* FXADRS INDEX SKEEP3 */
 if(w<02000u){if(ai_ea(s,w)==5u)W[A]=K(03563u);else ai_ca(s,w);}  /* 3562 CA 0 (Z reads as the next word) */
 else{agc_word word;physical=agc_fixed_physical(w,W[4],(agc_word)x->superbank);
  if(physical==AGC_TRANSFER_NONE||!apollo_rope_read(SC_MISSION,physical,&word))return EXEC_OUT_OF_SCOPE;
  W[A]=agc_extend(word);}
 TCL(03564u,L3565);                                        /* 3563 TC ADSUM */
L3564: TCL(03565u,L3576);                                  /* TC ADRSCHK (after FXADRS's TC ADSUM) */
L3565: if(ai_ts(s,SKEEP2))goto L3567;                      /* ADSUM TS SKEEP2 */
 ai_ad(s,SKEEP1);                                          /* 3566 */
L3567: if(ai_ts(s,SKEEP1))goto L3571;                      /* an overflow skips the CAF, adding the carry */
 ai_caf(s,SCK(SPZERO));                                    /* 3570 */
L3571: ai_ad(s,SKEEP1);
 if(ai_ts(s,SKEEP1))goto L3574;                            /* 3572 */
 ai_cs(s,SKEEP2);                                          /* 3573 */
L3574: ai_ad(s,SKEEP3);
 TCREG(Q);                                                 /* 3575 TC Q */
L3576: ai_lxch(s,A);                                       /* ADRSCHK */
 ai_ca(s,SKEEP3); ai_maskf(s,SCK(LOW10)); ai_adf(s,SCK(MMAXADRS));  /* 3577-3601 */
 if(ai_bzf(s))goto L3672;                                  /* 3602-3603 EXTEND; BZF SOPTION */
 switch(ai_ccs(s,SKEEP5)){                                 /* 3604 */
 case 0:TCL(03606u,L3610);                                 /* 3605 TC +3 */
 case 1:TCL(03607u,L3610);                                 /* 3606 TC +2 */
 case 2:TCL(03610u,L3672);                                 /* 3607 TC SOPTION */
 default:break;}
L3610: switch(ai_ccs(s,L)){
 case 0:TCL(03612u,L3620);case 1:TCL(03613u,L3620);case 2:TCL(03614u,L3620);  /* 3611-3613 TC CONTINU */
 default:break;}
 switch(ai_ccs(s,SKEEP5)){                                 /* 3614 */
 case 0:TCL(03616u,L3621);                                 /* 3615 TC CONTINU +1 */
 case 1:goto L3616;
 case 2:TCL(03620u,L3621);                                 /* 3617 TC CONTINU +1 */
 default:goto L3620;}
L3616: ai_caf(s,SCK(SM1));
 TCL(03620u,L3621);                                        /* 3617 */
L3620: ai_caf(s,SCK(SP1));                                 /* CONTINU */
L3621: TS(SKEEP5);
 switch(ai_ccs(s,SKEEP6)){                                 /* 3622 */
 case 0:break;
 case 1:goto L3624;
 case 2:TCL(03626u,L3627);                                 /* 3625 TC +2 */
 default:TCL(03627u,L3334);}                               /* 3626 TC CHECKNJ */
 switch(ai_ccs(s,NEWJOB)){                                 /* 3623 */
 case 0:goto L3624;
 case 1:TCL(03626u,L3627);                                 /* 3625 TC +2 */
 case 2:TCL(03627u,L3334);                                 /* 3626 TC CHECKNJ */
 default:goto L3627;}
L3624: OUT(K(03625u),SC_CHANG1);                           /* TC CHANG1 */
L3625: TCL(03626u,L3627);                                  /* TC +2 */
L3627: ai_incr(s,SKEEP3);                                  /* ADRS+1 */
 switch(ai_ccs(s,SKEEP7)){                                 /* 3630 */
 case 0:TCL(03632u,L3535);case 1:TCL(03633u,L3535);        /* 3631-3632 TC COMADRS */
 case 2:TCL(03634u,L3561);default:TCL(03635u,L3561);}      /* 3633-3634 TC FXADRS */
L3635: ai_cs(s,SKEEP4); ai_adf(s,SCK(LSTBNKCH));           /* NXTBNK */
 if(ai_bzf(s)){W[5]=SC_ENDSUMS;return EXEC_OK;}            /* 3637-3640 EXTEND; BZF ENDSUMS */
 ai_ca(s,SKEEP4); ai_adf(s,SCK(SBIT11));                   /* 3641-3642 */
 if(ai_ts(s,SKEEP4))goto L3645;                            /* 3643: banks 37 to 40 overflow */
 TCL(03645u,L3650);                                        /* 3644 TC CHKSUPR */
L3645: ai_caf(s,SCK(SBIT15)); ai_ads(s,SKEEP4);            /* 17TO20 */
 TCL(03650u,L3664);                                        /* 3647 TC GONXTBNK */
L3650: ai_maskf(s,SCK(HI5));                               /* CHKSUPR */
 if(ai_bzf(s))goto L3662;                                  /* 3651-3652 EXTEND; BZF NXTSUPR */
 ai_adf(s,SCK(S13BITS));                                   /* 3653 27TO30 */
 if(!ai_bzf(s))TCL(03657u,L3664);                          /* 3654-3655 EXTEND; BZF +2; 3656 TC GONXTBNK */
 ai_caf(s,SCK(SIXTY)); ai_ads(s,SKEEP4);                   /* 3657-3660 */
 TCL(03662u,L3664);                                        /* 3661 TC GONXTBNK */
L3662: ai_adf(s,SCK(SUPRCON)); ai_ads(s,SKEEP4);           /* NXTSUPR */
L3664: switch(ai_ccs(s,SKEEP7)){                           /* GONXTBNK */
 case 0:TCL(03666u,L3527);                                 /* 3665 TC COMMFX */
 case 1:ai_caf(s,SCK(SP1));TCL(03670u,L3545);              /* 3666-3667 CA S+1; TC FXFX */
 case 2:TCL(03670u,L3545);                                 /* 3667 TC FXFX */
 default:break;}
 ai_caf(s,SCK(SBIT7));                                     /* 3670 */
 TCL(03672u,L3527);                                        /* 3671 TC COMMFX */
L3672: ai_ca(s,SKEEP4); ai_maskf(s,SCK(HI5));              /* SOPTION */
 OUT(K(03675u),SC_LEFT5);                                  /* 3674 TC LEFT5 */
L3675: TS(L); ai_ca(s,SKEEP4); ai_maskf(s,SCK(S8BITS));    /* 3675-3677 */
 if(!ai_bzf(s)){                                           /* 3700-3701 EXTEND; BZF SOPT */
  TS(SR); ai_ca(s,L); ai_maskf(s,SCK(SEVEN)); ai_ad(s,SR); TS(L);}  /* 3702-3706 superbank banks */
 ai_ca(s,SKEEP6);                                          /* 3707 SOPT */
 if(!ai_bzf(s))OUT(K(03713u),SC_SDISPLAY);                 /* 3710-3711 EXTEND; BZF +2; 3712 TC SDISPLAY */
L3713: switch(ai_ccs(s,SKEEP1)){
 case 0:TCL(03715u,L3716);                                 /* 3714 TC +2 */
 case 1:TCL(03716u,L3717);                                 /* 3715 TC +2 */
 case 2:goto L3716;default:goto L3717;}
L3716: ai_adf(s,SCK(SP1));
L3717: if(ai_ts(s,SKEEP1))goto L3721;
 ai_cs(s,L);                                               /* 3720 BNKCHK */
L3721: ai_ad(s,SKEEP1); ai_adf(s,SCK(SM1));                /* 3721-3722 */
 TCL(03724u,L3275);                                        /* 3723 TC -1CHK */
L3724: TCL(03725u,L3635);                                  /* TC NXTBNK */
#undef TCREG
}

/* The compiled operands: the CADR after TC POSTJUMP (CHECKNJ) and the constants. */
const agc_site_operand *SC_SITE_TABLE(uint16_t *count){if(count){*count=SC_OPERAND_COUNT;}
 return SC_OPERANDS;}
/* SUPDACAL's read of the word ROPECHK addresses: the program's fixed_data hook. */
bool SC_ROPE_DATA(const agc_native_program *p,uint16_t physical,agc_word *word){(void)p;return apollo_rope_read(SC_MISSION,physical,word);}

#define SC_ENTRY(z) static enum exec_status e##z(const agc_native_program *p,apollo_overlay_state *s,void *io){return body(p,s,io,K(0##z##u));}
SC_ENTRY(3270) SC_ENTRY(3306) SC_ENTRY(3340) SC_ENTRY(3443) SC_ENTRY(3524) SC_ENTRY(3542)
SC_ENTRY(3625) SC_ENTRY(3627) SC_ENTRY(3635) SC_ENTRY(3675) SC_ENTRY(3713)
#define P43(z) (uint16_t)(043u*1024u+(K(z)-02000u))
const uint16_t SC_ENTRIES[SELF_CHECK_ROUTINES]={K(03270u),K(03306u),K(03340u),K(03443u),K(03524u),K(03542u),
 K(03625u),K(03627u),K(03635u),K(03675u),K(03713u)};
static const agc_routine_entry entries[SELF_CHECK_ROUTINES]={
 {P43(03270u),e3270},{P43(03306u),e3306},{P43(03340u),e3340},{P43(03443u),e3443},{P43(03524u),e3524},{P43(03542u),e3542},
 {P43(03625u),e3625},{P43(03627u),e3627},{P43(03635u),e3635},{P43(03675u),e3675},{P43(03713u),e3713}};
const agc_routine_table SC_TABLE={entries,SELF_CHECK_ROUTINES};
#undef W
