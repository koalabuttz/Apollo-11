/* SPDX-License-Identifier: MIT
 * Luminary099 Q_R-AXIS_RCS_AUTOPILOT.agc, all of it (bank 17, 02072-03251,
 * and MAKERUPT at 07754-07755), as routines of the control-transfer ABI.
 * Each source instruction is one agc_image_ops call (yaAGC-qualified
 * register effects) or, for MSU, DTCB, EDRUPT and the channel orders, a
 * helper here with yaAGC's effect, named with its erasable address, channel
 * or the source's fixed constant. TS overflow skips and CCS exits land where
 * the original's do, including the CCS exits that enter a word past its
 * EXTEND or a data word. */
#include "lm_q_r_axis.h"
#include "agc_image_ops.h"
/* Every instruction runs on the one canonical image through
 * include/agc_fast_redirect.h: a plain-cell operand under this file's
 * EBANK 6 goes to the fast helper (include/agc_fast_ops.h), any other
 * operand to a one-argument wrapper of the agc_image_ops helper; both have
 * its effects. -DLM_QR_GENERIC builds the same source on agc_image_ops alone. */
#ifdef LM_QR_GENERIC
#define AFR_GENERIC
#endif
#define AFR_EBN (LM_QR_EB>>8)
#include "agc_fast_redirect.h"
#define W AFR_S(s)->words
/* Branch coverage hook for the host test (empty in flight builds). */
#ifdef LM_QR_COUNT_PATHS
extern unsigned lm_qr_paths[];
#define LM_QR_PATH(n) ((void)++lm_qr_paths[n])
#else
#define LM_QR_PATH(n) ((void)0)
#endif
#define P17(z) (uint16_t)(017u*1024u+((z)-02000u))
/* Erasable: central and interrupt registers, counters, unswitched, and
 * EBANK6 (EBANK= CDUXD). */
#define LREG 1u
#define QREG 2u
#define ZEROREG 7u
#define ARUPT 010u
#define ZRUPT 015u
#define BRUPT 017u
#define TIME6 031u
#define CDUY 033u
#define CDUZ 034u
#define FLAGWRD5 0101u
#define FLGWRD10 0106u
#define DAPBOOLS 0111u
#define CH5MASK 01262u
#define RCSFLAGS 01273u
#define DELQEROR 01300u
#define DELREROR 01301u
#define M21 01415u
#define M31 01416u
#define M22 01417u
#define M32 01420u
#define OMEGAQ 01422u
#define OMEGAR 01423u
#define OMEGAU 01426u        /* = URATEDIF */
#define URATEDIF 01426u
#define VRATEDIF 01427u      /* = OMEGAV = EDOT */
#define EDOT 01427u
#define EDOTQ 01436u         /* = QRATEDIF */
#define QRATEDIF 01436u
#define RRATEDIF 01437u
#define CH31TEMP 01443u
#define STIKSENS 01444u
#define DYERROR 01450u       /* = QERROR */
#define QERROR 01450u
#define DYERROR1 01451u
#define DZERROR 01452u       /* = RERROR */
#define RERROR 01452u
#define DZERROR1 01453u
#define QLAST 01455u
#define RLAST 01456u
#define TCQR 01457u
#define OLDQRMIN 01461u
#define SAVEHAND 01462u
#define SAVEHAND1 01463u
#define NXT6ADR 01465u
#define T6NEXT 01466u
#define T6NEXT1 01467u
#define T6FURTHA 01470u
#define T6FURTHA1 01471u
#define NEXTU 01473u
#define NEXTV 01474u
#define TARGETDB 01476u      /* = -RATEDB */
#define MINUSRATEDB 01476u
#define RETJADR 01477u
#define SENSETYP 01500u
#define ALLOWGTS 01502u
#define AXISCTR 01505u
#define NOUJETS 01522u       /* NO.UJETS */
#define TJU 01525u
#define TJV 01526u
#define SKIPU 01535u
#define SKIPV 01536u
#define ONEANET2P1 01571u    /* 1/ANET2 +1 */
#define COEFFQ 01627u
#define COEFFR 01630u
#define COTROLER 01631u
#define INGTS 01633u
#define CDUYD 01636u
#define CDUZD 01637u
#define DAPTEMP1 01737u      /* = TEMP31 = ABSTJ = ROTEMP1 */
#define TEMP31 01737u
#define ABSTJ 01737u
#define ROTEMP1 01737u
#define DAPTEMP2 01740u      /* = ROTEMP2 */
#define ROTEMP2 01740u
#define DAPTEMP3 01741u      /* = POLYTEMP */
#define POLYTEMP 01741u
#define DAPTEMP4 01742u
#define NUMBERT 01743u
#define ROTINDEX 01744u
#define UERROR 01751u
#define EREG 01752u          /* E */
#define DAPARUPT 01753u
#define DAPBQRPT 01755u
#define DAPZRUPT 01757u
/* Fixed constants (rope words): BIT1 = ONE 04753, BIT2 = TWO 04752,
 * FOUR 04751, THREE 06245, BIT5 04747, BIT6 = ULLAGER 04746, BIT7 04745,
 * BIT8 = DRIFTBIT 04744, BIT9 04743, AORBTRAN 04742, ACC4OR2X = QRBIT
 * 04741, OURRCBIT 04740, BIT13 = CSMDOCKD = APSFLBIT = SNUFFBIT 04737,
 * USEQRJTS 04736, BIT15 = PULSES = NEGMAX 04735, ZERO 04755, NEG0 04754,
 * NEG3 07745, 00314OCT 05774, LIMITS 04734, AXISDIFF 17,3722 (TJET_LAW);
 * this file's 60MS, MINADR, OCT63, 40CYCL, LINRAT, HANDADR, GTSCADR,
 * INDEXES, +TJMINT6 (14MS), -150MS, TJLAWADR, ALLJETS, TYPEPOLY and
 * ADRRUPT. */
#define ONE 1u
#define BIT1 1u
#define TWO 2u
#define BIT2 2u
#define THREE 3u
#define FOUR 4u
#define BIT5 020u
#define BIT6 040u
#define ULLAGER 040u
#define BIT7 0100u
#define BIT8 0200u
#define DRIFTBIT 0200u
#define BIT9 0400u
#define AORBTRAN 01000u
#define ACC4OR2X 02000u
#define QRBIT 02000u
#define OURRCBIT 04000u
#define BIT13 010000u
#define CSMDOCKD 010000u
#define APSFLBIT 010000u
#define SNUFFBIT 010000u
#define USEQRJTS 020000u
#define BIT15 040000u
#define PULSES 040000u
#define NEGMAX 040000u
#define ZERO 0u
#define NEG0 077777u
#define NEG3 077774u
#define OCT00314 0314u
#define MS60 0140u
#define MINADR 02273u        /* GENADR MINRTN */
#define OCT63 063u
#define C40CYCL 050u
#define LINRAT 056u
#define HANDADR 02532u       /* GENADR BACKHAND */
#define MS14 026u            /* 14MS = +TJMINT6 */
#define MINUS150MS 077417u
#define TJLAWADR 02671u      /* GENADR TJLAW +3 */
#define ADRRUPT 03241u       /* ADRES ENDJASK */
#define SPSRCSW 0632u        /* 17,3727, read by the CADR SPSRCS word run as CS */

static int tc_word(agc_register w){return w>=02000u&&w<=07777u;}
static int entry(const agc_image *s,const void *io){AFR_BIND(s);return s&&io&&(W[4]&076000u)==LM_QR_FB17&&W[3]==LM_QR_EB;}
static agc_word oc(agc_register a){return (agc_word)((a&037777u)|((a>>1)&040000u));}
/* The signed value of an erasable word, and of the 16-bit A. */
static int sv(const agc_image *s,unsigned k){agc_word v=ai_word(s,k);return v&040000u?-(int)(~v&037777u):(int)v;}
static int sa(const agc_image *s){agc_register a=W[0];return a&0100000u?-(int)(~a&077777u):(int)(a&077777u);}
/* Channel orders on a 15-bit channel, as yaAGC's: READ, RAND, WOR. */
static void rd(agc_image *s,agc_word chan){W[0]=agc_extend(chan);}
static void rand(agc_image *s,agc_word chan){W[0]=agc_extend((agc_word)(oc(W[0])&chan));}
/* WOR CHAN13: bits 12-14 written to channel 13 are the HANDRUPT trap
 * enables, which do not read back (yaAGC keeps 043777). */
