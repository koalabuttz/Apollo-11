/* SPDX-License-Identifier: MIT */
#include "lm_t4rupt_imu.h"
#include "agc_image_ops.h"
#define W s->words
/* Erasable. */
#define TIME1 025u
#define CDUZ 034u
#define GYROCMD 047u
#define CDUXCMD 050u
#define CDUYCMD 051u
#define CDUZCMD 052u
#define ITEMP6 066u
#define RUPTREG1 070u
#define RUPTREG2 071u
#define FLAGWRD0 074u
#define FLAGWRD2 076u
#define FLAGWRD5 0101u
#define RADMODES 0110u
#define PIPADT 01075u                   /* 1/PIPADT */
#define DSPTAB11 01036u                 /* DSPTAB +11D */
#define IMODES30 01302u
#define IMODES33 01303u
/* Fixed constants (tools/lm_t4rupt_imu_source_map.py checks each against the rope). */
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
#define BIT14 020000u
#define BIT15 040000u
#define ONE 01u
#define SIX 06u
#define POSMAX 037777u
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
#define BITS6_15 040040u
#define MBIT10 076777u                  /* -BIT10 */
#define OCT37737 037737u
#define PRIO30 030000u
#define K90SECS 021450u
#define K4SECS 0620u
#define RRINIT 0102u
#define M70DEGS 063434u
#define M15DEGS 075252u
/* The descending power table from BIT15 (04735): INDEX RUPTREG1; CAF BIT14 or BIT13. */
static agc_word bit_after(unsigned top,unsigned r){return (agc_word)(1u<<(top-1u-r));}
/* Outside the file. */
#define ALARM 05567u
#define WAITLIST 05203u
#define VARDELAY 05224u
#define TASKOVER 05261u
#define POSTJUMP 04635u
#define IBNKCALL 04674u
#define ZEROICDU 05457u
#define VARALARM 05744u
#define RESUME 05270u                   /* GLOCKOK EQUALS RESUME */
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

/* LAMPTEST from a TC (Q its return): CS IMODES33; MASK BIT1; CCS A; INCR Q;
 * TC Q. Returns where it goes: Q, or Q + 1 when no lamp test is running. */
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
  case 02172u:goto IMUMON;case 02256u:goto L2256;case 02263u:goto C33TEST;case 02266u:goto ENDTNON;
  case 02313u:goto UNZ2;case 02314u:goto L2314;case 02321u:goto ISSUP;case 02346u:return tcf(s,TASKOVER);
  case 02363u:goto L2363;case 02367u:goto L2367;case 02373u:goto C33TEST;case 02456u:goto L2456;
  case 02462u:goto NOGIMRUN;case 02555u:goto NXTIFAIL;case 02611u:goto L2611;case 02631u:goto L2631;
  case 02642u:goto NXTIFAIL;case 02670u:case 02675u:case 02702u:goto NXTFL33;
  case 02703u:goto SETISSW_EXT;case 02731u:goto L2731;default:return EXEC_OUT_OF_SCOPE;}

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
 ai_tc(s,02225u);                        /* INDEX RUPTREG1; TC IFAILJMP */
 switch(ndx(r)){case 0u:goto ITURNON;case 1u:case 2u:goto SETISSW;case 3u:goto IMUCAGE;case 5u:goto IMUOP;default:return EXEC_OUT_OF_SCOPE;}
NXTIFAIL:
 c=ai_ccs(s,RUPTREG2);
 if(c==0u)goto NXTIFBIT_M1;
 if(c==2u)goto L2230;
 if(c==3u)goto L2231;
TNONTEST:
 ai_cs(s,IMODES30);
L2230:
 ai_maskf(s,BIT7);
L2231:
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
 if(c==1u)return tc(s,02254u,ALARM);     /* TC ALARM; OCT 213 */
L2256:                                   /* +3 (a masked word is never negative) */
 ai_tc(s,02257u);goto CAGESUB;
L2257:
 ai_caf(s,K90SECS);return tc(s,02260u,WAITLIST);   /* 2CADR ENDTNON */
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
 ai_tc(s,02432u);                        /* INDEX RUPTREG1; TC C33JMP */
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
 if(c==1u)return tc(s,02454u,IBNKCALL);  /* CADR SETCOARS (RAND of a bit is never negative) */
L2456:
 ai_caf(s,SIX);return tc(s,02457u,WAITLIST);       /* 2CADR CA+ECE */
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
 ai_tc(s,02510u);
 if(lamptest(s)==02511u)goto GLINVERT;
 return tcf(s,RESUME);
