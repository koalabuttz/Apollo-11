/* SPDX-License-Identifier: MIT */
#include "cm_t4rupt_imu.h"
#include "agc_image_ops.h"
#include "agc_image_w.h"
/* Erasable. */
#define CDUZ 034u
#define GYROCMD 047u
#define CDUXCMD 050u
#define CDUYCMD 051u
#define CDUZCMD 052u
#define ITEMP6 066u
#define RUPTREG1 070u
#define RUPTREG2 071u
#define STATE 074u
#define DSPTAB11 01036u                 /* DSPTAB +11D */
#define IMODES30 01320u
#define IMODES33 01321u
/* Fixed constants (tools/cm_t4rupt_imu_source_map.py checks each against the rope). */
#define BIT1 01u
#define BIT2 02u
#define BIT4 010u
#define BIT5 020u
#define BIT6 040u
#define BIT7 0100u
#define BIT8 0200u
#define BIT9 0400u
#define BIT10 01000u
#define BIT11 02000u
#define BIT13 010000u
#define BIT14 020000u
#define BIT15 040000u
#define ONE 01u
#define SIX 06u
#define POSMAX 037777u
#define IMUSEFLG 0200u
#define K30RDMSK 076400u
#define K33RDMSK 016000u                /* PRIO16 */
#define OCT15 015u
#define OCT15000 015000u                /* PRIO15 */
#define OC40010 040010u
#define OCT54 054u
#define OCT75 075u
#define OCT272 0272u
#define BITS7_8 0300u
#define BITS4_5 030u
#define OCT1720 01720u
#define OCT740 0740u
#define OCT77000 077000u
#define BIT15_6 040040u
#define MBIT10 076777u                  /* -BIT10 */
#define OCT37737 037737u
#define K90SECS 021450u
#define K4SECS 0620u
#define M70DEGS 063434u
#define M15DEGS 075252u
/* The descending power table from BIT15 (05001): INDEX RUPTREG1; CAF BIT14 or BIT13. */
static agc_word bit_after(unsigned top,unsigned r){return (agc_word)(1u<<(top-1u-r));}
/* Outside the file. */
#define ALARM 05650u
#define WAITLIST 05245u
#define VARDELAY 05272u
#define TASKOVER 05324u
#define POSTJUMP 04701u
#define IBNKCALL 04740u
#define ZEROICDU 05521u
#define VARALARM 05764u
#define RESUME 05333u                   /* GLOCKOK EQUALS RESUME */
#define FB06 014000u

static agc_word a15(const apollo_overlay_state *s){return agc_correct(W[0]);}
static agc_word lchan(const apollo_overlay_state *s){return agc_correct(W[1]);}
static void rxor(apollo_overlay_state *s,agc_word v){W[0]=agc_extend((agc_word)(a15(s)^v));}
static void rand_(apollo_overlay_state *s,agc_word v){W[0]=agc_extend((agc_word)(a15(s)&v));}
static void ror(apollo_overlay_state *s,agc_word v){W[0]=agc_extend((agc_word)(a15(s)|v));}
static void wor(apollo_overlay_state *s,agc_word *c){*c=(agc_word)(a15(s)|*c);W[0]=agc_extend(*c);}
static void wand(apollo_overlay_state *s,agc_word *c){*c=(agc_word)(a15(s)&*c);W[0]=agc_extend(*c);}
/* WAND CHAN33: A gets A AND the latched inputs; the write resets bits 11-15 to 1. */
static void wand33(apollo_overlay_state *s,agc_word *c){W[0]=agc_extend((agc_word)(a15(s)&*c));*c=(agc_word)(*c|076000u);}
static enum exec_status tc(apollo_overlay_state *s,agc_word at,agc_word target){ai_tc(s,(agc_word)(at+1u));W[5]=target;return EXEC_OK;}
static enum exec_status tcf(apollo_overlay_state *s,agc_word target){W[5]=target;return EXEC_OK;}
/* TC Q: yaAGC runs Q's word as TC, so Q ends at 3. */
static enum exec_status tc_q(apollo_overlay_state *s){W[5]=W[2];W[2]=3u;return EXEC_OK;}
static unsigned ndx(agc_word v){return v==077777u?0u:v;}