static void wor13(agc_image *s,agc_word *chan){agc_word v=(agc_word)((oc(W[0])|*chan)&077777u);*chan=(agc_word)(v&043777u);W[0]=agc_extend(v);}
/* MSU K (K above the registers): two's-complement difference of the
 * overflow-corrected A and K, returned in ones' complement. */
static void msu(agc_image *s,unsigned k){unsigned ui=oc(W[0])&077777u,uj=~(unsigned)ai_word(s,k)&077777u,d=ui+uj+1u;
 if(d&040000u){d|=0100000u;--d;}
 W[0]=agc_extend((agc_word)(d&077777u));
}
/* DTCB (DXCH Z) executed at `at`: Z,BB <-> A,L. */
static void dtcb(agc_image *s,agc_word at){agc_word z=agc_correct(W[0]),bb=agc_correct(W[1]);
 W[0]=(agc_word)(at+1u);W[1]=agc_extend(W[6]);W[5]=z;
 W[6]=(agc_word)(bb&076007u);W[3]=(agc_word)((bb&7u)<<8);W[4]=(agc_word)(bb&076000u);
}
/* A TC at ret - 1 to z, which is outside this file. */
static enum exec_status leave(agc_image *s,agc_word ret,agc_word z){ai_tc(s,ret);W[5]=z;return EXEC_OK;}
/* TC Q at the end of a subroutine: Z = Q, Q = 3. */
static void tcq(agc_image *s){ai_tc(s,QREG+1u);}
/* AXISCTR, which the file's loop holds at 0 (U) or 1 (V). */
#define AX ((unsigned)ai_word(s,AXISCTR))

/* ROT-TOUV body: LXCH ROTEMP2; EXTEND; MP COEFFQ; XCH ROTEMP2; EXTEND;
 * MP COEFFR; TS L; AD ROTEMP2; TS ROTEMP1; TCF +4; INDEX A; CS LIMITS;
 * TS ROTEMP1; CS ROTEMP2; AD L; TS 7; TCF +3; INDEX A; CS LIMITS;
 * LXCH ROTEMP1; TC Q (an overflowed sum, A +-1, selects LIMITS +-1:
 * limited to +-POSMAX). */
static void rot_touv_body(agc_image *s){static const agc_word limits[3]={037777u,057777u,040000u}; /* LIMITS -1 .. +1 */
 ai_lxch(s,ROTEMP2);ai_mp(s,COEFFQ);ai_xch(s,ROTEMP2);ai_mp(s,COEFFR);(void)ai_ts(s,LREG);ai_ad(s,ROTEMP2);
 if(ai_ts(s,ROTEMP1)){LM_QR_PATH(0);
  if(W[0u]==1u){LM_QR_PATH(165);ai_csf(s,limits[2]);}else{LM_QR_PATH(166);ai_csf(s,limits[0]);}
  (void)ai_ts(s,ROTEMP1);}
 ai_cs(s,ROTEMP2);ai_ad(s,LREG);
 if(ai_ts(s,ZEROREG)){LM_QR_PATH(1);
  if(W[0u]==1u){LM_QR_PATH(167);ai_csf(s,limits[2]);}else{LM_QR_PATH(168);ai_csf(s,limits[0]);}}
 ai_lxch(s,ROTEMP1);tcq(s);
}
/* ROT-TOUV, entered by TC from ret - 1. */
static void rot_touv(agc_image *s,agc_word ret){ai_tc(s,ret);rot_touv_body(s);}
enum {SEL_RETURN,SEL_ALARM};
/* SELCTSUB, entered by TC from ret - 1: INDEX ROTINDEX; CA ALLJETS;
 * INDEX NUMBERT; MASK TYPEPOLY; TS POLYTEMP; MASK CH5MASK; CCS A; TCF +2;
 * TC Q; CA THREE. FAILOOP: TS NUMBERT; (INDEX ROTINDEX ... MASK CH5MASK);
 * EXTEND; BZF FAILOOP -2; CCS NUMBERT; TCF FAILOOP; INDEX AXISCTR;
 * TS TJU; TC ALARM; OCT 02004 (its return: TCF NOROTAT). */
static int selctsub(agc_image *s,agc_word ret){uint8_t c;
 static const agc_word jets[9]={0110u,022u,0204u,041u, /* ALLJETS */
  0125u,0252u,0146u,0231u,0377u}; /* TYPEPOLY */
 ai_tc(s,ret);
 ai_caf(s,jets[sv(s,ROTINDEX)]);ai_maskf(s,jets[4+sv(s,NUMBERT)]);(void)ai_ts(s,POLYTEMP);ai_mask(s,CH5MASK);
 c=ai_ccs(s,0u);LM_QR_PATH(2u+c);
 if(c==1u){tcq(s);return SEL_RETURN;}
 if(c!=3u)ai_caf(s,THREE);
 for(;;){
  (void)ai_ts(s,NUMBERT);
  ai_caf(s,jets[sv(s,ROTINDEX)]);ai_maskf(s,jets[4+sv(s,NUMBERT)]);(void)ai_ts(s,POLYTEMP);ai_mask(s,CH5MASK);
  if(ai_bzf(s)){LM_QR_PATH(6);tcq(s);return SEL_RETURN;}
  c=ai_ccs(s,NUMBERT);LM_QR_PATH(7u+c);
  if(c!=0u)break;
 }
 if(c==1u)(void)ai_ts(s,TJU+AX);
 if(c==2u)(void)ai_ts(s,TJU);
 ai_tc(s,03223u);W[5]=LM_QR_ALARM;return SEL_ALARM;
}
/* QERRCALC body: CAE CDUY; EXTEND; MSU CDUYD; TS DAPTEMP1; EXTEND; MP M21;
 * TS E; CAE CDUZ; EXTEND; MSU CDUZD; TS DAPTEMP2; EXTEND; MP M22;
 * AD DELQEROR; AD E; XCH QERROR. RERRCALC: CAE DAPTEMP1; EXTEND; MP M31;
 * TS E; CAE DAPTEMP2; EXTEND; MP M32; AD DELREROR; AD E; XCH RERROR;
 * TC Q. */
static void qerrcalc_body(agc_image *s){
 ai_ca(s,CDUY);msu(s,CDUYD);(void)ai_ts(s,DAPTEMP1);ai_mp(s,M21);(void)ai_ts(s,EREG);
 ai_ca(s,CDUZ);msu(s,CDUZD);(void)ai_ts(s,DAPTEMP2);ai_mp(s,M22);ai_ad(s,DELQEROR);ai_ad(s,EREG);ai_xch(s,QERROR);
 ai_ca(s,DAPTEMP1);ai_mp(s,M31);(void)ai_ts(s,EREG);
 ai_ca(s,DAPTEMP2);ai_mp(s,M32);ai_ad(s,DELREROR);ai_ad(s,EREG);ai_xch(s,RERROR);tcq(s);
}
/* JTLST body: CS T6FURTHA; AD TIME6; EXTEND; BZMF MIDORLST; LXCH NXT6ADR;
 * DXCH T6NEXT; DXCH T6FURTHA; TS TIME6; LXCH NXT6ADR. TURNON: CA BIT15;
 * EXTEND; WOR CHAN13; TC Q. MIDORLST: AD T6NEXT; EXTEND; BZMF LASTCHG;
 * LXCH T6NEXT +1; DXCH T6FURTHA; EXTEND; SU TIME6; DXCH T6NEXT; TC Q.
 * LASTCHG: CS A; AD NEG0; TS T6FURTHA; TC Q (an overflow skips into
 * ROT-TOUV, which returns through the same Q). */
