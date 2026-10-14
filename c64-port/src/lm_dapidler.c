/* SPDX-License-Identifier: MIT
 * Luminary099 DAPIDLER_PROGRAM.agc, all of it (bank 16, 02000-02212;
 * bank 20, 02226-02446), as routines of the control-transfer ABI. Each
 * source instruction is one agc_image_ops call (yaAGC-qualified register
 * effects) or, for MSU, DTCB and the channel orders, a helper here with
 * yaAGC's effect, named with its erasable address, channel or the source's
 * fixed constant. CCS exits land where the original's do, including the
 * ones no admitted input reaches. */
#include "lm_dapidler.h"
#include "agc_image_ops.h"
/* Every instruction runs on the one canonical image through
 * include/agc_fast_redirect.h: a plain-cell operand under this file's
 * EBANK 6 goes to the fast helper (include/agc_fast_ops.h), any other
 * operand to a one-argument wrapper of the agc_image_ops helper; both have
 * its effects. -DLM_DI_GENERIC builds the same source on agc_image_ops alone. */
#ifdef LM_DI_GENERIC
#define AFR_GENERIC
#endif
#define AFR_EBN (LM_DI_EB>>8)
#include "agc_fast_redirect.h"
#include "lm_dap_shared.h"
#define W AFR_S(s)->words
/* Branch coverage hook for the host test (empty in flight builds). */
#ifdef LM_DI_COUNT_PATHS
extern unsigned lm_di_paths[];
#define LM_DI_PATH(n) ((void)++lm_di_paths[n])
#else
#define LM_DI_PATH(n) ((void)0)
#endif
#define P16(z) (uint16_t)(016u*1024u+((z)-02000u))
#define P20(z) (uint16_t)(020u*1024u+((z)-02000u))
/* Erasable: unswitched, and EBANK6 (EBANK= AOSQ, OMEGAP, AK). */
#define QREG 2u
#define QRUPT 012u
#define BANKRUPT 016u
#define ZEROREG 7u
#define TIME5 030u
#define TIME6 031u
#define CDUX 032u
#define CDUY 033u
#define CDUZ 034u
#define CDUXCMD 050u
#define CDUYCMD 051u
#define CDUZCMD 052u
#define ITEMP1 061u     /* = T5TEMP */
#define T5TEMP 061u
#define DINDX 063u
#define FLAGWRD0 074u
#define DAPBOOLS 0111u
#define CPHI 0321u
#define CTHETA 0322u
#define CPSI 0323u
#define RCSFLAGS 01273u
#define T5ADR 01274u
#define IMODES33 01303u
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
#define TRAPEDP 01430u
#define TRAPEDQ 01431u
#define TRAPEDR 01432u
#define NPTRAPS 01433u
#define NQTRAPS 01434u
#define NRTRAPS 01435u
#define OLDXFORP 01440u
#define OLDZFORQ 01442u
#define QERROR 01450u
#define RERROR 01452u
#define OLDPMIN 01460u
#define OLDQRMIN 01461u
#define PERROR 01464u
#define NXT6ADR 01465u
#define T6NEXT 01466u
#define T6NEXT1 01467u   /* T6NEXT +1 */
#define T6FURTHA 01470u
#define T6FURTHA1 01471u /* T6FURTHA +1 */
#define NEXTP 01472u
#define NEXTU 01473u
#define NEXTV 01474u
#define NEGUQ 01501u
#define ALLOWGTS 01502u
#define NEGUR 01503u
#define QACCDOT 01510u
#define RACCDOT 01512u
#define TJP 01524u
#define TJU 01525u
#define TJV 01526u
#define SKIPU 01535u
#define SKIPV 01536u
#define AOSQ 01537u
#define AOSQ1 01540u     /* AOSQ +1 */
#define AOSR 01541u
#define AOSR1 01542u     /* AOSR +1 */
#define AOSQTERM 01545u
#define AOSRTERM 01546u
#define COTROLER 01631u
#define QGIMTIMR 01632u
#define INGTS 01633u
#define RGIMTIMR 01634u
#define DAPZRUPT 01757u
#define AK 01761u
#define AK1 01762u       /* AK +1 */
#define AK2 01763u       /* AK +2 */
#define EDRIVEX 01764u
#define EDRIVEY 01765u
#define EDRIVEZ 01766u
#define PJETCTR 01767u
#define UJETCTR 01770u
#define VJETCTR 01771u
#define LCHAN 1u
/* Fixed constants: BIT13-14 04355, BIT6 04746, BIT3 = ACCSOKAY = FOUR
 * 04751, BIT10 04742, BIT13 04737, PRIO27 07715, ZERO 04755, BIT12 04740,
 * POSMAX 04733, TEN 04363, TWO = BIT2 04752, MS100 07727, BIT4 = DSPLYALT =
 * NEEDLBIT 04750, SIX 06242, 13,14,15 07740; this file's MANFLAG (16,2202),
 * BGIM23 (16,2203), 2CADRs IDLERADR (16,2155), PAXADIDL (16,2204), DSPCADR
 * (16,2211), MORECADR (20,2445), DACLIMIT (20,2431-2433), ONETENTH
 * (20,2434); LIMITS -1/0/+1 (04733-04735). */