/* The bit scans (NXTIFBIT, NXTIBT) index their dispatch tables by RUPTREG1;
 * a continuation entered with RUPTREG1 and RUPTREG2 must name only bits that
 * select a TCF of IFAILJMP (0-3, 5) or C33JMP (0-2), as IMUMON and C33TEST
 * leave them. */
static int scan_ok(const apollo_overlay_state *s,unsigned table){agc_word r=W[RUPTREG1],d=W[RUPTREG2];int base,i;unsigned p;
 if(d&040000u)return 0;
 base=r==077776u?-1:r==077777u?0:r<=6u?(int)r:-100;
 if(base<-1)return 0;
 for(p=1u;p<=14u;++p)if(d&(1u<<(p-1u))){i=base+(int)(15u-p);
  if(table==30u?(i<0||i>5||i==4):(i<0||i>2))return 0;}
 return 1;
}

/* LAMPTEST from a TC (Q its return): CS IMODES33; MASK BIT1; TCF ZOPFIN3
 * (CCS A; INCR Q; TC Q). Returns where it goes: Q, or Q + 1 when no lamp
 * test is running. */
static agc_word lamptest(apollo_overlay_state *s){agc_word z;
 ai_cs(s,IMODES33);ai_maskf(s,BIT1);
 if(ai_ccs(s,0u)==0u)ai_incr(s,2u);
 z=W[2];W[2]=3u;return z;
}
/* CAGESUB2 up to its TC Q. */
static void cagesub2(apollo_overlay_state *s){
 ai_cs(s,IMODES30);ai_maskf(s,OCT75);ai_ads(s,IMODES30);ai_cs(s,IMODES33);ai_maskf(s,BIT6);ai_ads(s,IMODES33);
}
/* SETISSW up to ISSWOFF's TC Q (returns 0) or ISSWON's QXCH ITEMP6 (returns 1;
 * TC VARALARM follows). */
static int setissw(apollo_overlay_state *s,agc_hardware *h){unsigned c;
 ai_caf(s,OCT15);ai_mask(s,IMODES30);ai_mpf(s,BIT10);ai_ca(s,IMODES30);ror(s,lchan(s));ai_com(s);ai_maskf(s,OCT15000);
 if(ai_ccs(s,0u)==0u){ai_qxch(s,ITEMP6);return 1;}  /* ISSWON (the masked word is never negative) */
 ai_caf(s,BIT1);ai_mask(s,IMODES33);                 /* ISSWOFF */
 c=ai_ccs(s,0u);
 if(c==1u){ai_csf(s,BIT1);wand(s,&h->channel11);}
 return 0;
}