static void jtlst_body(agc_image *s,lm_flight_io *io){
 ai_cs(s,T6FURTHA);ai_ad(s,TIME6);
 if(!ai_bzmf(s)){LM_QR_PATH(11);
  ai_lxch(s,NXT6ADR);ai_dxch(s,T6NEXT);ai_dxch(s,T6FURTHA);(void)ai_ts(s,TIME6);ai_lxch(s,NXT6ADR);
  ai_caf(s,BIT15);wor13(s,&io->chan13);tcq(s);return;}
 ai_ad(s,T6NEXT);
 if(!ai_bzmf(s)){LM_QR_PATH(12);ai_lxch(s,T6NEXT1);ai_dxch(s,T6FURTHA);ai_su(s,TIME6);ai_dxch(s,T6NEXT);tcq(s);return;}
 ai_cs(s,0u);ai_adf(s,NEG0);
 if(ai_ts(s,T6FURTHA)){LM_QR_PATH(13);rot_touv_body(s);return;}
 tcq(s);
}

/* CALLQERR and the returns into it run as parts, each ending where it
 * hands Z to the next part's key (RCS, RCS +1, QRCONTRL, CHEKSTIK, MINRTN,
 * BACKHAND, TRYGTS, STILLRCS, TJLAW +3, LOOPER, DOSKIP, DOSKIP +1,
 * XTRANS): the transfer loop runs that part next, as the original runs on,
 * and a flight module links only the parts its routines reach. AFTERTJ,
 * NOROTAT, FEEDBACK, CLOSEOUT and the short tails are called directly. */