#define BIT13_14 030000u
#define BIT6 040u
#define BIT3 4u
#define ACCSOKAY 4u
#define FOUR 4u
#define BIT10 01000u
#define BIT13 010000u
#define PRIO27 027000u
#define ZERO 0u
#define BIT12 04000u
#define POSMAX 037777u
#define TEN 012u
#define TWO 2u
#define BIT2 2u
#define MS100 037766u
#define DSPLYALT 010u
#define NEEDLBIT 010u
#define SIX 6u
#define BITS131415 070000u
#define MANFLAG 03021u
#define BGIM23 07400u
#define ONETENTH 03146u
#define RESUME 05270u
#define IBNKCALL 04674u
#define NOVAC 05072u

static int tc_word(agc_register w){return w>=02000u&&w<=07777u;}
static int entry(const agc_image *s,const void *io,agc_word fb){AFR_BIND(s);return s&&io&&(W[4]&076000u)==fb&&W[3]==LM_DI_EB;}
static agc_word oc(agc_register a){return (agc_word)((a&037777u)|((a>>1)&040000u));}
/* The signed value of the 16-bit A (+-1 after a TS overflow). */
static int sa(const agc_image *s){agc_register a=W[0];return a&0100000u?-(int)(~a&077777u):(int)(a&077777u);}
/* Channel orders on a 15-bit channel, as yaAGC's: READ, RAND, WAND, WOR,
 * WRITE; RXOR on channel 1 (L) works on the raw 16-bit registers. */
static void rd(agc_image *s,agc_word chan){W[0]=agc_extend(chan);}
LM_DAP_SHARED void rand(agc_image *s,agc_word chan){W[0]=agc_extend((agc_word)(oc(W[0])&chan));}
LM_DAP_SHARED void wand(agc_image *s,agc_word *chan){agc_word v=(agc_word)(oc(W[0])&*chan);*chan=v;W[0]=agc_extend(v);}
static void wor(agc_image *s,agc_word *chan){agc_word v=(agc_word)((oc(W[0])|*chan)&077777u);*chan=v;W[0]=agc_extend(v);}
static void wr(agc_image *s,agc_word *chan){*chan=(agc_word)(oc(W[0])&077777u);}
static void rxor_l(agc_image *s){W[0]=(agc_register)((W[0]^W[LCHAN])&0177777u);}
/* MSU K (K above the registers): two's-complement difference of the
 * overflow-corrected A and K, returned in ones' complement. */
LM_DAP_SHARED void msu(agc_image *s,unsigned k){unsigned ui=oc(W[0])&077777u,uj=~(unsigned)ai_word(s,k)&077777u,d=ui+uj+1u;
 if(d&040000u){d|=0100000u;--d;}
 W[0]=agc_extend((agc_word)(d&077777u));
}
/* DTCB (DXCH Z) executed at `at`: Z,BB <-> A,L. */
static void dtcb(agc_image *s,agc_word at){agc_word z=agc_correct(W[0]),bb=agc_correct(W[1]);
 W[0]=(agc_word)(at+1u);W[1]=agc_extend(W[6]);W[5]=z;
 W[6]=(agc_word)(bb&076007u);W[3]=(agc_word)((bb&7u)<<8);W[4]=(agc_word)(bb&076000u);
}
static enum exec_status transfer(agc_image *s,agc_word ret,agc_word z){ai_tc(s,ret);W[5]=z;return EXEC_OK;}

