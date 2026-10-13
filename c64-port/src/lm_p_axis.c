/* SPDX-License-Identifier: MIT
 * Luminary099 P-AXIS_RCS_AUTOPILOT.agc, all of it (bank 16, 02213-03640), as
 * routines of the control-transfer ABI. Each source instruction is one
 * agc_image_ops call (yaAGC-qualified register effects) or, for MSU, DV,
 * DTCB and the channel orders, a helper here with yaAGC's effect, named with
 * its erasable address, channel or the source's fixed constant. TS overflow
 * skips and CCS exits land where the original's do, including the CCS -0
 * exits that enter the next instruction past its EXTEND. */
#include "lm_p_axis.h"
#include "agc_image_ops.h"
#ifndef LM_PA_GENERIC
/* Every instruction runs on the one canonical image (fixed in the flight
 * store, bound at each entry elsewhere): a plain-cell operand under this
 * file's EBANK 6 goes to the fast helper (include/agc_fast_ops.h), any
 * other operand to a one-argument wrapper (include/agc_image_thin.h); both
 * have the effects of the agc_image_ops helper. -DLM_PA_GENERIC builds the
 * same source on agc_image_ops alone. */
#include "agc_fast_ops.h"
#include "agc_image_thin.h"
#define PA_EBN (LM_PA_EB>>8)
#define PA_OP(n,s,a) ((void)(s),AFO_PLAIN(PA_EBN,a)?afo_##n(AFO_CELL(PA_EBN,a)):AIT_ARG(n,a))
#define PA_OP2(n,s,a) ((void)(s),AFO_PLAIN2(PA_EBN,a)?afo_##n(AFO_CELL(PA_EBN,a)):AIT_ARG(n,a))
#define PA_F(n,s,k) ((void)(s),afo_##n(k))
#define ai_ca(s,a) PA_OP(ca,s,a)
#define ai_cs(s,a) PA_OP(cs,s,a)
#define ai_ad(s,a) PA_OP(ad,s,a)
#define ai_su(s,a) PA_OP(su,s,a)
#define ai_mask(s,a) PA_OP(mask,s,a)
#define ai_xch(s,a) PA_OP(xch,s,a)
#define ai_lxch(s,a) PA_OP(lxch,s,a)
#define ai_qxch(s,a) PA_OP(qxch,s,a)
#define ai_ads(s,a) PA_OP(ads,s,a)
#define ai_incr(s,a) PA_OP(incr,s,a)
#define ai_dim(s,a) PA_OP(dim,s,a)
#define ai_das(s,a) PA_OP2(das,s,a)
#define ai_dca(s,a) PA_OP2(dca,s,a)
#define ai_dxch(s,a) PA_OP2(dxch,s,a)
#define ai_mp(s,a) PA_OP(mp,s,a)
#define ai_dv(s,a) PA_OP(dv,s,a)
#define ai_ts(s,a) PA_OP(ts,s,a)
#define ai_ccs(s,a) PA_OP(ccs,s,a)
#define ai_caf(s,k) PA_F(caf,s,k)
#define ai_csf(s,k) PA_F(csf,s,k)
#define ai_adf(s,k) PA_F(adf,s,k)
#define ai_maskf(s,k) PA_F(maskf,s,k)
#define ai_mpf(s,k) PA_F(mpf,s,k)
#define ai_tc(s,k) PA_F(tc,s,k)
#define ai_dcaf(s,k) PA_F(dcaf,s,k)
#define ai_bzf(s) ((void)(s),afo_bzf())
#define ai_bzmf(s) ((void)(s),afo_bzmf())
#define W ((void)s,AFO_S)->words
#define PA_BIND(s) (AIT_BIND(s),AFO_BIND(s))
#else
#include "agc_image_w.h"
#define PA_BIND(s) ((void)0)
#endif
/* Branch coverage hook for the host test (empty in flight builds). */
#ifdef LM_PA_COUNT_PATHS
extern unsigned lm_pa_paths[];
#define LM_PA_PATH(n) ((void)++lm_pa_paths[n])
#else
#define LM_PA_PATH(n) ((void)0)
#endif
#define P16(z) (uint16_t)(016u*1024u+((z)-02000u))
/* Erasable: central and interrupt registers, counters, unswitched, and
 * EBANK6 (EBANK= PERROR). */
#define LREG 1u
#define QREG 2u
#define ZEROREG 7u
#define ARUPT 010u
#define QRUPT 012u
#define ZRUPT 015u
#define BANKRUPT 016u
#define BRUPT 017u
#define TIME5 030u
#define CDUX 032u
#define CDUY 033u
#define CDUZ 034u
#define QRHCCTR 042u
#define PRHCCTR 043u
#define RRHCCTR 044u
#define DAPBOOLS 0111u
#define CH6MASK 01263u
#define RCSFLAGS 01273u
#define DELPEROR 01277u
#define DKTRAP 01403u
#define DKOMEGAN 01404u
#define LMTRAP 01406u
#define LMOMEGAN 01407u
#define M11 01414u
#define M21 01415u
#define M31 01416u
#define M22 01417u
#define M32 01420u
#define OMEGAP 01421u
#define OMEGAQ 01422u
#define OMEGAR 01423u
#define ALPHAQ 01424u
#define ALPHAR 01425u
#define OMEGAU 01426u
#define EDOTP 01427u
#define TRAPEDP 01430u
#define TRAPEDQ 01431u
#define TRAPEDR 01432u
#define NPTRAPS 01433u
#define NQTRAPS 01434u
#define NRTRAPS 01435u
#define EDOTQ 01436u
#define EDOTR 01437u
#define OLDXFORP 01440u
#define OLDYFORP 01441u
#define OLDZFORQ 01442u
#define CH31TEMP 01443u
#define STIKSENS 01444u
#define TCP 01445u
#define DXERROR 01446u
#define DXERROR1 01447u /* DXERROR +1 */
#define DYERROR 01450u
#define DYERROR1 01451u
#define DZERROR 01452u
#define DZERROR1 01453u
#define PLAST 01454u
#define QLAST 01455u
#define RLAST 01456u
#define TCQR 01457u
#define OLDPMIN 01460u
#define SAVEHAND 01462u
#define PERROR 01464u
#define T6FURTHA 01470u
#define NEXTP 01472u
#define MINUS2JETLIM 01475u /* -2JETLIM */
#define TARGETDB 01476u      /* = -RATEDB */
#define MINUSRATEDB 01476u
#define SENSETYP 01500u
#define AXISCTR 01505u
#define NOPJETS 01521u       /* NO.PJETS */
#define TJP 01524u
#define ONEJACC 01530u       /* 1JACC */
#define AOSQ 01537u
#define AOSR 01541u
#define AOSQTERM 01545u
#define AOSRTERM 01546u
#define ONEANETP 01551u      /* 1/ANETP */
#define NEGUQ 01501u
#define NEGUR 01503u
#define QACCDOT 01510u
#define RACCDOT 01512u
#define QGIMTIMR 01632u
#define RGIMTIMR 01634u
#define CDUXD 01635u
#define CDUYD 01636u
#define CDUZD 01637u
#define DELCDUX 01640u
#define DELCDUY 01641u
#define DELCDUZ 01642u
#define OMEGAPD 01643u
#define OMEGAQD 01644u
#define OMEGARD 01645u
#define DAPTEMP1 01737u      /* = ABSEDOTP = ABSTJ */
#define ABSEDOTP 01737u
#define ABSTJ 01737u
#define DAPTEMP2 01740u
#define DAPTEMP3 01741u      /* = POLYTEMP = FIREFCT */
#define POLYTEMP 01741u
#define FIREFCT 01741u
#define TEMPNUM 01742u
#define DAPTEMP5 01743u      /* = NUMBERT */
#define NUMBERT 01743u
#define ROTINDEX 01744u
#define JETRATE 01745u
#define JETRATEQ 01746u
#define JETRATER 01747u
#define DAPTREG4 01750u
#define DAPTREG5 01751u
#define DAPTREG6 01752u      /* = E */
#define EREG 01752u
#define DAPARUPT 01753u
#define DAPBQRPT 01755u
#define DAPZRUPT 01757u
#define PJETCTR 01767u
#define UJETCTR 01770u
#define VJETCTR 01771u
/* Fixed constants (rope words): MS100 07727, CALLCODE 04766, BIT1 = ONE
 * 04753, ZERO 04755, TWO 04752, FOUR = BIT3 04751, FIVE 04756, SIX 06242,
 * TEN 04363, BIT4 04750, BIT7 04745, BIT9 = XOVINHIB 04743, PBIT = BIT10
 * 04742, QRBIT = BIT11 04741, BIT12 = OURRCBIT 04740, BIT13 = CSMDOCKD
 * 04737, BIT14 04736, BIT15 = PULSES = NEGMAX 04735, POSMAX 04733, -BIT14
 * 07736, DRIFTBIT 04744, AORBSYST = CALLGMBL 04747, OCT14 05751, PRIO6 =
 * RGIMBITS 05020, OCT1400 = QGIMBITS 05007, EBANK5 = BITS9,11 05014,
 * PRIO31 = 25/32 07716; this file's -OCT630, 1/10SEC .. LINRATP,
 * -160MST6, -FOURDEG, JETSALL/TYPEP, INDXYZ = -136MST6 .. MINTIMES, and the
 * words SUPERADR, SUPERJOB and QERRCALL. */