TLIM:
 ai_maskf(s,POSMAX);(void)ai_ts(s,RUPTREG2);
 c=ai_ccs(s,IMODES30);
 if(c<2u)goto TEMPOK;
 if(c==2u)ai_caf(s,BIT4);
 wor(s,&h->channel11);goto NXTIFAIL;    /* WOR DSALMOUT */
TEMPOK:
 ai_tc(s,02526u);
 if(lamptest(s)==02526u)goto NXTIFAIL;
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
 return tc(s,02553u,ALARM);              /* TC ALARM; OCT 207 */
ITURNON2:
 ai_cs(s,IMODES30);ai_maskf(s,BIT7);ai_ads(s,IMODES30);ai_caf(s,RRINIT);(void)ai_ts(s,RADMODES);
 goto NXTIFAIL;
IMUCAGE:
 if(ai_ccs(s,0u)==0u)goto ISSZERO;      /* A is a masked bit: + or +0 */
 ai_csf(s,OCT77000);wand(s,&h->channel14);ai_csf(s,OCT272);wand(s,&h->channel12);
 ai_csf(s,BIT7);ai_mask(s,FLAGWRD5);(void)ai_ts(s,FLAGWRD5);   /* CS ENGONBIT */
 ai_csf(s,PRIO30);rand_(s,h->channel11);ai_adf(s,BIT14);h->channel11=a15(s);   /* WRITE DSALMOUT */
 ai_tc(s,02607u);goto CAGESUB1;
L2607:
 return tc(s,02607u,IBNKCALL);           /* CADR RNDREFDR */
L2611:
 ai_csf(s,0u);(void)ai_ts(s,CDUXCMD);(void)ai_ts(s,CDUYCMD);(void)ai_ts(s,CDUZCMD);(void)ai_ts(s,GYROCMD);
 ai_csf(s,OCT740);wand(s,&h->channel14);goto NXTIFAIL;
IMUOP:
 if(ai_bzf(s))goto IMUOP2;
 ai_cs(s,IMODES33);ai_maskf(s,BIT6);ai_ads(s,IMODES33);
 return tc(s,02627u,IBNKCALL);           /* CADR RNDREFDR */
L2631:
 ai_csf(s,BITS7_8);ai_mask(s,FLAGWRD0);ai_xch(s,FLAGWRD0);ai_com(s);ai_maskf(s,BIT8);   /* IMUSEFLG */
 if(ai_ccs(s,0u)==0u)goto NXTIFAIL;
 return tc(s,02640u,ALARM);              /* TC ALARM; OCT 214 */
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
 ai_tc(s,02656u);goto SETISSW;
L2656:
 ai_cs(s,IMODES30);ai_maskf(s,BIT1);
 c=ai_ccs(s,0u);
 if(c==0u)goto NXTFL33;
 if(c==1u)ai_ca(s,IMODES30);
 if(c<=2u)ai_maskf(s,OCT1720);
 c=ai_ccs(s,0u);
 if(c==1u)return tc(s,02666u,ALARM);     /* TC ALARM; OCT 212 */
 goto NXTFL33;
DNTMFAST:
 if(ai_ccs(s,0u)==0u)goto NXTFL33;
 return tc(s,02673u,ALARM);              /* TC ALARM; OCT 1105 */
UPTMFAST:
 if(ai_ccs(s,0u)==0u)goto NXTFL33;
 return tc(s,02700u,ALARM);              /* TC ALARM; OCT 1106 */
SETISSW:                                 /* from IFAILJMP (Q NXTIFAIL), ISSUP or PIPFAIL */
 if(setissw(s,h))return tc(s,02730u,VARALARM);
 r=W[2];W[2]=3u;                         /* TC Q */
 if(r==02225u)goto NXTIFAIL;
 if(r==02337u)goto L2337;
 goto L2656;
SETISSW_EXT:                             /* from IMU_MODE_SWITCHING_ROUTINES */
 if(setissw(s,h))return tc(s,02730u,VARALARM);
 return tc_q(s);
L2731:                                   /* VARALARM's return */
 ai_caf(s,BIT1);wor(s,&h->channel11);
 r=W[ITEMP6];W[5]=r;W[2]=067u;           /* TC ITEMP6 runs ITEMP6's word as TC */
 if(r==02225u)goto NXTIFAIL;
 if(r==02337u)goto L2337;
 if(r==02656u)goto L2656;
 return EXEC_OK;
ENDTNON:
 ai_csf(s,BIT2);ai_mask(s,IMODES30);ai_xch(s,IMODES30);ai_maskf(s,BIT2);
 if(ai_bzf(s))goto ENDTNON2;
 ai_caf(s,BIT14);ai_mask(s,IMODES30);
 if(ai_bzf(s)){ai_caf(s,K90SECS);return tc(s,02265u,VARDELAY);}   /* RETNON */
 ai_cs(s,FLAGWRD0);ai_maskf(s,BIT8);    /* IMUSEBIT */
 if(ai_ccs(s,0u)==0u)return tcf(s,TASKOVER);
 return tc(s,02304u,POSTJUMP);           /* CADR IMUBAD */