static enum exec_status imu(apollo_overlay_state *s,agc_hardware *h,agc_word z){unsigned c;agc_word r;
 switch(z){
  case 02174u:goto IMUMON;case 02260u:goto L2260;case 02265u:goto C33TEST;case 02270u:goto ENDTNON;
  case 02315u:goto UNZ2;case 02316u:goto L2316;case 02323u:goto ISSUP;case 02341u:return tcf(s,TASKOVER);
  case 02356u:goto L2356;case 02362u:goto L2362;case 02366u:goto C33TEST;case 02451u:goto L2451;
  case 02455u:goto NOGIMRUN;case 02550u:goto NXTIFAIL;case 02573u:goto L2573;case 02613u:goto L2613;
  case 02624u:goto NXTIFAIL;case 02652u:case 02657u:case 02664u:goto NXTFL33;
  case 02665u:goto SETISSW_EXT;case 02713u:goto L2713;default:return EXEC_OUT_OF_SCOPE;}

IMUMON:
 ai_ca(s,IMODES30);rxor(s,h->channel30);ai_maskf(s,K30RDMSK);
 if(ai_bzf(s))goto TNONTEST;
 (void)ai_ts(s,RUPTREG1);ai_lxch(s,IMODES30);rxor(s,lchan(s));(void)ai_ts(s,IMODES30);
 ai_csf(s,ONE);ai_xch(s,RUPTREG1);
 if(ai_bzmf(s))goto TLIM;
 goto NXTIFBIT;
NXTIFBIT_M1:
 ai_adf(s,ONE);
NXTIFBIT:
 ai_incr(s,RUPTREG1);
 ai_double(s);
 if(!ai_ts(s,0u))goto NXTIFBIT;          /* TS A skips on overflow */
 ai_xch(s,RUPTREG2);
 r=W[RUPTREG1];ai_caf(s,bit_after(14u,ndx(r)));ai_mask(s,IMODES30);
 ai_tc(s,02227u);                        /* INDEX RUPTREG1; TC IFAILJMP */
 switch(ndx(r)){case 0u:goto ITURNON;case 1u:case 2u:goto SETISSW;case 3u:goto IMUCAGE;case 5u:goto IMUOP;default:return EXEC_OUT_OF_SCOPE;}
NXTIFAIL:
 c=ai_ccs(s,RUPTREG2);
 if(c==0u)goto NXTIFBIT_M1;
 if(c==2u)goto L2232;
 if(c==3u)goto L2233;
TNONTEST:
 ai_cs(s,IMODES30);
L2232:
 ai_maskf(s,BIT7);
L2233:
 c=ai_ccs(s,0u);
 if(c==0u)goto C33TEST;
 if(c==1u)ai_caf(s,BIT8);
 if(c<=2u)ai_mask(s,IMODES30);
 c=ai_ccs(s,0u);
 if(c==0u)goto PROCTNON;
 if(c==1u)ai_caf(s,BIT8);
 if(c<=2u)ai_ads(s,IMODES30);
 goto C33TEST;
PROCTNON:
 ai_csf(s,BITS7_8);ai_mask(s,IMODES30);(void)ai_ts(s,IMODES30);ai_maskf(s,BIT14);
 c=ai_ccs(s,0u);
 if(c==0u)goto OPONLY;
 if(c==1u)ai_cs(s,IMODES30);
 if(c<=2u)ai_maskf(s,BIT9);
 c=ai_ccs(s,0u);
 if(c==1u)return tc(s,02256u,ALARM);     /* TC ALARM; OCT 213 */
L2260:                                   /* +3 (a masked word is never negative) */
 ai_tc(s,02261u);goto CAGESUB;
L2261:
 ai_caf(s,K90SECS);return tc(s,02262u,WAITLIST);   /* 2CADR ENDTNON */
C33TEST:
 ai_ca(s,IMODES33);ai_maskf(s,K33RDMSK);(void)ai_ts(s,1u);ai_caf(s,K33RDMSK);wand33(s,&h->channel33);rxor(s,lchan(s));
 if(ai_bzf(s))goto GLOCKMON;
 (void)ai_ts(s,RUPTREG1);ai_lxch(s,IMODES33);rxor(s,lchan(s));(void)ai_ts(s,IMODES33);
 ai_caf(s,0u);ai_xch(s,RUPTREG1);ai_double(s);
 goto NXTIBT_P1;
NXTIBT_M1:
 ai_adf(s,ONE);
NXTIBT:
 ai_incr(s,RUPTREG1);
NXTIBT_P1:
 ai_double(s);
 if(!ai_ts(s,0u))goto NXTIBT;
 ai_xch(s,RUPTREG2);
 r=W[RUPTREG1];ai_caf(s,bit_after(13u,ndx(r)));ai_mask(s,IMODES33);
 ai_tc(s,02425u);                        /* INDEX RUPTREG1; TC C33JMP */
 switch(ndx(r)){case 0u:goto PIPFAIL;case 1u:goto DNTMFAST;case 2u:goto UPTMFAST;default:return EXEC_OUT_OF_SCOPE;}
NXTFL33:
 c=ai_ccs(s,RUPTREG2);
 if(c==0u)goto NXTIBT_M1;
 if(c==2u)goto GLOCKCHK;                 /* TCF GLOCKCHK at GLOCKMON +1 */
 if(c==3u)goto SETGLOCK;                 /* TCF SETGLOCK at GLOCKMON +2 */
GLOCKMON:
 c=ai_ccs(s,CDUZ);
 if(c==1u||c==3u)goto SETGLOCK;
GLOCKCHK:
 ai_adf(s,M70DEGS);
 if(ai_bzmf(s))goto SETGLOCK_M1;
 ai_adf(s,M15DEGS);
 if(ai_bzmf(s))goto NOGIMRUN;
 ai_caf(s,BIT4);rand_(s,h->channel12);
 c=ai_ccs(s,0u);
 if(c==0u)goto NOGIMRUN;
 if(c==1u)return tc(s,02447u,IBNKCALL);  /* CADR SETCOARS (RAND of a bit is never negative) */
L2451:
 ai_caf(s,SIX);return tc(s,02452u,WAITLIST);       /* 2CADR CA+ECE */
NOGIMRUN:
 ai_caf(s,BIT6);goto SETGLOCK;
SETGLOCK_M1:
 ai_caf(s,0u);
SETGLOCK:
 ai_ad(s,DSPTAB11);ai_maskf(s,BIT6);
 if(ai_bzf(s))return tcf(s,RESUME);     /* GLOCKOK */
 ai_mask(s,DSPTAB11);
 c=ai_ccs(s,0u);
 if(c==0u)goto GLAMPTST;
 if(c==1u)ai_caf(s,BIT6);
 if(c<=2u)ai_mask(s,IMODES30);
 c=ai_ccs(s,0u);
 if(c==0u)return tcf(s,RESUME);
GLINVERT:
 ai_cs(s,DSPTAB11);ai_maskf(s,BIT6);ai_adf(s,BIT15);ai_xch(s,DSPTAB11);ai_maskf(s,OCT37737);ai_ads(s,DSPTAB11);
 return tcf(s,RESUME);
GLAMPTST:
 ai_tc(s,02503u);
 if(lamptest(s)==02504u)goto GLINVERT;
 return tcf(s,RESUME);
TLIM:
 ai_maskf(s,POSMAX);(void)ai_ts(s,RUPTREG2);
 c=ai_ccs(s,IMODES30);
 if(c<2u)goto TEMPOK;
 if(c==2u)ai_caf(s,BIT4);
 wor(s,&h->channel11);goto NXTIFAIL;    /* WOR DSALMOUT */
TEMPOK:
 ai_tc(s,02521u);
 if(lamptest(s)==02521u)goto NXTIFAIL;
 ai_csf(s,BIT4);wand(s,&h->channel11);goto NXTIFAIL;
ITURNON:
 ai_caf(s,BIT2);ai_mask(s,IMODES30);
 c=ai_ccs(s,0u);
 if(c==0u)goto NXTIFAIL;
 if(c==1u)ai_caf(s,BIT14);
 if(c<=2u)ai_mask(s,IMODES30);
 if(ai_bzf(s))goto ITURNON2;
 ai_caf(s,BIT15);rand_(s,h->channel12);
 if(!ai_bzf(s))goto NXTIFAIL;
 ai_caf(s,BIT2);ai_ads(s,IMODES30);
 return tc(s,02546u,ALARM);              /* TC ALARM; OCT 207 */
ITURNON2:
 ai_cs(s,IMODES30);ai_maskf(s,BIT7);ai_ads(s,IMODES30);
 goto NXTIFAIL;
IMUCAGE:
 if(ai_ccs(s,0u)==0u)goto ISSZERO;      /* A is a masked bit: + or +0 */
 ai_csf(s,OCT77000);wand(s,&h->channel14);ai_csf(s,OCT272);wand(s,&h->channel12);
 ai_csf(s,BIT13);wand(s,&h->channel11); /* WAND DSALMOUT: engine off */
 ai_tc(s,02571u);goto CAGESUB1;
L2571:
 return tc(s,02571u,IBNKCALL);           /* CADR RNDREFDR */
L2573:
 ai_csf(s,0u);(void)ai_ts(s,CDUXCMD);(void)ai_ts(s,CDUYCMD);(void)ai_ts(s,CDUZCMD);(void)ai_ts(s,GYROCMD);
 ai_csf(s,OCT740);wand(s,&h->channel14);goto NXTIFAIL;
IMUOP:
 if(ai_bzf(s))goto IMUOP2;
 ai_cs(s,IMODES33);ai_maskf(s,BIT6);ai_ads(s,IMODES33);
 return tc(s,02611u,IBNKCALL);           /* CADR RNDREFDR */
L2613:
 ai_csf(s,BITS7_8);ai_mask(s,STATE);ai_xch(s,STATE);ai_com(s);ai_maskf(s,IMUSEFLG);
 if(ai_ccs(s,0u)==0u)goto NXTIFAIL;
 return tc(s,02622u,ALARM);              /* TC ALARM; OCT 214 */
IMUOP2:
 ai_caf(s,BIT2);ai_mask(s,IMODES30);
 if(ai_ccs(s,0u)==0u)goto NXTIFAIL;
 goto ITURNON2;
PIPFAIL:
 c=ai_ccs(s,0u);
 if(c==0u)ai_caf(s,BIT10);
 if(c<=1u)ai_xch(s,IMODES30);
 if(c<=2u)ai_maskf(s,MBIT10);
 ai_ads(s,IMODES30);
 ai_tc(s,02640u);goto SETISSW;
L2640:
 ai_cs(s,IMODES30);ai_maskf(s,BIT1);
 c=ai_ccs(s,0u);
 if(c==0u)goto NXTFL33;
 if(c==1u)ai_ca(s,IMODES30);
 if(c<=2u)ai_maskf(s,OCT1720);
 c=ai_ccs(s,0u);
 if(c==1u)return tc(s,02650u,ALARM);     /* TC ALARM; OCT 212 */
 goto NXTFL33;
DNTMFAST:
 if(ai_ccs(s,0u)==0u)goto NXTFL33;
 return tc(s,02655u,ALARM);              /* TC ALARM; OCT 1105 */
UPTMFAST:
 if(ai_ccs(s,0u)==0u)goto NXTFL33;
 return tc(s,02662u,ALARM);              /* TC ALARM; OCT 1106 */
SETISSW:                                 /* from IFAILJMP (Q NXTIFAIL), ISSUP or PIPFAIL */
 if(setissw(s,h))return tc(s,02712u,VARALARM);
 r=W[2];W[2]=3u;                         /* TC Q */
 if(r==02227u)goto NXTIFAIL;
 if(r==02332u)goto L2332;
 goto L2640;
SETISSW_EXT:                             /* from IMU_MODE_SWITCHING_ROUTINES */
 if(setissw(s,h))return tc(s,02712u,VARALARM);
 return tc_q(s);
L2713:                                   /* VARALARM's return */
 ai_caf(s,BIT1);wor(s,&h->channel11);
 r=W[ITEMP6];W[5]=r;W[2]=067u;           /* TC ITEMP6 runs ITEMP6's word as TC */
 if(r==02227u)goto NXTIFAIL;
 if(r==02332u)goto L2332;
 if(r==02640u)goto L2640;
 return EXEC_OK;
ENDTNON:
 ai_csf(s,BIT2);ai_mask(s,IMODES30);ai_xch(s,IMODES30);ai_maskf(s,BIT2);
 if(ai_bzf(s))goto ENDTNON2;
 ai_caf(s,BIT14);ai_mask(s,IMODES30);
 if(ai_bzf(s)){ai_caf(s,K90SECS);return tc(s,02267u,VARDELAY);}   /* RETNON */
 ai_cs(s,STATE);ai_maskf(s,IMUSEFLG);
 if(ai_ccs(s,0u)==0u)return tcf(s,TASKOVER);
 return tc(s,02306u,POSTJUMP);           /* CADR IMUBAD */
ENDTNON2:
 ai_caf(s,BIT15);wor(s,&h->channel12);
 return tc(s,02313u,IBNKCALL);           /* CADR NOATTOFF */
UNZ2:
 return tc(s,02315u,ZEROICDU);
L2316:
 ai_csf(s,BITS4_5);wand(s,&h->channel12);ai_caf(s,BIT11);
 return tc(s,02322u,VARDELAY);
ISSUP:
 ai_csf(s,OCT54);ai_mask(s,IMODES30);(void)ai_ts(s,IMODES30);
 ai_csf(s,BIT6);ai_mask(s,IMODES33);(void)ai_ts(s,IMODES33);
 ai_tc(s,02332u);goto SETISSW;
L2332:
 ai_csf(s,BIT15);wand(s,&h->channel12);ai_caf(s,K4SECS);
 return tc(s,02336u,WAITLIST);           /* 2CADR PFAILOK */
OPONLY:
 ai_caf(s,BIT4);rand_(s,h->channel12);
 c=ai_ccs(s,0u);
 if(c==0u)goto C33TEST;
 if(c==1u)ai_caf(s,IMUSEFLG);
 if(c<=2u)ai_mask(s,STATE);
 if(ai_ccs(s,0u)==0u)goto C33TEST;
 ai_tc(s,02354u);cagesub2(s);W[2]=3u;    /* TC CAGESUB2 ... TC Q */
ISSZERO:
 return tc(s,02354u,IBNKCALL);           /* CADR NOATTOFF */
L2356:
 ai_caf(s,BIT5);wor(s,&h->channel12);
 return tc(s,02361u,ZEROICDU);
L2362:
 ai_caf(s,BIT6);return tc(s,02363u,WAITLIST);      /* 2CADR UNZ2 */
CAGESUB:
 ai_csf(s,BIT15_6);wand(s,&h->channel12);ai_caf(s,BITS4_5);wor(s,&h->channel12);
CAGESUB1:
 ai_cs(s,DSPTAB11);ai_maskf(s,OC40010);ai_ads(s,DSPTAB11);
 cagesub2(s);
 r=W[2];W[2]=3u;                         /* TC Q */
 if(r==02261u)goto L2261;
 goto L2571;
}