#define MS100 037766u
#define CALLCODE 032u
#define ONE 1u
#define BIT1 1u
#define ZERO 0u
#define TWO 2u
#define FOUR 4u
#define BIT3 4u
#define FIVE 5u
#define SIX 6u
#define TEN 012u
#define BIT4 010u
#define BIT7 0100u
#define BIT9 0400u
#define XOVINHIB 0400u
#define PBIT 01000u
#define QRBIT 02000u
#define BIT12 04000u
#define OURRCBIT 04000u
#define BIT13 010000u
#define CSMDOCKD 010000u
#define BIT14 020000u
#define BIT15 040000u
#define PULSES 040000u
#define NEGMAX 040000u
#define POSMAX 037777u
#define MBIT14 057777u
#define DRIFTBIT 0200u
#define AORBSYST 020u
#define CALLGMBL 020u
#define OCT14 014u
#define RGIMBITS 06000u
#define QGIMBITS 01400u
#define BITS9_11 02400u
#define C25_32 031000u
#define MOCT630 077147u
#define C40CYC 050u
#define PQRBIT 074777u
#define LINRATP 056u
#define MINUS160MST6 077377u
#define MINUSFOURDEG 075117u
#define PLUS150MST6 0360u
#define MINUS136MST6 077445u
#define OCT07400 07400u
#define BITS8_9 0600u
#define ONE40 0632u          /* 1/40 */
#define MS200 06315u         /* 200MS */
#define SUPERJOBW 013641u    /* SUPERJOB: TCF RATELOOP */
#define SUPERADR 02667u      /* GENADR SUPERJOB +1 */

static int tc_word(agc_register w){return w>=02000u&&w<=07777u;}
static int entry(const agc_image *s,const void *io){PA_BIND(s);return s&&io&&(W[4]&076000u)==LM_PA_FB16&&W[3]==LM_PA_EB;}
static agc_word oc(agc_register a){return (agc_word)((a&037777u)|((a>>1)&040000u));}
/* The signed value of an erasable word, and of the 16-bit A. */
static int sv(const agc_image *s,unsigned k){agc_word v=ai_word(s,k);return v&040000u?-(int)(~v&037777u):(int)v;}
static int sa(const agc_image *s){agc_register a=W[0];return a&0100000u?-(int)(~a&077777u):(int)(a&077777u);}
/* Channel orders on a 15-bit channel, as yaAGC's: READ, RAND, WAND, WOR;
 * RXOR on channel 1 (L) works on the raw 16-bit registers. */
static void rd(agc_image *s,agc_word chan){W[0]=agc_extend(chan);}
static void rand(agc_image *s,agc_word chan){W[0]=agc_extend((agc_word)(oc(W[0])&chan));}
static void wand(agc_image *s,agc_word *chan){agc_word v=(agc_word)(oc(W[0])&*chan);*chan=v;W[0]=agc_extend(v);}
/* WOR CHAN13: bits 12-14 written to channel 13 are the HANDRUPT trap
 * enables, which do not read back (yaAGC keeps 043777). */
static void wor13(agc_image *s,agc_word *chan){agc_word v=(agc_word)((oc(W[0])|*chan)&077777u);*chan=(agc_word)(v&043777u);W[0]=agc_extend(v);}
static void rxor_l(agc_image *s){W[0]=(agc_register)((W[0]^W[LREG])&0177777u);}
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

/* 1STOTWOS, entered by TC from ret - 1: CCS A; AD ONE; TC Q; CS A; TC Q. */
static void onetotwos(agc_image *s,agc_word ret){uint8_t c;
 ai_tc(s,ret);c=ai_ccs(s,0u);LM_PA_PATH(0u+c);
 if(c==0u)ai_adf(s,ONE);
 if(c==2u)ai_cs(s,0u);
 tcq(s);
}
/* SUBDIVDE (plus3 0) or SUBDIVDE +3, entered by TC from ret - 1:
 * EXTEND; MP DAPTEMP3; DAS OMEGAU; +3 EXTEND; DCA OMEGAU; DXCH DAPTEMP5;
 * CCS OMEGAU; TCF +2; TCF DIVIDER; AD -OCT630; EXTEND; BZMF DIVIDER;
 * CCS OMEGAU; CA POSMAX; TC Q; CS POSMAX; TC Q.
 * DIVIDER: DXCH OMEGAU; EXTEND; DV DAPTREG4; TC Q. */
static void subdivde(agc_image *s,int plus3,agc_word ret){uint8_t c;
 ai_tc(s,ret);
 if(!plus3){ai_mp(s,DAPTEMP3);ai_das(s,OMEGAU);}
 ai_dca(s,OMEGAU);ai_dxch(s,DAPTEMP5);
 c=ai_ccs(s,OMEGAU);LM_PA_PATH(4u+c);
 if(c==1u||c==3u)goto divider; /* TCF DIVIDER; -0: BZMF DIVIDER on +0 */
 ai_adf(s,MOCT630);
 if(ai_bzmf(s)){LM_PA_PATH(8);goto divider;}
 c=ai_ccs(s,OMEGAU);LM_PA_PATH(9u+c);
 if(c==0u)ai_caf(s,POSMAX);
 if(c==2u)ai_csf(s,POSMAX);
 tcq(s);return;
divider:
 ai_dxch(s,OMEGAU);ai_dv(s,DAPTREG4);tcq(s);
}
/* OVERSUB, entered by TC from ret - 1: TS 7; TC Q; INDEX A; CS BIT15 -1;
 * TC Q (on overflow, A +-1 selects BIT15 or POSMAX: A limited to
 * +-POSMAX). */
static void oversub(agc_image *s,agc_word ret){static const agc_word limits[3]={037777u,057777u,040000u}; /* BIT15 -2 .. BIT15 */
 ai_tc(s,ret);
 if(ai_ts(s,ZEROREG)){LM_PA_PATH(13);ai_csf(s,limits[W[0u]==1u?2:0]);} /* INDEX A: A is +1 or -1 */
 tcq(s);
}
/* ZEROENBL, entered by TC from ret - 1: LXCH R-RHCCTR; CA Q-RHCCTR;
 * DXCH SAVEHAND; CA ZERO; TS P-RHCCTR; TS Q-RHCCTR; TS R-RHCCTR;
 * CA BITS8,9; EXTEND; WOR CHAN13; TC Q. */
static void zeroenbl(agc_image *s,lm_flight_io *io,agc_word ret){
 ai_tc(s,ret);
 ai_lxch(s,RRHCCTR);ai_ca(s,QRHCCTR);ai_dxch(s,SAVEHAND);ai_caf(s,ZERO);
 (void)ai_ts(s,PRHCCTR);(void)ai_ts(s,QRHCCTR);(void)ai_ts(s,RRHCCTR);
 ai_caf(s,BITS8_9);wor13(s,&io->chan13);tcq(s);
}
/* CALCPERR body: CA CDUY; EXTEND; MSU CDUYD; EXTEND; MP M11; XCH E;
 * CA CDUX; EXTEND; MSU CDUXD; AD DELPEROR; ADS E; XCH PERROR; TC Q. */
static void calcperr_body(agc_image *s){
 ai_ca(s,CDUY);msu(s,CDUYD);ai_mp(s,M11);ai_xch(s,EREG);
 ai_ca(s,CDUX);msu(s,CDUXD);ai_ad(s,DELPEROR);ai_ads(s,EREG);ai_xch(s,PERROR);tcq(s);
}
/* ALTSYST, entered by TC from ret - 1: CA DAPBOOLS; TS L; CA AORBSYST;
 * EXTEND; RXOR LCHAN; TS DAPBOOLS; RELINT; TC Q. */
static void altsyst(agc_image *s,lm_flight_io *io,agc_word ret){
 ai_tc(s,ret);
 ai_ca(s,DAPBOOLS);(void)ai_ts(s,LREG);ai_caf(s,AORBSYST);rxor_l(s);
 if(ai_ts(s,DAPBOOLS))LM_PA_PATH(14); /* skips RELINT */
 else io->bias.interrupt_enabled=1u;
 tcq(s);
}
enum {SEL_RETURN,SEL_ALARM,SEL_ABORTYZ};
/* SELECTP (yz 0, after CA SIX) or SELECTYZ (yz 1), entered by TC from
 * ret - 1. SELECTP: CA SIX; TS TEMPNUM; INDEX NUMBERT; CA TYPEP;
 * INDEX ROTINDEX; MASK JETSALL; TS POLYTEMP; MASK CH6MASK; CCS A; TCF +2;
 * TC Q; CCS TEMPNUM; TCF +4; TC ALARM; OCT 02003; TCF JETSOFF.
 * SELECTYZ: TS NUMBERT; TCF SELECTP +1. An INDEX of 07776 turns
 * MASK JETSALL into TC JETSALL -1, whose word is TCF ABORTYZ +2. */