static enum exec_status handoff(agc_image *s,agc_word z){W[5]=z;return EXEC_OK;}
/* GOTOGTS: EXTEND; DCA GTSCADR; DTCB. */
static enum exec_status gotogts(agc_image *s){
 static const agc_word gtscadr[2]={03077u,042106u}; /* 2CADR GTS */
 ai_dcaf(s,gtscadr);dtcb(s,02611u);return EXEC_OK;
}
static void alarmret_body(agc_image *s){
 /* (ALARM's return) CA 00314OCT; MASK POLYTEMP; TS NEXTU; CS 00314OCT;
  * MASK POLYTEMP; TS NEXTV. */
 ai_caf(s,OCT00314);ai_mask(s,POLYTEMP);(void)ai_ts(s,NEXTU);ai_csf(s,OCT00314);ai_mask(s,POLYTEMP);(void)ai_ts(s,NEXTV);
}
static enum exec_status closeout_part(agc_image *s,lm_flight_io *io){
 /* CLOSEOUT: CA ADRRUPT; TC MAKERUPT. MAKERUPT: EXTEND;
  * EDRUPT MAKERUPT. yaAGC's EDRUPT, with no interrupt pending, saves its
  * address +1 in ZRUPT and its word in BRUPT, sets the interrupt in
  * progress and vectors to address 0, where A (ADRRUPT) runs as
  * TC ENDJASK. ENDJASK: DXCH DAPARUPT; DXCH ARUPT; DXCH DAPBQRPT;
  * XCH BRUPT; LXCH Q; CAF NEGMAX; DXCH DAPZRUPT; DXCH ZRUPT; TCF NOQRSM. */
 ai_caf(s,ADRRUPT);ai_tc(s,03240u);
 W[ZRUPT]=07756u;W[BRUPT]=LM_QR_MAKERUPT;io->in_isr=1u;
 ai_tc(s,1u);
 ai_dxch(s,DAPARUPT);ai_dxch(s,ARUPT);ai_dxch(s,DAPBQRPT);ai_xch(s,BRUPT);ai_lxch(s,QREG);ai_caf(s,NEGMAX);
 ai_dxch(s,DAPZRUPT);ai_dxch(s,ZRUPT);
 W[5]=LM_QR_NOQRSM;return EXEC_OK;
}
static enum exec_status norotat(agc_image *s,lm_flight_io *io){
 static const agc_word indexes[2]={4u,015u}; /* INDEXES */
 /* NOROTAT: INDEX AXISCTR; CA INDEXES; INHINT; INDEX A;
  * TC WRITEP -1. */
 ai_caf(s,indexes[AX]);io->bias.interrupt_enabled=0u;
 return leave(s,03004u,(agc_word)(LM_QR_WRITEP1+W[0u]));
}
static enum exec_status feedback(agc_image *s){
 /* FEEDBACK: CS THREE; AD NUMBERT; EXTEND; BZMF +3; CA TWO; TCF +2;
  * CA ONE; INDEX AXISCTR; TS NO.UJETS; TCF LOOPER. */
 ai_csf(s,THREE);ai_ad(s,NUMBERT);
 if(ai_bzmf(s)){LM_QR_PATH(162);ai_caf(s,ONE);}
 else ai_caf(s,TWO);
 (void)ai_ts(s,NOUJETS+AX);return handoff(s,LM_QR_LOOPER);
}
static enum exec_status writeuret_part(agc_image *s,lm_flight_io *io){
 ai_caf(s,FOUR);ai_xch(s,SKIPV);io->bias.interrupt_enabled=1u;
 if(ai_bzf(s)){LM_QR_PATH(164);return closeout_part(s,io);}
 io->bias.interrupt_enabled=0u;return leave(s,03103u,LM_QR_WRITEV1);
}
static enum exec_status writevret_part(agc_image *s,lm_flight_io *io){
 io->bias.interrupt_enabled=1u;
 return closeout_part(s,io);
}
static enum exec_status aftertj(agc_image *s,lm_flight_io *io){uint8_t c;
 static const agc_word indexes[2]={4u,015u}; /* INDEXES */
 /* AFTERTJ: CA FLAGWRD5; MASK SNUFFBIT; EXTEND; BZF DOROTAT;
  * CS FLGWRD10; MASK APSFLBIT; EXTEND; BZF DOROTAT; CA DAPBOOLS;
  * MASK DRIFTBIT; EXTEND; BZF XTRANS. */
 ai_ca(s,FLAGWRD5);ai_maskf(s,SNUFFBIT);
 if(ai_bzf(s)){LM_QR_PATH(137);goto dorotat;}
 ai_cs(s,FLGWRD10);ai_maskf(s,APSFLBIT);
 if(ai_bzf(s)){LM_QR_PATH(138);goto dorotat;}
 ai_ca(s,DAPBOOLS);ai_maskf(s,DRIFTBIT);
 if(ai_bzf(s)){LM_QR_PATH(139);return handoff(s,LM_QR_XTRANS);}
dorotat: /* DOROTAT: CAF TWO; TS L; INDEX AXISCTR; CCS TJU; TCF +5;
  * TCF NOROTAT; TCF +2; TCF NOROTAT; ZL; AD ONE; TS ABSTJ; CA AXISCTR;
  * AD L; TS ROTINDEX; CA ABSTJ; AD -150MS; EXTEND; BZMF DOSKIP;
  * TC SELCTSUB; INDEX AXISCTR; CA INDEXES; TS L; CA POLYTEMP; INHINT;
  * INDEX L; TC WRITEP. */
 ai_caf(s,TWO);(void)ai_ts(s,LREG);
 c=ai_ccs(s,TJU+AX);LM_QR_PATH(140u+c);
 if(c==1u||c==3u)return norotat(s,io);
 if(c==2u)ai_zl(s);
 ai_adf(s,ONE);(void)ai_ts(s,ABSTJ);ai_ca(s,AXISCTR);ai_ad(s,LREG);(void)ai_ts(s,ROTINDEX);
 ai_ca(s,ABSTJ);ai_adf(s,MINUS150MS);
 if(ai_bzmf(s)){LM_QR_PATH(144);return handoff(s,LM_QR_DOSKIP);}
 if(selctsub(s,02766u)==SEL_ALARM)return EXEC_OK;
 ai_caf(s,indexes[AX]);(void)ai_ts(s,LREG);ai_ca(s,POLYTEMP);io->bias.interrupt_enabled=0u;
 return leave(s,02775u,(agc_word)(LM_QR_WRITEP+W[LREG]));
}
static void spsret_body(lm_flight_io *io,agc_image *s){
 /* RELINT; CAF FOUR; TS NUMBERT. */
 io->bias.interrupt_enabled=1u;ai_caf(s,FOUR);(void)ai_ts(s,NUMBERT);
}
/* CALLQERR to Q,RORGTS. */
static enum exec_status callqerr_part(agc_image *s,lm_flight_io *io){uint8_t c;
 /* CALLQERR: CA BIT13; EXTEND; RAND CHAN31; CCS A; TCF +5; CS DAPBOOLS;
  * MASK OURRCBIT; EXTEND; BZF Q,RORGTS; TC QERRCALC. */
 ai_caf(s,BIT13);rand(s,io->chan31);
 c=ai_ccs(s,0u);LM_QR_PATH(14u+c);
 if(c==0u)goto qerr;
 if(c==1u)ai_cs(s,DAPBOOLS);
 if(c!=3u)ai_maskf(s,OURRCBIT);
 if(ai_bzf(s)){LM_QR_PATH(18);goto qrorgts;}
qerr:
 ai_tc(s,02104u);qerrcalc_body(s);
qrorgts: /* Q,RORGTS: CCS COTROLER; TCF GOTOGTS; TCF TRYGTS; (RCS +1); RCS. */
 c=ai_ccs(s,COTROLER);LM_QR_PATH(19u+c);
 if(c==0u)return gotogts(s);
 if(c==1u)return handoff(s,LM_QR_TRYGTS);
 if(c==3u)return handoff(s,LM_QR_RCS1);
 return handoff(s,LM_QR_RCS);
}
/* RCS (or RCS +1): SENSEGET, the translation jets, to QRCONTRL. */
static enum exec_status rcs(agc_image *s,lm_flight_io *io,agc_word z){uint8_t c;
 /* RCS: CAF ZERO; TS COTROLER; DXCH EDOTQ; TC ROT-TOUV; DXCH OMEGAU. */
 if(z==LM_QR_RCS1)goto rcs1;
 ai_caf(s,ZERO);
rcs1:
 (void)ai_ts(s,COTROLER);ai_dxch(s,EDOTQ);rot_touv(s,02113u);ai_dxch(s,OMEGAU);
 /* SENSEGET: CA BIT7; EXTEND; RAND CHAN31; EXTEND; BZF +X0RULGE; CA BIT8;
  * EXTEND; RAND CHAN31; EXTEND; BZF -XTRANS; CA ULLAGER; MASK DAPBOOLS;
  * CCS A; TCF +X0RULGE; TS NEXTU; TS NEXTV; CS DAPBOOLS; MASK DRIFTBIT;
  * EXTEND; BZF TSENSE; CA FLGWRD10; MASK APSFLBIT; CCS A; CAF TWO.
  * TSENSE: TS SENSETYP; TCF QRCONTRL. */
 ai_caf(s,BIT7);rand(s,io->chan31);
 if(ai_bzf(s)){LM_QR_PATH(23);goto xorulge;}
 ai_caf(s,BIT8);rand(s,io->chan31);
 if(ai_bzf(s)){LM_QR_PATH(24);goto xtransm;}
 ai_caf(s,ULLAGER);ai_mask(s,DAPBOOLS);
 c=ai_ccs(s,0u);LM_QR_PATH(25u+c);
 if(c==0u)goto xorulge;
 if(c==1u)(void)ai_ts(s,NEXTU);
 if(c!=3u)(void)ai_ts(s,NEXTV);
 ai_cs(s,DAPBOOLS);ai_maskf(s,DRIFTBIT);
 if(ai_bzf(s)){LM_QR_PATH(29);goto tsense;}
 ai_ca(s,FLGWRD10);ai_maskf(s,APSFLBIT);
 c=ai_ccs(s,0u);LM_QR_PATH(30u+c);
 if(c==0u)ai_caf(s,TWO);
 if(c==2u)return handoff(s,LM_QR_QRCONTRL);
 if(c==3u)goto xorulge;
tsense:
 (void)ai_ts(s,SENSETYP);return handoff(s,LM_QR_QRCONTRL);
xorulge: /* +X0RULGE: CAF ONE. -XTRANS: AD FOUR; TS ROTINDEX; AD NEG3;
  * TS SENSETYP; CA DAPBOOLS; MASK ACC4OR2X; CCS A; TCF TRANS4;
  * CA DAPBOOLS; MASK AORBTRAN; CCS A; CA ONE; AD TWO. TSNUMBRT: TS NUMBERT;
  * TC SELCTSUB; CCS POLYTEMP; TCF +3; TC ALARM; OCT 02002.
  * TRANS4: CA FOUR; TCF TSNUMBRT. */
 ai_caf(s,ONE);
xtransm:
 ai_adf(s,FOUR);(void)ai_ts(s,ROTINDEX);ai_adf(s,NEG3);(void)ai_ts(s,SENSETYP);ai_ca(s,DAPBOOLS);ai_maskf(s,ACC4OR2X);
 c=ai_ccs(s,0u);LM_QR_PATH(34u+c);
 if(c==0u){ai_caf(s,FOUR);goto tsnumbrt;}
 if(c==1u)ai_ca(s,DAPBOOLS);
 if(c!=3u)ai_maskf(s,AORBTRAN);
 c=ai_ccs(s,0u);LM_QR_PATH(38u+c);
 if(c==0u)ai_caf(s,ONE);
 if(c<=1u)ai_adf(s,TWO);
 if(c==3u)goto selc;
tsnumbrt:
 (void)ai_ts(s,NUMBERT);
selc:
 if(selctsub(s,02166u)==SEL_ALARM)return EXEC_OK;
 c=ai_ccs(s,POLYTEMP);LM_QR_PATH(42u+c);
 if(c==1u)return leave(s,02171u,LM_QR_ALARM);
 if(c==2u)return leave(s,02172u,02002u); /* the word OCT 02002 runs as TC 02002 */
 alarmret_body(s);
 return handoff(s,LM_QR_QRCONTRL);
}
static enum exec_status qrcontrl(agc_image *s,lm_flight_io *io){uint8_t c;
 /* QRCONTRL: CA BIT13; EXTEND; RAND CHAN31; CCS A; TCF ATTSTEER.
  * CHKBIT10: CAF PULSES; MASK DAPBOOLS; EXTEND; BZF CHEKSTIK; INHINT;
  * TC IBNKCALL; CADR ZATTEROR. */
 ai_caf(s,BIT13);rand(s,io->chan31);
 c=ai_ccs(s,0u);LM_QR_PATH(46u+c);
 if(c==0u)return handoff(s,LM_QR_STILLRCS);
 if(c==1u)ai_caf(s,PULSES);
 if(c!=3u)ai_mask(s,DAPBOOLS);
 if(ai_bzf(s)){LM_QR_PATH(50);return handoff(s,LM_QR_CHEKSTIK);}
 io->bias.interrupt_enabled=0u;return leave(s,02213u,LM_QR_IBNKCALL);
}
static enum exec_status minret_part(agc_image *s,lm_flight_io *io){uint8_t c;
 /* CA ZERO; TS QERROR; TS RERROR; RELINT; EXTEND; READ CHAN31;
  * TS TEMP31; CCS OLDQRMIN; TCF CHECKIN. FIREQR: CA TEMP31; MASK BIT1;
  * EXTEND; BZF +QMIN; (BIT2 -QMIN, BIT5 +RMIN, BIT6 -RMIN); TCF XTRANS.
  * CHECKIN: CS TEMP31; MASK OCT63; TS OLDQRMIN; TCF XTRANS. */
 ai_caf(s,ZERO);(void)ai_ts(s,QERROR);(void)ai_ts(s,RERROR);io->bias.interrupt_enabled=1u;
 rd(s,io->chan31);(void)ai_ts(s,TEMP31);
 c=ai_ccs(s,OLDQRMIN);LM_QR_PATH(51u+c);
 if(c==0u){ai_cs(s,TEMP31);ai_maskf(s,OCT63);(void)ai_ts(s,OLDQRMIN);return handoff(s,LM_QR_XTRANS);}
 if(c==3u)goto pqmin; /* -0 enters the BZF's EXTEND with A +0 */
 if(c==1u)ai_ca(s,TEMP31);
 ai_maskf(s,BIT1);
 if(ai_bzf(s)){LM_QR_PATH(55);goto pqmin;}
 ai_ca(s,TEMP31);ai_maskf(s,BIT2);
 if(ai_bzf(s)){LM_QR_PATH(56);goto mqmin;}
 ai_ca(s,TEMP31);ai_maskf(s,BIT5);
 if(ai_bzf(s)){LM_QR_PATH(57);goto prmin;}
 ai_ca(s,TEMP31);ai_maskf(s,BIT6);
 if(ai_bzf(s)){LM_QR_PATH(58);goto mrmin;}
 return handoff(s,LM_QR_XTRANS);
 /* +QMIN: CA 14MS; TS TJU; CS 14MS; TCF MINQR. -QMIN: CS 14MS; TS TJU;
  * CA 14MS; TCF MINQR. +RMIN: CA 14MS; TCF +2. -RMIN: CS 14MS; TS TJU.
  * MINQR: TS TJV; CA MINADR; TS RETJADR; CA ONE; TS OLDQRMIN. */
pqmin:
 ai_caf(s,MS14);(void)ai_ts(s,TJU);ai_csf(s,MS14);goto minqr;
mqmin:
 ai_csf(s,MS14);(void)ai_ts(s,TJU);ai_caf(s,MS14);goto minqr;
prmin:
 ai_caf(s,MS14);goto tstju;
mrmin:
 ai_csf(s,MS14);
tstju:
 (void)ai_ts(s,TJU);
minqr:
 (void)ai_ts(s,TJV);ai_caf(s,MINADR);(void)ai_ts(s,RETJADR);ai_caf(s,ONE);(void)ai_ts(s,OLDQRMIN);
 return handoff(s,LM_QR_MINRTN);
}
static enum exec_status minrtn(agc_image *s,lm_flight_io *io){uint8_t c;
 /* MINRTN: TS AXISCTR; CA DAPBOOLS; MASK CSMDOCKD; EXTEND;
  * BZF MIMRET; INDEX AXISCTR; CCS TJU; CA 60MS; TCF +2; CS 60MS;
  * INDEX AXISCTR; TS TJU. MIMRET: CA DAPBOOLS; MASK AORBTRAN; CCS A;
  * CA ONE; AD TWO; TS NUMBERT; TCF AFTERTJ. */
 (void)ai_ts(s,AXISCTR);ai_ca(s,DAPBOOLS);ai_maskf(s,CSMDOCKD);
 if(ai_bzf(s))LM_QR_PATH(59);
 else{
  c=ai_ccs(s,TJU+AX);LM_QR_PATH(60u+c);
  if(c==0u)ai_caf(s,MS60);
  if(c==2u)ai_csf(s,MS60);
  (void)ai_ts(s,TJU+AX);
 }
 ai_ca(s,DAPBOOLS);ai_maskf(s,AORBTRAN);
 c=ai_ccs(s,0u);LM_QR_PATH(64u+c);
 if(c==0u)ai_caf(s,ONE);
 if(c<=1u)ai_adf(s,TWO);
 if(c<=2u)(void)ai_ts(s,NUMBERT);
 return aftertj(s,io);
}
static enum exec_status chekstik(agc_image *s,lm_flight_io *io){uint8_t c;
 /* CHEKSTIK: TS INGTS; CS ONE; TS COTROLER; CA BIT15;
  * MASK CH31TEMP; EXTEND; BZF RHCACTIV; CA OURRCBIT; MASK DAPBOOLS; EXTEND;
  * BZF STILLRCS; CS BIT9; MASK RCSFLAGS; TS RCSFLAGS; TCF DAMPING.
  * DAMPING: CA ZERO; TS SAVEHAND; TS SAVEHAND +1. */
 (void)ai_ts(s,INGTS);ai_csf(s,ONE);(void)ai_ts(s,COTROLER);ai_caf(s,BIT15);ai_mask(s,CH31TEMP);
 if(ai_bzf(s)){LM_QR_PATH(68);goto rhcactiv;}
 ai_caf(s,OURRCBIT);ai_mask(s,DAPBOOLS);
 if(ai_bzf(s)){LM_QR_PATH(69);return handoff(s,LM_QR_STILLRCS);}
 ai_csf(s,BIT9);ai_mask(s,RCSFLAGS);(void)ai_ts(s,RCSFLAGS);
 ai_caf(s,ZERO);(void)ai_ts(s,SAVEHAND);(void)ai_ts(s,SAVEHAND1);
rhcactiv: /* RHCACTIV: CCS SAVEHAND; TCF +3; TCF +2; TCF +1; DOUBLE; DOUBLE;
  * AD LINRAT; EXTEND; MP SAVEHAND; CA L; EXTEND; MP STIKSENS; XCH QLAST;
  * COM; AD QLAST; TS DAPTEMP3; CCS SAVEHAND +1; TCF +3; TCF +2; TCF +1;
  * (R alike: TS DAPTEMP4); CS QLAST; AD OMEGAQ; TS QRATEDIF; CS RLAST;
  * AD OMEGAR; TS RRATEDIF. ENTERQR: DXCH QRATEDIF; TC ROT-TOUV;
  * DXCH URATEDIF. */
 c=ai_ccs(s,SAVEHAND);LM_QR_PATH(70u+c);
 ai_double(s);ai_double(s);ai_adf(s,LINRAT);ai_mp(s,SAVEHAND);ai_ca(s,LREG);ai_mp(s,STIKSENS);ai_xch(s,QLAST);
 ai_com(s);ai_ad(s,QLAST);
 if(ai_ts(s,DAPTEMP3))LM_QR_PATH(74); /* skips the CCS: TCF +3 */
 else{c=ai_ccs(s,SAVEHAND1);LM_QR_PATH(75u+c);}
 ai_double(s);ai_double(s);ai_adf(s,LINRAT);ai_mp(s,SAVEHAND1);ai_ca(s,LREG);ai_mp(s,STIKSENS);ai_xch(s,RLAST);
 ai_com(s);ai_ad(s,RLAST);
 if(ai_ts(s,DAPTEMP4)){LM_QR_PATH(79);goto l2411;}
 ai_cs(s,QLAST);
l2411:
 ai_ad(s,OMEGAQ);
 if(ai_ts(s,QRATEDIF)){LM_QR_PATH(80);goto l2414;}
 ai_cs(s,RLAST);
l2414:
 ai_ad(s,OMEGAR);
 if(ai_ts(s,RRATEDIF)){LM_QR_PATH(81);goto l2417;}
 ai_dxch(s,QRATEDIF);
l2417:
 rot_touv(s,02420u);ai_dxch(s,URATEDIF);
 /* CCS DAPTEMP3; TC +3; TC +2; TC +1; AD -RATEDB; EXTEND; BZMF +2;
  * TCF ENTERUV -2; (DAPTEMP4 alike); CA RCSFLAGS; MASK QRBIT; EXTEND;
  * BZF +2; TCF ENTERUV; TCF STILLRCS. ENTERUV -2: CA 40CYCL; TS TCQR.
  * ENTERUV: INHINT; TC IBNKCALL; FCADR ZATTEROR. */
 c=ai_ccs(s,DAPTEMP3);LM_QR_PATH(82u+c);
 if(c==0u)ai_tc(s,02423u);
 if(c==1u)ai_tc(s,02424u);
 if(c==2u)ai_tc(s,02425u);
 ai_ad(s,MINUSRATEDB);
 if(!ai_bzmf(s)){LM_QR_PATH(86);goto enteruv2;}
 c=ai_ccs(s,DAPTEMP4);LM_QR_PATH(87u+c);
 if(c==0u)ai_tc(s,02433u);
 if(c==1u)ai_tc(s,02434u);
 if(c==2u)ai_tc(s,02435u);
 ai_ad(s,MINUSRATEDB);
 if(!ai_bzmf(s)){LM_QR_PATH(91);goto enteruv2;}
 ai_ca(s,RCSFLAGS);ai_maskf(s,QRBIT);
 if(ai_bzf(s)){LM_QR_PATH(92);return handoff(s,LM_QR_STILLRCS);}
 goto enteruv;
enteruv2:
 ai_caf(s,C40CYCL);(void)ai_ts(s,TCQR);
enteruv:
 io->bias.interrupt_enabled=0u;return leave(s,02453u,LM_QR_IBNKCALL);
}
static enum exec_status enteruvret_part(agc_image *s,lm_flight_io *io){uint8_t c;
 /* RELINT; CA ZERO; TS DYERROR; TS DYERROR +1; TS DZERROR;
  * TS DZERROR +1; CCS URATEDIF; TCF +3; TCF +2; TCF +1; AD TARGETDB;
  * EXTEND; BZMF VDB; CCS VRATEDIF; TCF +3; TCF +2; TCF +1; AD TARGETDB;
  * EXTEND; BZMF +2; TCF QRTIME; CA ZERO; TS VRATEDIF; TCF QRTIME.
  * VDB: CCS VRATEDIF; TC +3; TC +2; TC +1; AD TARGETDB; EXTEND;
  * BZMF TOPSEUDO; CA ZERO; TS URATEDIF. */
 io->bias.interrupt_enabled=1u;ai_caf(s,ZERO);
 (void)ai_ts(s,DYERROR);(void)ai_ts(s,DYERROR1);(void)ai_ts(s,DZERROR);(void)ai_ts(s,DZERROR1);
 c=ai_ccs(s,URATEDIF);LM_QR_PATH(93u+c);
 ai_ad(s,TARGETDB);
 if(ai_bzmf(s)){LM_QR_PATH(97);goto vdb;}
 c=ai_ccs(s,VRATEDIF);LM_QR_PATH(98u+c);
 ai_ad(s,TARGETDB);
 if(!ai_bzmf(s)){LM_QR_PATH(102);goto qrtime;}
 ai_caf(s,ZERO);(void)ai_ts(s,VRATEDIF);goto qrtime;
vdb:
 c=ai_ccs(s,VRATEDIF);LM_QR_PATH(103u+c);
 if(c==0u)ai_tc(s,02506u);
 if(c==1u)ai_tc(s,02507u);
 if(c==2u)ai_tc(s,02510u);
 ai_ad(s,TARGETDB);
 if(ai_bzmf(s)){LM_QR_PATH(107);goto topseudo;}
 ai_caf(s,ZERO);(void)ai_ts(s,URATEDIF);
qrtime: /* QRTIME: CA TCQR; EXTEND; BZMF +5; CS RCSFLAGS; MASK QRBIT;
  * ADS RCSFLAGS; TC +4. TOPSEUDO: CS QRBIT; MASK RCSFLAGS;
  * TS RCSFLAGS; CA HANDADR; TS RETJADR; CA ONE. */
 ai_ca(s,TCQR);
 if(ai_bzmf(s)){LM_QR_PATH(108);goto topseudo;}
 ai_cs(s,RCSFLAGS);ai_maskf(s,QRBIT);ai_ads(s,RCSFLAGS);ai_tc(s,02524u);goto handadr;
topseudo:
 ai_csf(s,QRBIT);ai_mask(s,RCSFLAGS);(void)ai_ts(s,RCSFLAGS);
handadr:
 ai_caf(s,HANDADR);(void)ai_ts(s,RETJADR);ai_caf(s,ONE);
 return handoff(s,LM_QR_BACKHAND);
}
static enum exec_status backhand(agc_image *s,lm_flight_io *io){uint8_t c;
 static const agc_word axisdiff[2]={0u,020u}; /* AXISDIFF (TJET_LAW) */
 /* BACKHAND: TS AXISCTR; CA FOUR; TS NUMBERT; INDEX AXISCTR;
  * INDEX SKIPU; TCF +1; CA FOUR; INDEX AXISCTR; TS SKIPU; TCF LOOPER;
  * INDEX AXISCTR; CCS URATEDIF; CA ZERO; TCF +2; CA ONE; INDEX AXISCTR;
  * AD AXISDIFF; INDEX A; CS 1/ANET2 +1; EXTEND; INDEX AXISCTR;
  * MP URATEDIF; TS Q; DAS A; AD Q; TS A; TCF +2; CA Q.
  * SETTIME: INDEX AXISCTR; TS TJU; TCF AFTERTJ. (SKIPU is 0 or 4: TCF +1
  * or the word four past it.) */
 (void)ai_ts(s,AXISCTR);ai_caf(s,FOUR);(void)ai_ts(s,NUMBERT);
 if(ai_word(s,SKIPU+AX)==0u){LM_QR_PATH(109);ai_caf(s,FOUR);(void)ai_ts(s,SKIPU+AX);return handoff(s,LM_QR_LOOPER);}
 c=ai_ccs(s,URATEDIF+AX);LM_QR_PATH(110u+c);
 if(c==0u)ai_caf(s,ZERO);
 if(c==2u)ai_caf(s,ONE);
 ai_adf(s,axisdiff[AX]);ai_cs(s,ONEANET2P1+(unsigned)sa(s)); /* INDEX A: A is 0, 1, 020 or 021 */
 ai_mp(s,URATEDIF+AX);(void)ai_ts(s,QREG);ai_ddoubl(s);ai_ad(s,QREG);
 if(ai_ts(s,0u)){LM_QR_PATH(114);ai_ca(s,QREG);}
 (void)ai_ts(s,TJU+AX);return aftertj(s,io);
}
static enum exec_status trygts(agc_image *s,lm_flight_io *io){uint8_t c;
 /* TRYGTS: CAF USEQRJTS; MASK DAPBOOLS; CCS A; TCF RCS;
  * CCS ALLOWGTS; TCF +2; TCF RCS; EXTEND; READ CHAN5; CCS A;
  * TCF CHKINGTS. GOTOGTS: EXTEND; DCA GTSCADR; DTCB. */
 ai_caf(s,USEQRJTS);ai_mask(s,DAPBOOLS);
 c=ai_ccs(s,0u);LM_QR_PATH(115u+c);
 if(c==0u||c==3u)return handoff(s,LM_QR_RCS);
 if(c==1u){
  c=ai_ccs(s,ALLOWGTS);LM_QR_PATH(119u+c); /* -0 is refused */
  if(c==1u)return handoff(s,LM_QR_RCS);
 }
 rd(s,io->chan5);
 c=ai_ccs(s,0u);LM_QR_PATH(122u+c); /* negative is refused */
 if(c!=0u)return gotogts(s);
 /* CHKINGTS: CCS INGTS; TCF +2; TCF RCS; INHINT; TC IBNKCALL;
  * CADR TIMEGMBL. */
 c=ai_ccs(s,INGTS);LM_QR_PATH(124u+c);
 if(c==1u)return handoff(s,LM_QR_RCS);
 if(c!=3u)io->bias.interrupt_enabled=0u;
 return leave(s,02617u,LM_QR_IBNKCALL);
}
/* STILLRCS (or TJLAW +3) to TC TJETLAW, IBNKCALL SPSRCS or AFTERTJ. */
static enum exec_status stillrcs(agc_image *s,lm_flight_io *io,agc_word z){uint8_t c;
 if(z==LM_QR_TJLAW3)goto tjlaw3;
 /* STILLRCS (ATTSTEER): CA RERROR; LXCH A; CA QERROR;
  * TC ROT-TOUV; DXCH UERROR. TJLAW: CA TJLAWADR; TS RETJADR; CA ONE;
  * TS AXISCTR; INDEX AXISCTR; INDEX SKIPU; TCF +1; CA FOUR; INDEX AXISCTR;
  * TS SKIPU; TCF LOOPER; INDEX AXISCTR; CA UERROR; TS E; INDEX AXISCTR;
  * CA OMEGAU; TS EDOT; CA DAPBOOLS; MASK CSMDOCKD; CCS A; TCF +3;
  * TC TJETLAW; TCF AFTERTJ. +3: CS DAPBOOLS; MASK USEQRJTS; CCS A;
  * TS COTROLER; INHINT; TC IBNKCALL; CADR SPSRCS. */
 ai_ca(s,RERROR);ai_lxch(s,0u);ai_ca(s,QERROR);rot_touv(s,02665u);ai_dxch(s,UERROR);
 ai_caf(s,TJLAWADR);(void)ai_ts(s,RETJADR);ai_caf(s,ONE);
tjlaw3:
 (void)ai_ts(s,AXISCTR);
 if(ai_word(s,SKIPU+AX)==0u){LM_QR_PATH(128);ai_caf(s,FOUR);(void)ai_ts(s,SKIPU+AX);return handoff(s,LM_QR_LOOPER);}
 ai_ca(s,UERROR+AX);(void)ai_ts(s,EREG);ai_ca(s,OMEGAU+AX);(void)ai_ts(s,EDOT);
 ai_ca(s,DAPBOOLS);ai_maskf(s,CSMDOCKD);
 c=ai_ccs(s,0u);LM_QR_PATH(129u+c);
 if(c==1u)return leave(s,02714u,LM_QR_TJETLAW);
 if(c==2u)return aftertj(s,io);
 ai_cs(s,DAPBOOLS);ai_maskf(s,USEQRJTS);
 c=ai_ccs(s,0u);LM_QR_PATH(133u+c);
 if(c==0u)(void)ai_ts(s,COTROLER);
 if(c<=1u)io->bias.interrupt_enabled=0u;
 if(c!=3u)return leave(s,02723u,LM_QR_IBNKCALL);
 ai_csf(s,SPSRCSW); /* -0 runs the CADR SPSRCS word as CS 03727 */
 spsret_body(io,s);
 return aftertj(s,io);
}
static enum exec_status looper(agc_image *s,lm_flight_io *io){uint8_t c;
 /* LOOPER: CCS AXISCTR; TC RETJADR; TCF CLOSEOUT (the negative and
  * -0 exits enter DOSKIP and DOSKIP +1). TC RETJADR runs the word there:
  * GENADR MINRTN, BACKHAND or TJLAW +3 is a TC with Q = RETJADR +1. */
 c=ai_ccs(s,AXISCTR);LM_QR_PATH(145u+c);
 if(c==1u)return closeout_part(s,io);
 if(c==2u)return handoff(s,LM_QR_DOSKIP);
 if(c==3u)return handoff(s,LM_QR_DOSKIP1);
 ai_tc(s,03007u);
 switch(ai_word(s,RETJADR)){
 case MINADR:LM_QR_PATH(149);ai_tc(s,RETJADR+1u);return handoff(s,LM_QR_MINRTN);
 case HANDADR:LM_QR_PATH(150);ai_tc(s,RETJADR+1u);return handoff(s,LM_QR_BACKHAND);
 case TJLAWADR:LM_QR_PATH(151);ai_tc(s,RETJADR+1u);return handoff(s,LM_QR_TJLAW3);
 default:LM_QR_PATH(152);return EXEC_OUT_OF_SCOPE; /* unreachable: refused at the entry */
 }
}
/* DOSKIP (or DOSKIP +1) and NOTMIN, to TC WRITEP. */
static enum exec_status doskip(agc_image *s,lm_flight_io *io,agc_word z){uint8_t c;
 static const agc_word indexes[2]={4u,015u}; /* INDEXES */
 /* DOSKIP: CS ABSTJ; AD +TJMINT6; EXTEND; BZMF NOTMIN; ADS ABSTJ;
  * INDEX AXISCTR; CCS TJU; CA +TJMINT6; TCF +2; CS +TJMINT6;
  * INDEX AXISCTR; TS TJU; CCS SENSETYP; TCF NOTMIN -1; EXTEND;
  * READ LOSCALAR; MASK ONE; TS NUMBERT. */
 if(z==LM_QR_DOSKIP)ai_cs(s,ABSTJ);
 ai_adf(s,MS14);
 if(ai_bzmf(s)){LM_QR_PATH(153);goto notmin;}
 ai_ads(s,ABSTJ);
 c=ai_ccs(s,TJU+AX);LM_QR_PATH(154u+c);
 if(c==0u)ai_caf(s,MS14);
 if(c==2u)ai_csf(s,MS14);
 (void)ai_ts(s,TJU+AX);
 c=ai_ccs(s,SENSETYP);LM_QR_PATH(158u+c);
 if(c==2u)io->bias.interrupt_enabled=0u; /* READ LOSCALAR's word without its EXTEND: INHINT */
 if(c==1u)rd(s,io->chan4);
 if(c!=0u)ai_maskf(s,ONE);
 (void)ai_ts(s,NUMBERT);
notmin: /* NOTMIN: TC SELCTSUB; INDEX AXISCTR; CA INDEXES; INHINT;
  * TS T6FURTHA +1; CA POLYTEMP; INDEX T6FURTHA +1; TC WRITEP. */
 if(selctsub(s,03033u)==SEL_ALARM)return EXEC_OK;
 ai_caf(s,indexes[AX]);io->bias.interrupt_enabled=0u;(void)ai_ts(s,T6FURTHA1);ai_ca(s,POLYTEMP);
 return leave(s,03042u,(agc_word)(LM_QR_WRITEP+ai_word(s,T6FURTHA1)));
}
static enum exec_status notminret_part(agc_image *s,lm_flight_io *io){
 /* CA ABSTJ; TS T6FURTHA; TC JTLST; RELINT; CA ZERO;
  * INDEX AXISCTR; TS SKIPU. FEEDBACK: CS THREE; AD NUMBERT; EXTEND;
  * BZMF +3; CA TWO; TCF +2; CA ONE; INDEX AXISCTR; TS NO.UJETS;
  * TCF LOOPER. */
 ai_ca(s,ABSTJ);(void)ai_ts(s,T6FURTHA);ai_tc(s,03045u);jtlst_body(s,io);
 io->bias.interrupt_enabled=1u;ai_caf(s,ZERO);(void)ai_ts(s,SKIPU+AX);
 return feedback(s);
}
static enum exec_status xtrans(agc_image *s,lm_flight_io *io){
 /* XTRANS: CA ZERO; TS TJU; TS TJV; CA FOUR; INHINT; XCH SKIPU;
  * EXTEND; BZF +2; TC WRITEU -1; CA FOUR; XCH SKIPV; RELINT; EXTEND;
  * BZF CLOSEOUT; INHINT; TC WRITEV -1; RELINT; TCF CLOSEOUT. */
 ai_caf(s,ZERO);(void)ai_ts(s,TJU);(void)ai_ts(s,TJV);ai_caf(s,FOUR);io->bias.interrupt_enabled=0u;ai_xch(s,SKIPU);
 if(!ai_bzf(s)){LM_QR_PATH(163);return leave(s,03074u,LM_QR_WRITEU1);}
 return writeuret_part(s,io);
}