ENDTNON2:
 ai_caf(s,BIT15);wor(s,&h->channel12);
 return tc(s,02311u,IBNKCALL);           /* CADR NOATTOFF */
UNZ2:
 return tc(s,02313u,ZEROICDU);
L2314:
 ai_csf(s,BITS4_5);wand(s,&h->channel12);ai_caf(s,BIT11);
 return tc(s,02320u,VARDELAY);
ISSUP:
 ai_csf(s,OCT54);ai_mask(s,IMODES30);(void)ai_ts(s,IMODES30);
 ai_csf(s,BIT6);ai_mask(s,IMODES33);(void)ai_ts(s,IMODES33);
 ai_cs(s,FLAGWRD2);ai_maskf(s,BIT15);   /* DRFTBIT */
 if(!ai_bzf(s)){ai_ads(s,FLAGWRD2);ai_ca(s,TIME1);ai_xch(s,PIPADT);}
 ai_tc(s,02337u);goto SETISSW;
L2337:
 ai_csf(s,BIT15);wand(s,&h->channel12);ai_caf(s,K4SECS);
 return tc(s,02343u,WAITLIST);           /* 2CADR PFAILOK */
OPONLY:
 ai_caf(s,BIT4);rand_(s,h->channel12);
 c=ai_ccs(s,0u);
 if(c==0u)goto C33TEST;
 if(c==1u)ai_caf(s,BIT8);                /* IMUSEBIT */
 if(c<=2u)ai_mask(s,FLAGWRD0);
 if(ai_ccs(s,0u)==0u)goto C33TEST;
 ai_tc(s,02361u);cagesub2(s);W[2]=3u;    /* TC CAGESUB2 ... TC Q */
ISSZERO:
 return tc(s,02361u,IBNKCALL);           /* CADR NOATTOFF */
L2363:
 ai_caf(s,BIT5);wor(s,&h->channel12);
 return tc(s,02366u,ZEROICDU);
L2367:
 ai_caf(s,BIT6);return tc(s,02370u,WAITLIST);      /* 2CADR UNZ2 */
CAGESUB:
 ai_csf(s,BITS6_15);wand(s,&h->channel12);ai_caf(s,BITS4_5);wor(s,&h->channel12);
CAGESUB1:
 ai_cs(s,DSPTAB11);ai_maskf(s,OC40010);ai_ads(s,DSPTAB11);
 cagesub2(s);
 r=W[2];W[2]=3u;                         /* TC Q */
 if(r==02257u)goto L2257;
 goto L2607;
}

static enum exec_status entry(const agc_native_program *p,apollo_overlay_state *s,void *io){agc_hardware *h;agc_word z,t;(void)io;
 if(!p||!s||!(h=p->hardware)||W[4]!=FB06)return EXEC_OUT_OF_SCOPE;
 z=W[5];
 if((z==02555u||z==02611u||z==02631u||z==02642u)&&!scan_ok(s,30u))return EXEC_OUT_OF_SCOPE;
 if((z==02670u||z==02675u||z==02702u)&&!scan_ok(s,33u))return EXEC_OUT_OF_SCOPE;
 if(z==02611u&&(h->channel14&077000u))return EXEC_OUT_OF_SCOPE;   /* reached only after IMUCAGE's WAND CHAN14 */
 if(z==02731u){t=W[ITEMP6];
  if(t<02000u||t>07777u||(t==02225u&&!scan_ok(s,30u))||(t==02656u&&!scan_ok(s,33u)))return EXEC_OUT_OF_SCOPE;}
 return imu(s,h,z);
}
#define B6(z) (uint16_t)(06u*1024u+((z)-02000u))
static const agc_routine_entry entries[LM_T4RUPT_IMU_ENTRIES]={
 {B6(02172u),entry},{B6(02256u),entry},{B6(02263u),entry},{B6(02266u),entry},{B6(02313u),entry},{B6(02314u),entry},
 {B6(02321u),entry},{B6(02346u),entry},{B6(02363u),entry},{B6(02367u),entry},{B6(02373u),entry},{B6(02456u),entry},
 {B6(02462u),entry},{B6(02555u),entry},{B6(02611u),entry},{B6(02631u),entry},{B6(02642u),entry},{B6(02670u),entry},
 {B6(02675u),entry},{B6(02702u),entry},{B6(02703u),entry},{B6(02731u),entry}};
const agc_routine_table lm_t4rupt_imu_routines={entries,LM_T4RUPT_IMU_ENTRIES};