static int selectp(agc_image *s,int yz,agc_word ret){uint8_t c;int k;
 static const agc_word jetsall[14]={0252u,0125u,0140u,06u,0220u,011u,0151u, /* JETSALL */
  0146u,0226u,0231u,0151u,0132u,0245u,0377u}; /* TYPEP */
 ai_tc(s,ret);
 if(!yz)ai_caf(s,SIX);
 for(;;){
  if(yz)(void)ai_ts(s,NUMBERT);
  (void)ai_ts(s,TEMPNUM);
  ai_caf(s,jetsall[7+sv(s,NUMBERT)]);
  k=sv(s,ROTINDEX);
  if(k==07776){LM_PA_PATH(15);ai_tc(s,03537u);return SEL_ABORTYZ;}
  ai_maskf(s,jetsall[k]);(void)ai_ts(s,POLYTEMP);ai_mask(s,CH6MASK);
  c=ai_ccs(s,0u);
  if(c!=0u){LM_PA_PATH(16);tcq(s);return SEL_RETURN;}
  if(ai_ccs(s,TEMPNUM)!=0u){LM_PA_PATH(17);ai_tc(s,03547u);W[5]=LM_PA_ALARM;return SEL_ALARM;}
  LM_PA_PATH(18);yz=1;
 }
}

/* The rate loop (BACKP to the first call that leaves the file) runs as
 * parts, each ending where it hands Z to the next part's key (16,2451,
 * Q-RATE, R-RATE, PRETIMCK,
 * CHKVISFZ, TSNEXTP -1, TSNEXTP, DETENTCK, RATERROR, PJETSLEC -6 and -1,
 * PURGENCY and PURGENCY +4): the
 * transfer loop runs that part next, as the original runs on, and a flight
 * module links only the parts its routines reach. */
enum {AT_PJETSLEC6,AT_PJETSLEC1,AT_SPSRET,AT_TJETRET};
static enum exec_status handoff(agc_image *s,agc_word z){W[5]=z;return EXEC_OK;}
/* ABORTYZ +2: CA BIT1; LXCH RCSFLAGS; EXTEND; RXOR 1; TS RCSFLAGS;
 * CA ZERO; TCF TSNEXTP (the caller's). */
static void abortyz2(agc_image *s){
 ai_caf(s,BIT1);ai_lxch(s,RCSFLAGS);rxor_l(s);
 if(ai_ts(s,RCSFLAGS))LM_PA_PATH(53);
 else ai_caf(s,ZERO);
}
static enum exec_status jetsoff(agc_image *s){ /* JETSOFF: TC WRITEP -1. */
 return leave(s,03443u,LM_PA_WRITEP1);
}
/* QRAXIS: CS OMEGARD; AD OMEGAR; TC OVERSUB; TS EDOTR; CS OMEGAQD;
 * AD OMEGAQ; TC OVERSUB; TS EDOTQ; EXTEND; DCA QERRCALL; DTCB. */