enum {AT_CALLQERR,AT_ALARMRET,AT_MINRET,AT_ENTERUVRET,AT_TIMEGMBLRET,AT_TJETRET,AT_SPSRET,AT_WRITEPRET,AT_NOROTATRET,
 AT_NOTMINRET,AT_WRITEURET,AT_WRITEVRET,AT_SELALARMRET};
/* (Ifs, so each compare's outcome is its own edge for the C64 rows.) */
static int skips(const agc_image *s){agc_word u=ai_word(s,SKIPU),v=ai_word(s,SKIPV);if(u!=0u&&u!=4u)return 0;if(v==0u||v==4u)return 1;return 0;}
static int rjok(const agc_image *s){agc_word w=ai_word(s,RETJADR);return w==MINADR||w==HANDADR||w==TJLAWADR;}
/* Refused with the image unchanged: values no producer stores, where the
 * original would read them before storing them. CALLQERR, ALARMRET, MINRET,
 * ENTERUVRET and TIMEGMBLRET store AXISCTR, NUMBERT and RETJADR (MINRTN,
 * MIMRET, BACKHAND, TJLAW, TSNUMBRT) before reading them, so P-AXIS's
 * PURGENCY AXISCTR -1, GTS's SAVESR in AXISCTR and PJETSLEC's NUMBERT 5 or
 * 6 are accepted there (SENSEGET's SELCTSUB, which indexes TJU by AXISCTR
 * on its alarm, never alarms: TYPEPOLY 0 and 1 share no jet with ALLJETS
 * 4 or 5). The loop's own returns read them as the loop left them. */