/* SETTIME5: CAF MS100; TS TIME5; TCF RESUME. */
static enum exec_status settime5(agc_image *s){ai_caf(s,MS100);(void)ai_ts(s,TIME5);W[5]=RESUME;return EXEC_OK;}
/* MOREIDLE: TC IBNKCALL; CADR QERRCALC. */
static enum exec_status moreidle(agc_image *s){return transfer(s,02160u,IBNKCALL);}
/* SHUTDOWN: EXTEND; DCA IDLERADR; DXCH T5ADR; CAF ZERO; TS NEXTP; TS NEXTU;
 * TS NEXTV; EXTEND; WRITE CHAN5; EXTEND; WRITE CHAN6; CS BGIM23; EXTEND;
 * WAND CHAN12; TCF SETTIME5. */
static enum exec_status shutdown(agc_image *s,p66_bias_io *io){(void)io;static const agc_word idleradr[2]={02024u,034106u};
 ai_dcaf(s,idleradr);ai_dxch(s,T5ADR);ai_caf(s,ZERO);(void)ai_ts(s,NEXTP);(void)ai_ts(s,NEXTU);(void)ai_ts(s,NEXTV);
 wr(s,&lmf_hw->channel5);wr(s,&lmf_hw->channel6);ai_csf(s,BGIM23);wand(s,&lmf_hw->channel12);
 return settime5(s);
}
/* CHEKBITS: READ CHAN31; COM; MASK BIT13-14; BZF MOREIDLE; CS IMODES33;
 * MASK BIT6; CCS A; TCF JUMPDSP; CS RCSFLAGS; MASK BIT3; ADS RCSFLAGS;
 * TCF SHUTDOWN. JUMPDSP: EXTEND; DCA DSPCADR; DTCB. */
static enum exec_status chekbits_body(agc_image *s,p66_bias_io *io){static const agc_word dspcadr[2]={02226u,040106u};uint8_t c;
 rd(s,lmf_hw->channel31);ai_com(s);ai_maskf(s,BIT13_14);
 if(ai_bzf(s)){LM_DI_PATH(0);return moreidle(s);}
 ai_cs(s,IMODES33);ai_maskf(s,BIT6);
 c=ai_ccs(s,0u);LM_DI_PATH(1u+c);
 if(c==0u){ai_dcaf(s,dspcadr);dtcb(s,02210u);return EXEC_OK;}
 if(c==1u)ai_cs(s,RCSFLAGS);
 if(c<=2u)ai_maskf(s,BIT3);
 ai_ads(s,RCSFLAGS);return shutdown(s,io);
}
static enum exec_status chekbits(const agc_native_program *p,agc_image *s,void *io){AFO_FIX(s);if(!LMF_HW_SET(p))return EXEC_OUT_OF_SCOPE;(void)p;
 if(!entry(s,io,LM_DI_FB16)||!tc_word(W[2]))return EXEC_OUT_OF_SCOPE;
 return chekbits_body(s,io);}
/* CHECKUP: TC CHEKBITS. */
static enum exec_status checkup_body(agc_image *s,p66_bias_io *io){ai_tc(s,02042u);return chekbits_body(s,io);}
static enum exec_status checkup(const agc_native_program *p,agc_image *s,void *io){AFO_FIX(s);if(!LMF_HW_SET(p))return EXEC_OUT_OF_SCOPE;(void)p;
 if(!entry(s,io,LM_DI_FB16))return EXEC_OUT_OF_SCOPE;
 return checkup_body(s,io);}
/* DAPIDLER: LXCH BANKRUPT; EXTEND; QXCH QRUPT; CA RCSFLAGS; MASK BIT13;
 * CCS A; TCF CHECKUP; CA BIT13; ADS RCSFLAGS; CAF PRIO27; TC NOVAC;
 * 2CADR 1/ACCSET. from = 0 enters at DAPIDLER, 1 at its EXTEND. */