static enum exec_status qraxis(agc_image *s){
 static const agc_word qerrcall[2]={02072u,036106u}; /* 2CADR CALLQERR */
 ai_cs(s,OMEGARD);ai_ad(s,OMEGAR);oversub(s,03627u);(void)ai_ts(s,EDOTR);
 ai_cs(s,OMEGAQD);ai_ad(s,OMEGAQ);oversub(s,03633u);(void)ai_ts(s,EDOTQ);
 ai_dcaf(s,qerrcall);dtcb(s,03636u);
 return EXEC_OK;
}
/* BACKP: rate derivation, to 16,2451. */
static enum exec_status rateloop(agc_image *s){
 /* BACKP: CA DAPTEMP1; EXTEND; MP 1JACC; TS JETRATE; CAE DAPTREG4; TS L;
  * EXTEND; MSU OLDXFORP; LXCH OLDXFORP; TS DAPTEMP1; CA 1/40;
  * TS DAPTREG4; CS JETRATE; EXTEND; MP BIT14; ADS TRAPEDP; CA JETRATEQ;
  * AD AOSQTERM; EXTEND; MP -BIT14; ADS TRAPEDQ; CA JETRATER; AD AOSRTERM;
  * EXTEND; MP -BIT14; ADS TRAPEDR. */
 ai_ca(s,DAPTEMP1);ai_mp(s,ONEJACC);(void)ai_ts(s,JETRATE);ai_ca(s,DAPTREG4);(void)ai_ts(s,LREG);
 msu(s,OLDXFORP);ai_lxch(s,OLDXFORP);(void)ai_ts(s,DAPTEMP1);ai_caf(s,ONE40);(void)ai_ts(s,DAPTREG4);
 ai_cs(s,JETRATE);ai_mpf(s,BIT14);ai_ads(s,TRAPEDP);
 ai_ca(s,JETRATEQ);ai_ad(s,AOSQTERM);ai_mpf(s,MBIT14);ai_ads(s,TRAPEDQ);
 ai_ca(s,JETRATER);ai_ad(s,AOSRTERM);ai_mpf(s,MBIT14);ai_ads(s,TRAPEDR);
 /* CA DAPTREG5; TS L; EXTEND; MSU OLDYFORP; LXCH OLDYFORP; TS DAPTEMP2;
  * EXTEND; MP M11; AD DAPTEMP1; DXCH OMEGAU; TC SUBDIVDE +3; EXTEND;
  * SU OMEGAP; ADS TRAPEDP; TC OVERSUB; TS TRAPEDP; EXTEND; DCA DAPTEMP5;
  * DAS DXERROR; CS PLAST; EXTEND; MP 1/40; DAS DXERROR. */
 ai_ca(s,DAPTREG5);(void)ai_ts(s,LREG);msu(s,OLDYFORP);ai_lxch(s,OLDYFORP);(void)ai_ts(s,DAPTEMP2);
 ai_mp(s,M11);ai_ad(s,DAPTEMP1);ai_dxch(s,OMEGAU);subdivde(s,1,02363u);
 ai_su(s,OMEGAP);ai_ads(s,TRAPEDP);oversub(s,02367u);(void)ai_ts(s,TRAPEDP);
 ai_dca(s,DAPTEMP5);ai_das(s,DXERROR);ai_cs(s,PLAST);ai_mpf(s,ONE40);ai_das(s,DXERROR);
 /* CA DAPTREG6; TS L; EXTEND; MSU OLDZFORQ; TS DAPTEMP3; LXCH OLDZFORQ;
  * CA M21; EXTEND; MP DAPTEMP2; DXCH OMEGAU; CA M22; TC SUBDIVDE; EXTEND;
  * SU OMEGAQ; ADS TRAPEDQ; TC OVERSUB; TS TRAPEDQ; EXTEND; DCA DAPTEMP5;
  * DAS DYERROR; CS QLAST; EXTEND; MP 1/40; DAS DYERROR. */
 ai_ca(s,DAPTREG6);(void)ai_ts(s,LREG);msu(s,OLDZFORQ);(void)ai_ts(s,DAPTEMP3);ai_lxch(s,OLDZFORQ);
 ai_ca(s,M21);ai_mp(s,DAPTEMP2);ai_dxch(s,OMEGAU);ai_ca(s,M22);subdivde(s,0,02413u);
 ai_su(s,OMEGAQ);ai_ads(s,TRAPEDQ);oversub(s,02417u);(void)ai_ts(s,TRAPEDQ);
 ai_dca(s,DAPTEMP5);ai_das(s,DYERROR);ai_cs(s,QLAST);ai_mpf(s,ONE40);ai_das(s,DYERROR);
 /* CA M31; EXTEND; MP DAPTEMP2; DXCH OMEGAU; CA M32; TC SUBDIVDE; EXTEND;
  * SU OMEGAR; ADS TRAPEDR; TC OVERSUB; TS TRAPEDR; EXTEND; DCA DAPTEMP5;
  * DAS DZERROR; CS RLAST; EXTEND; MP 1/40; DAS DZERROR. */
 ai_ca(s,M31);ai_mp(s,DAPTEMP2);ai_dxch(s,OMEGAU);ai_ca(s,M32);subdivde(s,0,02435u);
 ai_su(s,OMEGAR);ai_ads(s,TRAPEDR);oversub(s,02441u);(void)ai_ts(s,TRAPEDR);
 ai_dca(s,DAPTEMP5);ai_das(s,DZERROR);ai_cs(s,RLAST);ai_mpf(s,ONE40);ai_das(s,DZERROR);
 return handoff(s,LM_PA_TRAPS);
}
/* 16,2451: the P trap filter, to Q-RATE. */
static enum exec_status traps(agc_image *s){uint8_t c;
 /* CA DAPBOOLS; MASK CSMDOCKD; EXTEND; BZF LMONLY; EXTEND; DCA DKOMEGAN;
  * DXCH DAPTREG4; CA DKTRAP; TCF +5. LMONLY: EXTEND; DCA LMOMEGAN;
  * DXCH DAPTREG4; CA LMTRAP. +5 TS DAPTREG6. */
 ai_ca(s,DAPBOOLS);ai_maskf(s,CSMDOCKD);
 if(ai_bzf(s)){LM_PA_PATH(19);ai_dca(s,LMOMEGAN);ai_dxch(s,DAPTREG4);ai_ca(s,LMTRAP);}
 else{LM_PA_PATH(20);ai_dca(s,DKOMEGAN);ai_dxch(s,DAPTREG4);ai_ca(s,DKTRAP);}
 (void)ai_ts(s,DAPTREG6);
 /* CCS TRAPEDP; TCF +2; TCF SMALPDIF; AD DAPTREG6; EXTEND;
  * BZMF SMALPDIF; ZL; LXCH TRAPEDP; CA ZERO; EXTEND; DV NPTRAPS;
  * ADS OMEGAP; TC OVERSUB; TS OMEGAP; CA DAPTREG4; TS NPTRAPS.
  * SMALPDIF: INCR NPTRAPS. */
 c=ai_ccs(s,TRAPEDP);LM_PA_PATH(21u+c);
 if(c==1u||c==3u)goto smalpdif;
 ai_ad(s,DAPTREG6);
 if(ai_bzmf(s)){LM_PA_PATH(25);goto smalpdif;}
 ai_zl(s);ai_lxch(s,TRAPEDP);ai_caf(s,ZERO);ai_dv(s,NPTRAPS);ai_ads(s,OMEGAP);oversub(s,02504u);(void)ai_ts(s,OMEGAP);
 ai_ca(s,DAPTREG4);(void)ai_ts(s,NPTRAPS);
smalpdif:
 ai_incr(s,NPTRAPS);
 /* P-RATE: CA JETRATE; ADS OMEGAP; TC OVERSUB; TS OMEGAP; CCS TRAPEDQ;
  * TCF +2; TCF Q-RATE; AD DAPTREG6; EXTEND; BZMF Q-RATE; ZL;
  * LXCH TRAPEDQ; CA ZERO; EXTEND; DV NQTRAPS; TS DAPTEMP1; ADS OMEGAQ;
  * TC OVERSUB; TS OMEGAQ; CA DAPTREG4; XCH NQTRAPS; AD DAPTREG5;
  * XCH DAPTEMP1; EXTEND; MP FIVE; EXTEND; DV DAPTEMP1; ADS AOSQ. */
 ai_ca(s,JETRATE);ai_ads(s,OMEGAP);oversub(s,02513u);(void)ai_ts(s,OMEGAP);
 c=ai_ccs(s,TRAPEDQ);LM_PA_PATH(26u+c);
 if(c==1u||c==3u)return handoff(s,LM_PA_QRATE);
 ai_ad(s,DAPTREG6);
 if(ai_bzmf(s)){LM_PA_PATH(30);return handoff(s,LM_PA_QRATE);}
 ai_zl(s);ai_lxch(s,TRAPEDQ);ai_caf(s,ZERO);ai_dv(s,NQTRAPS);(void)ai_ts(s,DAPTEMP1);ai_ads(s,OMEGAQ);
 oversub(s,02532u);(void)ai_ts(s,OMEGAQ);
 ai_ca(s,DAPTREG4);ai_xch(s,NQTRAPS);ai_ad(s,DAPTREG5);ai_xch(s,DAPTEMP1);ai_mpf(s,FIVE);ai_dv(s,DAPTEMP1);ai_ads(s,AOSQ);
 return handoff(s,LM_PA_QRATE);
}
/* Q-RATE: the Q trap filter and AOS term, to R-RATE. */
static enum exec_status qrate(agc_image *s){uint8_t c;
/* Q-RATE: INCR NQTRAPS; CA JETRATEQ; AD AOSQTERM; ADS OMEGAQ;
  * TC OVERSUB; TS OMEGAQ; CCS TRAPEDR ... ADS AOSR, as for Q. */
 ai_incr(s,NQTRAPS);ai_ca(s,JETRATEQ);ai_ad(s,AOSQTERM);ai_ads(s,OMEGAQ);oversub(s,02551u);(void)ai_ts(s,OMEGAQ);
 c=ai_ccs(s,TRAPEDR);LM_PA_PATH(31u+c);
 if(c==1u||c==3u)return handoff(s,LM_PA_RRATE);
 ai_ad(s,DAPTREG6);
 if(ai_bzmf(s)){LM_PA_PATH(35);return handoff(s,LM_PA_RRATE);}
 ai_zl(s);ai_lxch(s,TRAPEDR);ai_caf(s,ZERO);ai_dv(s,NRTRAPS);(void)ai_ts(s,DAPTEMP2);ai_ads(s,OMEGAR);
 oversub(s,02570u);(void)ai_ts(s,OMEGAR);
 ai_ca(s,DAPTREG4);ai_xch(s,NRTRAPS);ai_ad(s,DAPTREG5);ai_xch(s,DAPTEMP2);ai_mpf(s,FIVE);ai_dv(s,DAPTEMP2);ai_ads(s,AOSR);
 return handoff(s,LM_PA_RRATE);
}
/* R-RATE: the R trap filter, AOS terms, to PRETIMCK. */
static enum exec_status rrate(agc_image *s){
/* R-RATE: INCR NRTRAPS; CA JETRATER; AD AOSRTERM; ADS OMEGAR;
  * TC OVERSUB; TS OMEGAR; CS DAPBOOLS; MASK DRIFTBIT; CCS A;
  * TCF WORKTIME; TS ALPHAQ; TS ALPHAR; TS AOSQTERM; TS AOSRTERM; TS AOSQ;
  * TS AOSR; TCF PRETIMCK. */
 ai_incr(s,NRTRAPS);ai_ca(s,JETRATER);ai_ad(s,AOSRTERM);ai_ads(s,OMEGAR);oversub(s,02607u);(void)ai_ts(s,OMEGAR);
 ai_cs(s,DAPBOOLS);ai_maskf(s,DRIFTBIT);
 if(ai_ccs(s,0u)==0u){
  /* WORKTIME: CA QACCDOT; EXTEND; MP CALLCODE; DAS AOSQ; CA AOSQ;
   * TS ALPHAQ; EXTEND; MP 200MS; TS AOSQTERM; CA RACCDOT; EXTEND;
   * MP CALLCODE; DAS AOSR; CA AOSR; TS ALPHAR; EXTEND; MP 200MS;
   * TS AOSRTERM; TCF PRETIMCK. */
  LM_PA_PATH(36);
  ai_ca(s,QACCDOT);ai_mpf(s,CALLCODE);ai_das(s,AOSQ);ai_ca(s,AOSQ);(void)ai_ts(s,ALPHAQ);ai_mpf(s,MS200);(void)ai_ts(s,AOSQTERM);
  ai_ca(s,RACCDOT);ai_mpf(s,CALLCODE);ai_das(s,AOSR);ai_ca(s,AOSR);(void)ai_ts(s,ALPHAR);ai_mpf(s,MS200);(void)ai_ts(s,AOSRTERM);
 }else{
  LM_PA_PATH(37);
  (void)ai_ts(s,ALPHAQ);(void)ai_ts(s,ALPHAR);(void)ai_ts(s,AOSQTERM);(void)ai_ts(s,AOSRTERM);(void)ai_ts(s,AOSQ);(void)ai_ts(s,AOSR);
 }
 return handoff(s,LM_PA_PRETIMCK);
}
/* PRETIMCK: the gimbal timers, the jet counters and SKIPPAXS, to CHKVISFZ. */
static enum exec_status pretimck(agc_image *s,lm_flight_io *io){uint8_t c;
 /* PRETIMCK: CCS QGIMTIMR; TCF DECQTIMR; TCF TURNOFFQ; CHKRTIMR ... (the
  * -0 exit enters CHKRTIMR's TCF DECRTIMR). DECQTIMR: TS QGIMTIMR;
  * TCF CHKRTIMR. TURNOFFQ: TS NEGUQ; TS QACCDOT; CS QGIMBITS; EXTEND;
  * WAND CHAN12; CAF NEGMAX; TS QGIMTIMR; TCF CHKRTIMR. */
 c=ai_ccs(s,QGIMTIMR);LM_PA_PATH(38u+c);
 if(c==0u){(void)ai_ts(s,QGIMTIMR);goto chkrtimr;}
 if(c==1u){(void)ai_ts(s,NEGUQ);(void)ai_ts(s,QACCDOT);ai_csf(s,QGIMBITS);wand(s,&io->chan12);ai_caf(s,NEGMAX);(void)ai_ts(s,QGIMTIMR);goto chkrtimr;}
 if(c==3u)goto decrtimr;
chkrtimr: /* CHKRTIMR: CCS RGIMTIMR; TCF DECRTIMR; TCF TURNOFFR; EXTEND;
  * DIM PJETCTR (-0 enters its word without the EXTEND: ADS PJETCTR).
  * DECRTIMR: TS RGIMTIMR; TCF CHKRTIMR +3. TURNOFFR: TS NEGUR;
  * TS RACCDOT; CS RGIMBITS; EXTEND; WAND CHAN12; CAF NEGMAX;
  * TS RGIMTIMR; TCF CHKRTIMR +3. */
 c=ai_ccs(s,RGIMTIMR);LM_PA_PATH(42u+c);
 if(c==0u){
decrtimr:
  (void)ai_ts(s,RGIMTIMR);
 }else if(c==1u){
  (void)ai_ts(s,NEGUR);(void)ai_ts(s,RACCDOT);ai_csf(s,RGIMBITS);wand(s,&io->chan12);ai_caf(s,NEGMAX);(void)ai_ts(s,RGIMTIMR);
 }else if(c==3u){ai_ads(s,PJETCTR);goto dimu;}
 /* CHKRTIMR +3: EXTEND; DIM PJETCTR; EXTEND; DIM UJETCTR; EXTEND;
  * DIM VJETCTR; CA BIT12; MASK RCSFLAGS; EXTEND; BZF SKIPPAXS;
  * TC CHKVISFZ. */
 ai_dim(s,PJETCTR);
dimu:
 ai_dim(s,UJETCTR);ai_dim(s,VJETCTR);ai_caf(s,BIT12);ai_mask(s,RCSFLAGS);
 if(ai_bzf(s)){
  /* SKIPPAXS: CS RCSFLAGS; MASK BIT12; ADS RCSFLAGS; TCF QRAXIS. */
  LM_PA_PATH(46);ai_cs(s,RCSFLAGS);ai_maskf(s,BIT12);ai_ads(s,RCSFLAGS);return qraxis(s);
 }
 ai_tc(s,02710u);
 return handoff(s,LM_PA_CHKVISFZ);
}
/* CHKVISFZ and TRYUORV (translation), to TSNEXTP. */
static enum exec_status chkvisfz(agc_image *s,lm_flight_io *io){int k;
 static const agc_word indxyz[16]={077445u,4u,2u,07776u,5u,9u,10u,07776u,3u,8u,7u,07776u,07776u,07776u,07776u,07776u}; /* INDXYZ = -136MST6 */
 /* CHKVISFZ: EXTEND; READ CHAN31; CS A; MASK 07400OCT; EXTEND;
  * BZF TSNEXTP; EXTEND; MP BIT7; INDEX A; CA INDXYZ; TS ROTINDEX. */
 rd(s,io->chan31);ai_cs(s,0u);ai_maskf(s,OCT07400);
 if(ai_bzf(s)){LM_PA_PATH(47);return handoff(s,LM_PA_TSNEXTP);}
 ai_mpf(s,BIT7);ai_caf(s,indxyz[sa(s)]);(void)ai_ts(s,ROTINDEX);
tryuorv: /* TRYUORV: CA SIX; TC SELECTYZ; CS SIX; AD NUMBERT; EXTEND;
  * BZF TSNEXTP -1; CS FIVE; AD ROTINDEX; EXTEND; BZMF ALTERYZ;
  * CS NUMBERT; AD FOUR; EXTEND; BZMF TSNEXTP -1. */
 ai_caf(s,SIX);
 k=selectp(s,1,02755u);
 if(k==SEL_ALARM)return EXEC_OK;
 if(k==SEL_ABORTYZ){abortyz2(s);return handoff(s,LM_PA_TSNEXTP);}
 ai_csf(s,SIX);ai_ad(s,NUMBERT);
 if(ai_bzf(s)){LM_PA_PATH(48);return handoff(s,LM_PA_TSNEXTP1);}
 ai_csf(s,FIVE);ai_ad(s,ROTINDEX);
 if(ai_bzmf(s)){
  /* ALTERYZ: CA BIT1; LXCH RCSFLAGS; EXTEND; RXOR 1; TS RCSFLAGS;
   * MASK BIT1; AD FOUR; ADS ROTINDEX; TCF TRYUORV. */
  LM_PA_PATH(49);ai_caf(s,BIT1);ai_lxch(s,RCSFLAGS);rxor_l(s);
  if(ai_ts(s,RCSFLAGS))LM_PA_PATH(50);
  else ai_maskf(s,BIT1);
  ai_adf(s,FOUR);ai_ads(s,ROTINDEX);goto tryuorv;
 }
 ai_cs(s,NUMBERT);ai_adf(s,FOUR);
 if(ai_bzmf(s)){LM_PA_PATH(51);return handoff(s,LM_PA_TSNEXTP1);}
 /* ABORTYZ: TC ALARM; OCT 02001. */
 LM_PA_PATH(52);return leave(s,02772u,LM_PA_ALARM);
}
/* TSNEXTP -1 (or TSNEXTP, or ABORTYZ +2): MANMODE, CHECKP and FIREP. */
static enum exec_status manual(agc_image *s,lm_flight_io *io,agc_word at){uint8_t c;
 if(at==LM_PA_ABORTYZ2){abortyz2(s);goto tsnextp;}
 if(at==LM_PA_TSNEXTP)goto tsnextp;
 /* TSNEXTP -1: CA POLYTEMP. */
 ai_ca(s,POLYTEMP);
tsnextp: /* TSNEXTP: TS NEXTP; CA BIT13; EXTEND; RAND CHAN31; EXTEND;
  * BZF MANMODE; CA DAPBOOLS; MASK XOVINHIB; CCS A; TCF PURGENCY;
  * TCF DETENTCK. */
 (void)ai_ts(s,NEXTP);ai_caf(s,BIT13);rand(s,io->chan31);
 if(!ai_bzf(s)){
  ai_ca(s,DAPBOOLS);ai_maskf(s,XOVINHIB);
  if(ai_ccs(s,0u)==0u){LM_PA_PATH(54);return handoff(s,LM_PA_PURGENCY);}
  LM_PA_PATH(55);return handoff(s,LM_PA_DETENTCK);
 }
 /* MANMODE: CA PULSES; MASK DAPBOOLS; EXTEND; BZF DETENTCK; CA ZERO;
  * TS PERROR; CA CDUX; TS CDUXD; CCS OLDPMIN; TCF CHECKP. */
 ai_caf(s,PULSES);ai_mask(s,DAPBOOLS);
 if(ai_bzf(s)){LM_PA_PATH(56);return handoff(s,LM_PA_DETENTCK);}
 ai_caf(s,ZERO);(void)ai_ts(s,PERROR);ai_ca(s,CDUX);(void)ai_ts(s,CDUXD);
 c=ai_ccs(s,OLDPMIN);LM_PA_PATH(57u+c);
 if(c==0u){
  /* CHECKP: EXTEND; READ CHAN31; CS A; MASK OCT14; TS OLDPMIN;
   * TCF JETSOFF. */
  rd(s,io->chan31);ai_cs(s,0u);ai_maskf(s,OCT14);(void)ai_ts(s,OLDPMIN);return jetsoff(s);
 }
 /* FIREP: CA BIT3; EXTEND; RAND CHAN31; EXTEND; BZF +XMIN; CA BIT4;
  * EXTEND; RAND CHAN31; EXTEND; BZF -XMIN; TCF JETSOFF (the CCS's
  * negative exit enters at the RAND's EXTEND). */
 if(c==1u)ai_caf(s,BIT3);
 rand(s,io->chan31);
 if(ai_bzf(s)){LM_PA_PATH(61);ai_caf(s,TEN);}
 else{
  ai_caf(s,BIT4);rand(s,io->chan31);
  if(!ai_bzf(s)){LM_PA_PATH(62);return jetsoff(s);}
  LM_PA_PATH(63);ai_csf(s,TEN);
 }
 /* -XMIN: CS TEN; TCF +2. +XMIN: CA TEN; TS TJP; CA ONE; TS OLDPMIN;
  * TCF PJETSLEC -6. */
 (void)ai_ts(s,TJP);ai_caf(s,ONE);(void)ai_ts(s,OLDPMIN);return handoff(s,LM_PA_PJETSLEC6);
}
/* DETENTCK: RHCMOVED, RUTH, RATEDONE, RATEDAMP and JUSTOUT. */
static enum exec_status detentck(agc_image *s,lm_flight_io *io){
/* DETENTCK: EXTEND; READ CHAN31; TS CH31TEMP; MASK BIT15;
  * EXTEND; BZF RHCMOVED; CAF OURRCBIT; MASK DAPBOOLS; EXTEND;
  * BZF PURGENCY; CA BIT9; MASK RCSFLAGS; EXTEND; BZF RUTH; CAF BIT13;
  * EXTEND; RAND CHAN31; EXTEND; BZF RATEDAMP; CS BITS9,11; MASK RCSFLAGS;
  * TS RCSFLAGS; TCF RATEDAMP. */
 rd(s,io->chan31);(void)ai_ts(s,CH31TEMP);ai_maskf(s,BIT15);
 if(ai_bzf(s)){
  /* RHCMOVED: CA OURRCBIT; MASK DAPBOOLS; EXTEND; BZF JUSTOUT -1. */
  LM_PA_PATH(64);ai_caf(s,OURRCBIT);ai_mask(s,DAPBOOLS);
  if(ai_bzf(s)){LM_PA_PATH(65);goto justout1;}
  return handoff(s,LM_PA_RATERROR);
 }
 ai_caf(s,OURRCBIT);ai_mask(s,DAPBOOLS);
 if(ai_bzf(s)){LM_PA_PATH(66);return handoff(s,LM_PA_PURGENCY);}
 ai_caf(s,BIT9);ai_mask(s,RCSFLAGS);
 if(ai_bzf(s)){
  /* RUTH: CA RCSFLAGS; MASK PBIT; EXTEND; BZF +2; TCF RATEDAMP;
   * CA RCSFLAGS; MASK QRBIT; EXTEND; BZF RATEDONE; TCF RATEDAMP. */
  ai_ca(s,RCSFLAGS);ai_maskf(s,PBIT);
  if(!ai_bzf(s)){LM_PA_PATH(67);goto ratedamp;}
  ai_ca(s,RCSFLAGS);ai_maskf(s,QRBIT);
  if(!ai_bzf(s)){LM_PA_PATH(68);goto ratedamp;}
  /* RATEDONE: CS OURRCBIT; INHINT; MASK DAPBOOLS; TS DAPBOOLS; CAF BIT13;
   * EXTEND; RAND CHAN31; EXTEND; BZF +4; CA CDUX; TS CDUXD; TC +3;
   * TC IBNKCALL; FCADR ZATTEROR; RELINT; TCF PURGENCY. */
  LM_PA_PATH(69);ai_csf(s,OURRCBIT);io->bias.interrupt_enabled=0u;ai_mask(s,DAPBOOLS);(void)ai_ts(s,DAPBOOLS);
  ai_caf(s,BIT13);rand(s,io->chan31);
  if(ai_bzf(s)){LM_PA_PATH(70);return leave(s,03153u,LM_PA_IBNKCALL);}
  ai_ca(s,CDUX);(void)ai_ts(s,CDUXD);ai_tc(s,03152u);
  io->bias.interrupt_enabled=1u;return handoff(s,LM_PA_PURGENCY);
 }
 ai_caf(s,BIT13);rand(s,io->chan31);
 if(!ai_bzf(s)){LM_PA_PATH(71);ai_csf(s,BITS9_11);ai_mask(s,RCSFLAGS);(void)ai_ts(s,RCSFLAGS);}
ratedamp: /* RATEDAMP: CA ZERO; TS P-RHCCTR; TCF RATERROR. */
 ai_caf(s,ZERO);(void)ai_ts(s,PRHCCTR);
 return handoff(s,LM_PA_RATERROR);
justout1: /* JUSTOUT -1: TS PERROR. JUSTOUT: CA OURRCBIT; ADS DAPBOOLS;
  * CA ZERO; TS DXERROR; TS DXERROR +1; TS DYERROR; TS DYERROR +1;
  * TS DZERROR; TS DZERROR +1; TS PLAST; TS QLAST; TS RLAST;
  * TS Q-RHCCTR; TS R-RHCCTR; CA PQRBIT; MASK RCSFLAGS; TS RCSFLAGS;
  * CS RCSFLAGS; MASK BIT9; ADS RCSFLAGS; TC ZEROENBL; TCF JETSOFF. */
 (void)ai_ts(s,PERROR);ai_caf(s,OURRCBIT);ai_ads(s,DAPBOOLS);ai_caf(s,ZERO);
 (void)ai_ts(s,DXERROR);(void)ai_ts(s,DXERROR1);(void)ai_ts(s,DYERROR);(void)ai_ts(s,DYERROR1);
 (void)ai_ts(s,DZERROR);(void)ai_ts(s,DZERROR1);(void)ai_ts(s,PLAST);(void)ai_ts(s,QLAST);(void)ai_ts(s,RLAST);
 (void)ai_ts(s,QRHCCTR);(void)ai_ts(s,RRHCCTR);
 ai_caf(s,PQRBIT);ai_mask(s,RCSFLAGS);(void)ai_ts(s,RCSFLAGS);ai_cs(s,RCSFLAGS);ai_maskf(s,BIT9);ai_ads(s,RCSFLAGS);
 zeroenbl(s,io,03204u);return jetsoff(s);
}
/* RATERROR and PEGI: the P-axis RCS law, to PJETSLEC. */
static enum exec_status raterror(agc_image *s,lm_flight_io *io){uint8_t c;
/* RATERROR: CA CDUX; TS CDUXD; CCS P-RHCCTR; TCF +3; TCF +2;
  * TCF +1; DOUBLE; DOUBLE; AD LINRATP; EXTEND; MP P-RHCCTR; CA L; EXTEND;
  * MP STIKSENS; XCH PLAST; COM; AD PLAST; TS DAPTEMP1; TC ZEROENBL;
  * CS PLAST; AD OMEGAP; TS EDOTP; CCS DAPTEMP1; TCF +3; TCF +8D; TCF +1;
  * AD -RATEDB; EXTEND; BZMF +4; CA 40CYC; TS TCP; TC PEGI; CA RCSFLAGS;
  * MASK PBIT; EXTEND; BZF +2; TC PEGI; CA DXERROR; TS E; TS PERROR;
  * TC PURGENCY +4. */
 ai_ca(s,CDUX);(void)ai_ts(s,CDUXD);c=ai_ccs(s,PRHCCTR);LM_PA_PATH(72u+c);
 ai_double(s);ai_double(s);ai_adf(s,LINRATP);ai_mp(s,PRHCCTR);ai_ca(s,LREG);ai_mp(s,STIKSENS);ai_xch(s,PLAST);
 ai_com(s);ai_ad(s,PLAST);
 if(ai_ts(s,DAPTEMP1))LM_PA_PATH(76); /* skips TC ZEROENBL */
 else zeroenbl(s,io,03252u);
 ai_cs(s,PLAST);ai_ad(s,OMEGAP);
 if(ai_ts(s,EDOTP)){LM_PA_PATH(77);goto l3261;} /* skips the CCS: TCF +3 */
 c=ai_ccs(s,DAPTEMP1);LM_PA_PATH(78u+c);
 if(c==1u)goto l3267;
l3261:
 ai_ad(s,MINUSRATEDB);
 if(!ai_bzmf(s)){LM_PA_PATH(82);ai_caf(s,C40CYC);(void)ai_ts(s,TCP);ai_tc(s,03267u);goto pegi;}
l3267:
 ai_ca(s,RCSFLAGS);ai_maskf(s,PBIT);
 if(!ai_bzf(s)){LM_PA_PATH(83);ai_tc(s,03274u);goto pegi;}
 ai_ca(s,DXERROR);(void)ai_ts(s,EREG);(void)ai_ts(s,PERROR);ai_tc(s,03300u);return handoff(s,LM_PA_PURGENCY4);
pegi: /* PEGI: CA CDUX; TS CDUXD; CA ZERO; TS DXERROR; TS DXERROR +1;
  * TS PERROR; CCS EDOTP; TC +3; TC +2; TC +1; TS ABSEDOTP; AD TARGETDB;
  * EXTEND; BZMF LAST; CA TCP; EXTEND; BZMF LAST; CS RCSFLAGS; MASK PBIT;
  * ADS RCSFLAGS; TCF +4. LAST: CS PBIT; MASK RCSFLAGS; TS RCSFLAGS. */
 ai_ca(s,CDUX);(void)ai_ts(s,CDUXD);ai_caf(s,ZERO);(void)ai_ts(s,DXERROR);(void)ai_ts(s,DXERROR1);(void)ai_ts(s,PERROR);
 c=ai_ccs(s,EDOTP);LM_PA_PATH(84u+c);
 if(c==0u)ai_tc(s,03310u); /* TC +3 at 3307 */
 if(c==1u)ai_tc(s,03311u); /* TC +2 at 3310 */
 if(c==2u)ai_tc(s,03312u); /* TC +1 at 3311 */
 (void)ai_ts(s,ABSEDOTP);ai_ad(s,TARGETDB);
 if(ai_bzmf(s)){LM_PA_PATH(88);goto last;}
 ai_ca(s,TCP);
 if(ai_bzmf(s)){LM_PA_PATH(89);goto last;}
 ai_cs(s,RCSFLAGS);ai_maskf(s,PBIT);ai_ads(s,RCSFLAGS);goto l3330;
last:
 ai_csf(s,PBIT);ai_mask(s,RCSFLAGS);(void)ai_ts(s,RCSFLAGS);
l3330: /* CS EDOTP; EXTEND; MP 1/ANETP; DAS A; TC OVERSUB; EXTEND;
  * MP 25/32; TS TJP; CA ABSEDOTP; AD -2JETLIM; EXTEND; BZMF +3; CA SIX;
  * TCF +8D; CA TJP; ADS TJP. */
 ai_cs(s,EDOTP);ai_mp(s,ONEANETP);ai_ddoubl(s);oversub(s,03335u);ai_mpf(s,C25_32);(void)ai_ts(s,TJP);
 ai_ca(s,ABSEDOTP);ai_ad(s,MINUS2JETLIM);
 if(!ai_bzmf(s)){LM_PA_PATH(90);ai_caf(s,SIX);return handoff(s,LM_PA_PJETSLEC1);}
 ai_ca(s,TJP);ai_ads(s,TJP);
 return handoff(s,LM_PA_PJETSLEC6);
}
/* PJETSLEC -6 or -1 to TC WRITEP, and the SPSRCS and TJETLAW returns. */
static enum exec_status jetselect(agc_image *s,lm_flight_io *io,unsigned at){uint8_t c;int k;
 if(at==AT_SPSRET)goto spsret;
 if(at==AT_TJETRET)goto tjetret;
 if(at==AT_PJETSLEC1)goto pjetslec1;
pjetslec6: /* PJETSLEC -6: CA AORBSYST; MASK DAPBOOLS; CCS A; CA ONE;
  * AD FOUR. -1: TS NUMBERT. */
 ai_caf(s,AORBSYST);ai_mask(s,DAPBOOLS);
 if(ai_ccs(s,0u)==0u){LM_PA_PATH(91);ai_caf(s,ONE);}
 ai_adf(s,FOUR);
pjetslec1:
 (void)ai_ts(s,NUMBERT);
 /* PJETSLEC: CA ONE; TS L; CCS TJP; TCF +5; TCF JETSOFF; TCF +2;
  * TCF JETSOFF; ZL; AD ONE; TS ABSTJ; LXCH ROTINDEX; TC SELECTP. */
 ai_caf(s,ONE);(void)ai_ts(s,LREG);c=ai_ccs(s,TJP);LM_PA_PATH(92u+c);
 if(c==1u||c==3u)return jetsoff(s);
 if(c==2u)ai_zl(s);
 ai_adf(s,ONE);(void)ai_ts(s,ABSTJ);ai_lxch(s,ROTINDEX);
 k=selectp(s,0,03372u);
 if(k==SEL_ALARM)return EXEC_OK;
 if(k==SEL_ABORTYZ){abortyz2(s);return handoff(s,LM_PA_TSNEXTP);}
 /* CS SIX; AD NUMBERT; EXTEND; BZF +2; CS TWO; AD FOUR; TS NO.PJETS;
  * CA POLYTEMP; TC WRITEP. */
 ai_csf(s,SIX);ai_ad(s,NUMBERT);
 if(!ai_bzf(s)){LM_PA_PATH(96);ai_csf(s,TWO);}
 ai_adf(s,FOUR);(void)ai_ts(s,NOPJETS);ai_ca(s,POLYTEMP);
 return leave(s,03403u,LM_PA_WRITEP);
spsret: /* CA TJP; EXTEND; BZF DKALT; RELINT; TCF PJETSLEC -6.
  * DKALT: TC ALTSYST; JETSOFF. */
 ai_ca(s,TJP);
 if(ai_bzf(s)){LM_PA_PATH(102);altsyst(s,io,03442u);return jetsoff(s);}
 io->bias.interrupt_enabled=1u;goto pjetslec6;
tjetret: /* RELINT; CS FIREFCT; AD -FOURDEG; EXTEND; BZMF PJETSLEC -6;
  * CCS TJP; TCF +2; TCF JETSOFF; AD -160MST6; EXTEND; BZMF PJETSLEC -6;
  * CA SIX; TCF PJETSLEC -1. */
 io->bias.interrupt_enabled=1u;ai_cs(s,FIREFCT);ai_adf(s,MINUSFOURDEG);
 if(ai_bzmf(s)){LM_PA_PATH(103);goto pjetslec6;}
 c=ai_ccs(s,TJP);LM_PA_PATH(104u+c);
 if(c==1u)return jetsoff(s);
 if(c==3u)goto pjetslec6; /* EXTEND; BZMF PJETSLEC -6 on +0 */
 ai_adf(s,MINUS160MST6);
 if(ai_bzmf(s)){LM_PA_PATH(108);goto pjetslec6;}
 ai_caf(s,SIX);goto pjetslec1;
}
/* PURGENCY (or +4, or ZATTEROR's return) to IBNKCALL SPSRCS or TJETLAW. */
static enum exec_status urgency(agc_image *s,lm_flight_io *io,agc_word at){
 if(at==LM_PA_ZATTRET){io->bias.interrupt_enabled=1u;goto purgency;}
 if(at==LM_PA_PURGENCY4)goto purgency4;
purgency: /* PURGENCY: TC CALCPERR; CS OMEGAPD; AD OMEGAP; TS EDOTP;
  * CS ONE; TS AXISCTR; CA DAPBOOLS; MASK CSMDOCKD; EXTEND; BZF HEADTJET;
  * INHINT; TC IBNKCALL; CADR SPSRCS. */
 ai_tc(s,03464u);calcperr_body(s);
 ai_cs(s,OMEGAPD);ai_ad(s,OMEGAP);
 if(ai_ts(s,EDOTP)){LM_PA_PATH(99);goto purgency5;}
purgency4:
 ai_csf(s,ONE);
purgency5:
 (void)ai_ts(s,AXISCTR);ai_ca(s,DAPBOOLS);ai_maskf(s,CSMDOCKD);
 io->bias.interrupt_enabled=0u;
 if(!ai_bzf(s)){LM_PA_PATH(100);return leave(s,03477u,LM_PA_IBNKCALL);}
 /* HEADTJET: CA ZERO; TS SENSETYP; INHINT; TC IBNKCALL; CADR TJETLAW. */
 LM_PA_PATH(101);ai_caf(s,ZERO);(void)ai_ts(s,SENSETYP);
 return leave(s,03511u,LM_PA_IBNKCALL);
}
/* 16,3403 (WRITEP's return): CS ABSTJ; AD +150MST6; EXTEND;
 * BZMF QRAXIS; AD -136MST6; EXTEND; BZMF +5; ADS ABSTJ; INDEX ROTINDEX;
 * CA MINTIMES; TS TJP; CA ABSTJ; ZL; INHINT; DXCH T6FURTHA;
 * TC IBNKCALL; CADR JTLST. */