static int checks(agc_image *s,void *v,unsigned at){lm_flight_io *io=v;agc_word st;
 if(!entry(s,v)||!skips(s))return 0;
 st=ai_word(s,SENSETYP);
 switch(at){
 case AT_CALLQERR:
  if(ai_word(s,ALLOWGTS)==077777u||(io->chan5&040000u))return 0;
  break;
 case AT_TIMEGMBLRET:break; /* SENSEGET stores SENSETYP */
 case AT_ALARMRET:case AT_MINRET:case AT_ENTERUVRET:
  if(st>5u&&st<040000u)return 0;
  break;
 default:
  if(AX>1u||ai_word(s,NUMBERT)>4u||(st>5u&&st<040000u))return 0;
  /* LOOPER's TC RETJADR runs with AXISCTR 1. */
  if(AX==1u&&!rjok(s)&&at!=AT_WRITEURET&&at!=AT_WRITEVRET)return 0;
 }
 return 1;
}
#define ENTRY(name,at,body) static enum exec_status name(const agc_native_program *p,agc_image *s,void *v){ \
 lm_flight_io *io=v;(void)p;(void)io;if(!checks(s,v,at))return EXEC_OUT_OF_SCOPE; \
 body}
ENTRY(callqerr,AT_CALLQERR,return callqerr_part(s,io);)
ENTRY(alarmret,AT_ALARMRET,alarmret_body(s);return handoff(s,LM_QR_QRCONTRL);)
ENTRY(minret,AT_MINRET,return minret_part(s,io);)
ENTRY(enteruvret,AT_ENTERUVRET,return enteruvret_part(s,io);)
/* RELINT; CAF ZERO; TS INGTS; TCF RCS. */
ENTRY(timegmblret,AT_TIMEGMBLRET,io->bias.interrupt_enabled=1u;ai_caf(s,ZERO);(void)ai_ts(s,INGTS);
 return handoff(s,LM_QR_RCS);)