static enum exec_status dapidler_body(agc_image *s,p66_bias_io *io,uint8_t from){uint8_t c;
 if(from==0u)ai_lxch(s,BANKRUPT);
 ai_qxch(s,QRUPT);ai_ca(s,RCSFLAGS);ai_maskf(s,BIT13);
 c=ai_ccs(s,0u);LM_DI_PATH(5u+c);
 if(c==0u)return checkup_body(s,io);
 if(c==1u)ai_caf(s,BIT13);
 if(c<=2u)ai_ads(s,RCSFLAGS);
 ai_caf(s,PRIO27);return transfer(s,02037u,NOVAC);
}
static enum exec_status dapidler(const agc_native_program *p,agc_image *s,void *io){AFO_FIX(s);if(!LMF_HW_SET(p))return EXEC_OUT_OF_SCOPE;(void)p;
 if(!entry(s,io,LM_DI_FB16))return EXEC_OUT_OF_SCOPE;
 return dapidler_body(s,io,0u);}
/* CHEKMORE: CAF BIT10; EXTEND; RAND CHAN30; CCS A; TCF MOREIDLE; RETURN
 * (then DAPIDLER and its EXTEND for the negative exits). */
static enum exec_status chekmore(const agc_native_program *p,agc_image *s,void *v){AFO_FIX(s);if(!LMF_HW_SET(p))return EXEC_OUT_OF_SCOPE;p66_bias_io *io=v;uint8_t c;(void)p;
 if(!entry(s,io,LM_DI_FB16)||!tc_word(W[2]))return EXEC_OUT_OF_SCOPE;
 ai_caf(s,BIT10);rand(s,lmf_hw->channel30);
 c=ai_ccs(s,0u);LM_DI_PATH(9u+c);
 if(c==0u)return moreidle(s);
 if(c==1u){W[5]=W[QREG];ai_tc(s,QREG+1u);return EXEC_OK;} /* RETURN: Z = Q, Q = 3 */
 return dapidler_body(s,io,(uint8_t)(c-2u));
}
/* 16,2042: CAE DAPBOOLS; MASK ACCSOKAY; EXTEND; BZF MOREIDLE; STARTDAP
 * TC IBNKCALL; FCADR ZATTEROR. */
static enum exec_status accsokay(const agc_native_program *p,agc_image *s,void *io){AFO_FIX(s);if(!LMF_HW_SET(p))return EXEC_OUT_OF_SCOPE;(void)p;
 if(!entry(s,io,LM_DI_FB16))return EXEC_OUT_OF_SCOPE;
 ai_ca(s,DAPBOOLS);ai_maskf(s,ACCSOKAY);
 if(ai_bzf(s)){LM_DI_PATH(13);return moreidle(s);}
 LM_DI_PATH(14);return transfer(s,02047u,IBNKCALL);
}
/* 16,2050: the DAP initialization (CAF ZERO; TS TJP .. VJETCTR; RCSFLAGS
 * by MANFLAG and BIT12; OLDXFORP, OLDZFORQ; SKIPU/V; TIME6, T6NEXT,
 * T6FURTHA, NXT6ADR; NEXTP/U/V; DAPZRUPT; NPTRAPS..; T5ADR = PAXIS), then
 * SETTIME5. */