static enum exec_status writepret_body(agc_image *s,lm_flight_io *io){
 static const agc_word mintimes[2]={077751u,026u};
 ai_cs(s,ABSTJ);ai_adf(s,PLUS150MST6);
 if(ai_bzmf(s)){LM_PA_PATH(97);return qraxis(s);}
 ai_adf(s,MINUS136MST6);
 if(!ai_bzmf(s)){LM_PA_PATH(98);ai_ads(s,ABSTJ);ai_caf(s,mintimes[sv(s,ROTINDEX)]);(void)ai_ts(s,TJP);}
 ai_ca(s,ABSTJ);ai_zl(s);io->bias.interrupt_enabled=0u;ai_dxch(s,T6FURTHA);
 return leave(s,03423u,LM_PA_IBNKCALL);
}
/* 16,3424 (JTLST's return): CS BIT12; MASK RCSFLAGS; TS RCSFLAGS;
 * TC ALTSYST; TCF QRAXIS. */
static enum exec_status jtlstret_body(agc_image *s,lm_flight_io *io){
 ai_csf(s,BIT12);ai_mask(s,RCSFLAGS);(void)ai_ts(s,RCSFLAGS);altsyst(s,io,03430u);return qraxis(s);
}
/* 16,3443 (WRITEP -1's return): CA ZERO; TS TJP; TCF QRAXIS. */
static enum exec_status jetsoffret_body(agc_image *s){
 ai_caf(s,ZERO);(void)ai_ts(s,TJP);return qraxis(s);
}