ENTRY(tjetret,AT_TJETRET,return aftertj(s,io);)
ENTRY(spsret,AT_SPSRET,spsret_body(io,s);return aftertj(s,io);)
/* RELINT; TCF FEEDBACK. */
ENTRY(writepret,AT_WRITEPRET,io->bias.interrupt_enabled=1u;return feedback(s);)
/* RELINT; (LOOPER). */
ENTRY(norotatret,AT_NOROTATRET,io->bias.interrupt_enabled=1u;return handoff(s,LM_QR_LOOPER);)
ENTRY(notminret,AT_NOTMINRET,return notminret_part(s,io);)
ENTRY(writeuret,AT_WRITEURET,return writeuret_part(s,io);)
ENTRY(writevret,AT_WRITEVRET,return writevret_part(s,io);)
ENTRY(selalarmret,AT_SELALARMRET,return norotat(s,io);)
/* The parts' own keys. */
#define PART(name,body) static enum exec_status name(const agc_native_program *p,agc_image *s,void *v){ \
 lm_flight_io *io=v;(void)p;if(!entry(s,v))return EXEC_OUT_OF_SCOPE; \
 return body;}
PART(rcs_e,rcs(s,io,LM_QR_RCS))
PART(rcs1_e,rcs(s,io,LM_QR_RCS1))
PART(qrcontrl_e,qrcontrl(s,io))
PART(minrtn_e,minrtn(s,io))
PART(chekstik_e,chekstik(s,io))
PART(backhand_e,backhand(s,io))
PART(trygts_e,trygts(s,io))
PART(stillrcs_e,stillrcs(s,io,LM_QR_STILLRCS))
PART(tjlaw3_e,stillrcs(s,io,LM_QR_TJLAW3))
PART(looper_e,looper(s,io))
PART(doskip_e,doskip(s,io,LM_QR_DOSKIP))
PART(doskip1_e,doskip(s,io,LM_QR_DOSKIP1))
PART(xtrans_e,xtrans(s,io))
/* CLOSEOUT, from GTS's DTCB. */
PART(closeout,closeout_part(s,io))
/* QERRCALC, JTLST and ROT-TOUV, entered by TC or IBNKCALL with the return
 * in Q. */