static enum exec_status entry(const agc_native_program *p,apollo_overlay_state *s,void *io){agc_hardware *h;agc_word z,t;(void)io;
 if(!p||!s||!(h=p->hardware)||W[4]!=FB06)return EXEC_OUT_OF_SCOPE;
 z=W[5];
 if((z==02550u||z==02573u||z==02613u||z==02624u)&&!scan_ok(s,30u))return EXEC_OUT_OF_SCOPE;
 if((z==02652u||z==02657u||z==02664u)&&!scan_ok(s,33u))return EXEC_OUT_OF_SCOPE;
 if(z==02573u&&(h->channel14&077000u))return EXEC_OUT_OF_SCOPE;   /* reached only after IMUCAGE's WAND CHAN14 */
 if(z==02713u){t=W[ITEMP6];
  if(t<02000u||t>07777u||(t==02227u&&!scan_ok(s,30u))||(t==02640u&&!scan_ok(s,33u)))return EXEC_OUT_OF_SCOPE;}
 return imu(s,h,z);
}
#define B6(z) (uint16_t)(06u*1024u+((z)-02000u))
static const agc_routine_entry entries[CM_T4RUPT_IMU_ENTRIES]={
 {B6(02174u),entry},{B6(02260u),entry},{B6(02265u),entry},{B6(02270u),entry},{B6(02315u),entry},{B6(02316u),entry},
 {B6(02323u),entry},{B6(02341u),entry},{B6(02356u),entry},{B6(02362u),entry},{B6(02366u),entry},{B6(02451u),entry},
 {B6(02455u),entry},{B6(02550u),entry},{B6(02573u),entry},{B6(02613u),entry},{B6(02624u),entry},{B6(02652u),entry},
 {B6(02657u),entry},{B6(02664u),entry},{B6(02665u),entry},{B6(02713u),entry}};
const agc_routine_table cm_t4rupt_imu_routines={entries,CM_T4RUPT_IMU_ENTRIES};