/* PAXFILT +4 (ACDT+C12's return): DXCH ARUPT; DXCH DAPARUPT; CA SUPERJOB;
 * XCH BRUPT; LXCH QRUPT; DXCH DAPBQRPT; CA SUPERADR; DXCH ZRUPT;
 * DXCH DAPZRUPT; TCF NOQBRSM +1. */
static enum exec_status afteracdt_body(agc_image *s){
 ai_dxch(s,ARUPT);ai_dxch(s,DAPARUPT);ai_caf(s,SUPERJOBW);ai_xch(s,BRUPT);ai_lxch(s,QRUPT);ai_dxch(s,DAPBQRPT);
 ai_caf(s,SUPERADR);ai_dxch(s,ZRUPT);ai_dxch(s,DAPZRUPT);
 W[5]=LM_PA_NOQBRSM1;return EXEC_OK;
}
/* 16,2224 (CHEKBITS's return): CA CDUX; TS DAPTREG4; CA CDUY; TS DAPTREG5;
 * CA CDUZ; TS DAPTREG6; CA CDUXD; EXTEND; MSU DELCDUX; TC 1STOTWOS;
 * TS CDUXD; (Y, Z alike); EXTEND; DIM TCP; EXTEND; DIM TCQR;
 * TCF PAXFILT. PAXFILT: CA CALLGMBL; MASK RCSFLAGS; CCS A;
 * TC ACDT+C12. */