static enum exec_status startdap(const agc_native_program *p,agc_image *s,void *io){AFO_FIX(s);if(!LMF_HW_SET(p))return EXEC_OUT_OF_SCOPE;(void)p;
 static const agc_word paxadidl[2]={02213u,034106u};
 static const agc_word zeroed[31]={TJP,TJU,TJV,OMEGAP,OMEGAQ,OMEGAR,TRAPEDP,TRAPEDQ,TRAPEDR,AOSQ,AOSQ1,AOSR,AOSR1,
  ALPHAQ,ALPHAR,NEGUQ,NEGUR,AOSQTERM,AOSRTERM,QACCDOT,RACCDOT,ALLOWGTS,COTROLER,INGTS,QGIMTIMR,RGIMTIMR,
  OLDPMIN,OLDQRMIN,PJETCTR,UJETCTR,VJETCTR};
 static const agc_word t6[6]={T6NEXT1,T6FURTHA1,NXT6ADR,NEXTP,NEXTU,NEXTV};unsigned i;
 if(!entry(s,io,LM_DI_FB16))return EXEC_OUT_OF_SCOPE;
 ai_caf(s,ZERO);for(i=0;i<31u;++i)(void)ai_ts(s,zeroed[i]);
 ai_csf(s,MANFLAG);ai_mask(s,RCSFLAGS);(void)ai_ts(s,RCSFLAGS);
 ai_dca(s,CDUX);ai_dxch(s,OLDXFORP);ai_ca(s,CDUZ);(void)ai_ts(s,OLDZFORQ);
 ai_cs(s,RCSFLAGS);ai_maskf(s,BIT12);ai_ads(s,RCSFLAGS);
 ai_caf(s,FOUR);(void)ai_ts(s,SKIPU);(void)ai_ts(s,SKIPV);
 ai_caf(s,POSMAX);(void)ai_ts(s,TIME6);(void)ai_ts(s,T6NEXT);(void)ai_ts(s,T6FURTHA);
 ai_caf(s,ZERO);for(i=0;i<6u;++i)(void)ai_ts(s,t6[i]);
 ai_csf(s,TEN);(void)ai_ts(s,DAPZRUPT);
 ai_caf(s,TWO);(void)ai_ts(s,NPTRAPS);(void)ai_ts(s,NQTRAPS);(void)ai_ts(s,NRTRAPS);
 ai_dcaf(s,paxadidl);ai_dxch(s,T5ADR);
 return settime5(s);
}
/* 16,2161: TC IBNKCALL; CADR CALCPERR. */
static enum exec_status calcperr(const agc_native_program *p,agc_image *s,void *io){AFO_FIX(s);if(!LMF_HW_SET(p))return EXEC_OUT_OF_SCOPE;(void)p;
 if(!entry(s,io,LM_DI_FB16))return EXEC_OUT_OF_SCOPE;
 return transfer(s,02162u,IBNKCALL);}
/* 16,2163 SHUTDOWN. */
static enum exec_status shutdown_entry(const agc_native_program *p,agc_image *s,void *io){AFO_FIX(s);if(!LMF_HW_SET(p))return EXEC_OUT_OF_SCOPE;(void)p;
 if(!entry(s,io,LM_DI_FB16))return EXEC_OUT_OF_SCOPE;
 return shutdown(s,io);}

/* OVERSUB2: TS 7; TC Q; INDEX A; CS LIMITS; TC Q. Entered by TC with
 * return ret: A unchanged, or POSMAX/NEGMAX on overflow. */
static void oversub2(agc_image *s,agc_word ret){static const agc_word limits[3]={037777u,057777u,040000u}; /* LIMITS -1, 0, +1 */
 ai_tc(s,ret);
 if(ai_ts(s,ZEROREG)){LM_DI_PATH(15);ai_csf(s,limits[sa(s)+1]);}
 ai_tc(s,QREG+1u); /* TC Q: Z = ret, Q = 3 */
}
/* RETNMORE: EXTEND; DCA MORECADR; DTCB. */
static enum exec_status retnmore(agc_image *s){static const agc_word morecadr[2]={02016u,034106u};
 ai_dcaf(s,morecadr);dtcb(s,02444u);return EXEC_OK;}