static enum exec_status qerrcalc(const agc_native_program *p,agc_image *s,void *v){(void)p;
 if(!entry(s,v)||!tc_word(W[QREG]))return EXEC_OUT_OF_SCOPE;
 W[5]=W[QREG];qerrcalc_body(s);return EXEC_OK;
}
static enum exec_status jtlst(const agc_native_program *p,agc_image *s,void *v){(void)p;
 if(!entry(s,v)||!tc_word(W[QREG]))return EXEC_OUT_OF_SCOPE;
 W[5]=W[QREG];jtlst_body(s,v);return EXEC_OK;
}
static enum exec_status rottouv(const agc_native_program *p,agc_image *s,void *v){(void)p;
 if(!entry(s,v)||!tc_word(W[QREG]))return EXEC_OUT_OF_SCOPE;
 W[5]=W[QREG];rot_touv_body(s);return EXEC_OK;
}
const agc_site_operand lm_q_r_axis_sites[LM_QR_SITES]={
#include "lm_q_r_axis_sites.inc"
};
/* 1/10S OCT 1, BIT8,9 OCT 00600, SCLNORM OCT 266, and ZEROTJ: CA ZERO;
 * TCF SETTIME (no code reaches it). */
static int qr_data(uint16_t physical,agc_word *word){
 static const uint16_t at[5]={P17(02343u),P17(03111u),P17(03112u),P17(02571u),P17(02572u)};
 static const agc_word w[5]={1u,0600u,0266u,034755u,012566u};unsigned i;
 for(i=0;i<5u;++i)if(at[i]==physical){*word=w[i];return 1;}
 return 0;
}
bool lm_q_r_axis_data(uint16_t physical,agc_word *word){return word&&qr_data(physical,word);}
static const agc_routine_entry entries[30]={
 {P17(LM_QR_CALLQERR),callqerr},{P17(LM_QR_RCS),rcs_e},{P17(LM_QR_RCS1),rcs1_e},{P17(LM_QR_ALARMRET),alarmret},
 {P17(LM_QR_QRCONTRL),qrcontrl_e},{P17(LM_QR_MINRET),minret},{P17(LM_QR_MINRTN),minrtn_e},
 {P17(LM_QR_CHEKSTIK),chekstik_e},{P17(LM_QR_ENTERUVRET),enteruvret},{P17(LM_QR_BACKHAND),backhand_e},
 {P17(LM_QR_TRYGTS),trygts_e},{P17(LM_QR_TIMEGMBLRET),timegmblret},{P17(LM_QR_QERRCALC),qerrcalc},
 {P17(LM_QR_STILLRCS),stillrcs_e},{P17(LM_QR_TJLAW3),tjlaw3_e},{P17(LM_QR_TJETRET),tjetret},
 {P17(LM_QR_SPSRET),spsret},{P17(LM_QR_WRITEPRET),writepret},{P17(LM_QR_NOROTATRET),norotatret},
 {P17(LM_QR_LOOPER),looper_e},{P17(LM_QR_DOSKIP),doskip_e},{P17(LM_QR_DOSKIP1),doskip1_e},
 {P17(LM_QR_NOTMINRET),notminret},{P17(LM_QR_XTRANS),xtrans_e},{P17(LM_QR_WRITEURET),writeuret},
 {P17(LM_QR_WRITEVRET),writevret},{P17(LM_QR_JTLST),jtlst},{P17(LM_QR_ROTTOUV),rottouv},
 {P17(LM_QR_SELALARMRET),selalarmret},{P17(LM_QR_CLOSEOUT),closeout}};
const agc_routine_table lm_q_r_axis_routines={entries,30};