static enum exec_status afterchek_body(agc_image *s){
 ai_ca(s,CDUX);(void)ai_ts(s,DAPTREG4);ai_ca(s,CDUY);(void)ai_ts(s,DAPTREG5);ai_ca(s,CDUZ);(void)ai_ts(s,DAPTREG6);
 ai_ca(s,CDUXD);msu(s,DELCDUX);onetotwos(s,02236u);(void)ai_ts(s,CDUXD);
 ai_ca(s,CDUYD);msu(s,DELCDUY);onetotwos(s,02243u);(void)ai_ts(s,CDUYD);
 ai_ca(s,CDUZD);msu(s,DELCDUZ);onetotwos(s,02250u);(void)ai_ts(s,CDUZD);
 ai_dim(s,TCP);ai_dim(s,TCQR);
 ai_caf(s,CALLGMBL);ai_mask(s,RCSFLAGS);
 if(ai_ccs(s,0u)==0u){LM_PA_PATH(109);return leave(s,02653u,LM_PA_ACDTC12);}
 LM_PA_PATH(110);return afteracdt_body(s);
}
/* PAXIS: CA MS100; ADS TIME5; LXCH BANKRUPT; EXTEND; QXCH QRUPT;
 * CCS DAPZRUPT; TC BAILOUT; OCT 02000 (TC CHEKBITS); TC CHEKBITS. */