/* ALTDSPLY .. RETNMORE. */
static enum exec_status altdsply(const agc_native_program *p,agc_image *s,void *v){AFO_FIX(s);if(!LMF_HW_SET(p))return EXEC_OUT_OF_SCOPE;p66_bias_io *io=v;uint8_t c;
 static const agc_word daclimit[3]={077177u,037200u,0600u}; /* DEC -384, DACLIMIT 16000, 384 B-14 */
 (void)p;
 if(!entry(s,io,LM_DI_FB20))return EXEC_OUT_OF_SCOPE;
 /* CA RCSFLAGS; TS L; CA DSPLYALT; EXTEND; RXOR LCHAN; TS RCSFLAGS;
  * MASK DSPLYALT; CCS A; TCF NEEDLER; CAE FLAGWRD0; MASK NEEDLBIT; CCS A;
  * TCF DSPLYTOT; CS QERROR; TS AK +1; CS RERROR; TS AK +2; CS PERROR;
  * XCH AK; TCF RETNMORE. */
 ai_ca(s,RCSFLAGS);(void)ai_ts(s,LCHAN);ai_caf(s,DSPLYALT);rxor_l(s);(void)ai_ts(s,RCSFLAGS);ai_maskf(s,DSPLYALT);
 c=ai_ccs(s,0u);LM_DI_PATH(16u+c);
 if(c==0u)goto needler;
 if(c==1u)ai_ca(s,FLAGWRD0);
 if(c<=2u)ai_maskf(s,NEEDLBIT);
 c=ai_ccs(s,0u);LM_DI_PATH(20u+c);
 if(c==0u)goto dsplytot;
 if(c==1u)ai_cs(s,QERROR);
 if(c<=2u)(void)ai_ts(s,AK1);
 ai_cs(s,RERROR);(void)ai_ts(s,AK2);ai_cs(s,PERROR);ai_xch(s,AK);
 return retnmore(s);
dsplytot: /* EXTEND; QXCH ITEMP1; CA CTHETA; EXTEND; MSU CDUY; TS AK; EXTEND;
  * MP M21; XCH AK +1; CA CPSI; EXTEND; MSU CDUZ; TS AK +2; EXTEND; MP M22;
  * AD AK +1; TC OVERSUB2; TS AK +1; CA AK; EXTEND; MP M31; XCH AK +2;
  * EXTEND; MP M32; AD AK +2; TC OVERSUB2; TS AK +2; CA AK; EXTEND; MP M11;
  * XCH AK; CAE CPHI; EXTEND; MSU CDUX; AD AK; TC OVERSUB2; TS AK; EXTEND;
  * QXCH ITEMP1; TCF RETNMORE. */
 ai_qxch(s,ITEMP1);
 ai_ca(s,CTHETA);msu(s,CDUY);(void)ai_ts(s,AK);ai_mp(s,M21);ai_xch(s,AK1);
 ai_ca(s,CPSI);msu(s,CDUZ);(void)ai_ts(s,AK2);ai_mp(s,M22);ai_ad(s,AK1);oversub2(s,02273u);(void)ai_ts(s,AK1);
 ai_ca(s,AK);ai_mp(s,M31);ai_xch(s,AK2);ai_mp(s,M32);ai_ad(s,AK2);oversub2(s,02304u);(void)ai_ts(s,AK2);
 ai_ca(s,AK);ai_mp(s,M11);ai_xch(s,AK);ai_ca(s,CPHI);msu(s,CDUX);ai_ad(s,AK);oversub2(s,02316u);(void)ai_ts(s,AK);
 ai_qxch(s,ITEMP1);
 return retnmore(s);
needler: /* CA RCSFLAGS; MASK SIX; EXTEND; BZF NEEDLES3; MASK BIT3; EXTEND;
  * BZF NEEDLER2; CS BIT6; EXTEND; WAND CHAN12; NEEDLE11 ... */
 ai_ca(s,RCSFLAGS);ai_maskf(s,SIX);
 if(ai_bzf(s))goto needles3;
 ai_maskf(s,BIT3);
 if(ai_bzf(s)){
  /* NEEDLER2: CAF BIT6; EXTEND; WOR CHAN12; CS SIX; MASK RCSFLAGS;
   * TS RCSFLAGS; TCF RETNMORE. */
  LM_DI_PATH(24);ai_caf(s,BIT6);wor(s,&lmf_hw->channel12);ai_csf(s,SIX);ai_mask(s,RCSFLAGS);(void)ai_ts(s,RCSFLAGS);
  return retnmore(s);
 }
 LM_DI_PATH(25);ai_csf(s,BIT6);wand(s,&lmf_hw->channel12);
 /* NEEDLE11: CS ZERO; TS AK; TS AK1; TS AK2; TS EDRIVEX; TS EDRIVEY;
  * TS EDRIVEZ; TS CDUXCMD; TS CDUYCMD; TS CDUZCMD; CS SIX; MASK RCSFLAGS;
  * AD BIT2; TS RCSFLAGS; TCF RETNMORE. */
 ai_csf(s,ZERO);(void)ai_ts(s,AK);(void)ai_ts(s,AK1);(void)ai_ts(s,AK2);
 (void)ai_ts(s,EDRIVEX);(void)ai_ts(s,EDRIVEY);(void)ai_ts(s,EDRIVEZ);
 (void)ai_ts(s,CDUXCMD);(void)ai_ts(s,CDUYCMD);(void)ai_ts(s,CDUZCMD);
 ai_csf(s,SIX);ai_mask(s,RCSFLAGS);ai_adf(s,BIT2);(void)ai_ts(s,RCSFLAGS);
 return retnmore(s);
needles3: /* CAF BIT6; EXTEND; RAND CHAN12; CCS A; TCF NEEDLES; CS RCSFLAGS;
  * MASK BIT3; ADS RCSFLAGS; TCF RETNMORE. */
 ai_caf(s,BIT6);rand(s,lmf_hw->channel12);
 c=ai_ccs(s,0u);LM_DI_PATH(26u+c);
 if(c!=0u){
  if(c==1u)ai_cs(s,RCSFLAGS);
  if(c<=2u)ai_maskf(s,BIT3);
  ai_ads(s,RCSFLAGS);return retnmore(s);
 }
 /* NEEDLES: CAF TWO; DACLOOP TS DINDX; CS ONETENTH; EXTEND; INDEX DINDX;
  * MP AK; TS L; CCS A; CA DACLIMIT; TCF +2; CS DACLIMIT; AD L;
  * TS T5TEMP; TCF +4; INDEX A; CAF DACLIMIT; TS L; INDEX DINDX;
  * CS EDRIVEX; AD L; INDEX DINDX; ADS CDUXCMD; INDEX DINDX; LXCH EDRIVEX;
  * CCS DINDX; TCF DACLOOP; CAF 13,14,15; EXTEND; WOR CHAN14;
  * TCF RETNMORE. */
 ai_caf(s,TWO);
 for(;;){unsigned d;
  (void)ai_ts(s,DINDX);d=ai_word(s,DINDX);
  ai_csf(s,ONETENTH);ai_mp(s,AK+d);(void)ai_ts(s,LCHAN);
  c=ai_ccs(s,0u);LM_DI_PATH(30u+c);
  if(c==0u)ai_caf(s,daclimit[1]);
  if(c==2u)ai_csf(s,daclimit[1]);
  ai_ad(s,LCHAN);
  if(ai_ts(s,T5TEMP)){LM_DI_PATH(34);ai_caf(s,daclimit[sa(s)+1]);(void)ai_ts(s,LCHAN);}
  ai_cs(s,EDRIVEX+d);ai_ad(s,LCHAN);ai_ads(s,CDUXCMD+d);ai_lxch(s,EDRIVEX+d);
  /* DINDX is 2, 1 or 0 here (TS of CAF TWO or of the CCS below). */
  c=ai_ccs(s,DINDX);
  if(c!=0u)break;
 }
 LM_DI_PATH(34u+c);
 if(c==3u)return transfer(s,02430u,05014u); /* past the EXTEND: WOR's word runs as TC 05014 */
 if(c==1u)ai_caf(s,BITS131415);
 wor(s,&lmf_hw->channel14);
 return retnmore(s);
}
static const agc_routine_entry entries[9]={
 {P16(02000u),chekbits},{P16(02016u),chekmore},{P16(02024u),dapidler},{P16(02041u),checkup},{P16(02042u),accsokay},
 {P16(02050u),startdap},{P16(02161u),calcperr},{P16(02163u),shutdown_entry},{P20(02226u),altdsply}};
const agc_routine_table lm_dapidler_routines={entries,9};
/* The entries one 100 ms DAP pass reaches in attitude hold or jet firing
 * (Lane F's RCS DAP cycle trace, /mnt/project-files/lane-f/dap-cycle-trace.md):
 * the pinned DAP set links this table, so the other entries stay in the
 * file's packed module (docs/FLIGHT-STORE.md). */
static const agc_routine_entry hot_entries[3]={
 {P16(02000u),chekbits},{P16(02016u),chekmore},{P20(02226u),altdsply}};
const agc_routine_table lm_dapidler_hot_routines={hot_entries,3};