static enum exec_status paxis(const agc_native_program *p,agc_image *s,void *v){uint8_t c;(void)p;
 if(!entry(s,v))return EXEC_OUT_OF_SCOPE;
 ai_caf(s,MS100);ai_ads(s,TIME5);ai_lxch(s,BANKRUPT);ai_qxch(s,QRUPT);
 c=ai_ccs(s,DAPZRUPT);LM_PA_PATH(111u+c);
 if(c==0u)return leave(s,02222u,LM_PA_BAILOUT);
 if(c==1u)return leave(s,02223u,02000u); /* the word OCT 02000 runs as TC CHEKBITS */
 if(c==2u)return leave(s,02224u,LM_PA_CHEKBITS);
 return afterchek_body(s);
}
static enum exec_status chekbits2(const agc_native_program *p,agc_image *s,void *v){(void)p;
 if(!entry(s,v))return EXEC_OUT_OF_SCOPE;
 return leave(s,02224u,LM_PA_CHEKBITS);
}
static enum exec_status afterchek(const agc_native_program *p,agc_image *s,void *v){(void)p;
 if(!entry(s,v))return EXEC_OUT_OF_SCOPE;
 return afterchek_body(s);
}
static enum exec_status afteracdt(const agc_native_program *p,agc_image *s,void *v){(void)p;
 if(!entry(s,v))return EXEC_OUT_OF_SCOPE;
 return afteracdt_body(s);
}
/* SUPERJOB: TCF RATELOOP (RESUME of the job PAXFILT set up). */
static enum exec_status superjob(const agc_native_program *p,agc_image *s,void *v){(void)p;
 if(!entry(s,v))return EXEC_OUT_OF_SCOPE;
 W[5]=LM_PA_RATELOOP;return EXEC_OK;
}
/* CALCPERR, entered by TC or IBNKCALL with the return in Q. */
static enum exec_status calcperr(const agc_native_program *p,agc_image *s,void *v){(void)p;
 if(!entry(s,v)||!tc_word(W[QREG]))return EXEC_OUT_OF_SCOPE;
 W[5]=W[QREG];calcperr_body(s);return EXEC_OK;
}
static int flow_ok(agc_image *s,void *v){return entry(s,v)&&ai_word(s,OLDPMIN)!=077777u;}
#define FLOW(name,body) static enum exec_status name(const agc_native_program *p,agc_image *s,void *v){(void)p; \
 if(!flow_ok(s,v))return EXEC_OUT_OF_SCOPE; \
 return body;}
FLOW(backp,rateloop(s))
FLOW(traps_e,traps(s))
FLOW(qrate_e,qrate(s))
FLOW(rrate_e,rrate(s))
FLOW(pretimck_e,pretimck(s,v))
FLOW(chkvisfz_e,chkvisfz(s,v))
FLOW(abortyzret,manual(s,v,LM_PA_ABORTYZ2))
FLOW(tsnextp1,manual(s,v,LM_PA_TSNEXTP1))
FLOW(tsnextp,manual(s,v,LM_PA_TSNEXTP))
FLOW(detentck_e,detentck(s,v))
FLOW(raterror_e,raterror(s,v))
FLOW(zattret,urgency(s,v,LM_PA_ZATTRET))
FLOW(pjetslec6,jetselect(s,v,AT_PJETSLEC6))
FLOW(pjetslec1,jetselect(s,v,AT_PJETSLEC1))
FLOW(jtlstret,jtlstret_body(s,v))
FLOW(jetsoffret,jetsoffret_body(s))
FLOW(purgency,urgency(s,v,LM_PA_PURGENCY))
FLOW(purgency4,urgency(s,v,LM_PA_PURGENCY4))
FLOW(spsret,jetselect(s,v,AT_SPSRET))
FLOW(tjetret,jetselect(s,v,AT_TJETRET))
FLOW(selalarm,jetsoff(s))
static enum exec_status writepret(const agc_native_program *p,agc_image *s,void *v){(void)p;
 if(!flow_ok(s,v)||W[ai_ea(s,ROTINDEX)]>1u)return EXEC_OUT_OF_SCOPE;
 return writepret_body(s,v);
}
const agc_site_operand lm_p_axis_sites[LM_PA_SITES]={
#include "lm_p_axis_sites.inc"
};
/* KAOS DEC 60, 1/10SEC OCT 1, NORMSCL OCT 266, -100MS DEC -.1,
 * PSKIPADR GENADR SKIPPAXS. */
static int pa_data(uint16_t physical,agc_word *word){
 static const uint16_t at[5]={P16(02623u),P16(03132u),P16(03614u),P16(03615u),P16(03623u)};
 static const agc_word w[5]={074u,1u,0266u,074631u,02734u};unsigned i;
 for(i=0;i<5u;++i)if(at[i]==physical){*word=w[i];return 1;}
 return 0;
}
bool lm_p_axis_data(uint16_t physical,agc_word *word){return word&&pa_data(physical,word);}
static const agc_routine_entry entries[28]={
 {P16(LM_PA_PAXIS),paxis},{P16(LM_PA_CHEKBITS2),chekbits2},{P16(LM_PA_AFTERCHEK),afterchek},{P16(LM_PA_BACKP),backp},
 {P16(LM_PA_TRAPS),traps_e},{P16(LM_PA_QRATE),qrate_e},{P16(LM_PA_RRATE),rrate_e},{P16(LM_PA_AFTERACDT),afteracdt},{P16(LM_PA_SUPERJOB),superjob},
 {P16(LM_PA_PRETIMCK),pretimck_e},{P16(LM_PA_CHKVISFZ),chkvisfz_e},{P16(LM_PA_ABORTYZ2),abortyzret},
 {P16(LM_PA_TSNEXTP1),tsnextp1},{P16(LM_PA_TSNEXTP),tsnextp},{P16(LM_PA_DETENTCK),detentck_e},
 {P16(LM_PA_ZATTRET),zattret},{P16(LM_PA_RATERROR),raterror_e},{P16(LM_PA_PJETSLEC6),pjetslec6},
 {P16(LM_PA_PJETSLEC1),pjetslec1},{P16(LM_PA_WRITEPRET),writepret},{P16(LM_PA_JTLSTRET),jtlstret},
 {P16(LM_PA_JETSOFFRET),jetsoffret},{P16(LM_PA_CALCPERR),calcperr},{P16(LM_PA_PURGENCY),purgency},
 {P16(LM_PA_PURGENCY4),purgency4},{P16(LM_PA_SPSRET),spsret},{P16(LM_PA_TJETRET),tjetret},
 {P16(LM_PA_SELALARM),selalarm}};
const agc_routine_table lm_p_axis_routines={entries,28};
