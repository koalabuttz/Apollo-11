/* SPDX-License-Identifier: MIT */
/* Comanche055 FRESH_START_AND_RESTART, bank 04: GOP00FIX, V37 and its
 * major-mode table search (CHECKTAB), R00TOP00, ISITP00, ISSERVON, CANV37,
 * ROC/R00, POOH, SEUDOP00, GROUPKIL, GOMOD/GOGOPROG, the rendezvous checks
 * (RENDV00, RENDN00, P00FIZZ, KILL20, REV37, STATQUO), NOUVEAU, V37NONO,
 * V37XEQ/V37XEQC and INITSUB/RAKE; and bank 20: VAC5STOR. One
 * agc_image_ops call per original instruction, under labels L<address>.
 * Every TC out of the file leaves through OUT; the routine resumes at the
 * return point as its own entry. INITSUB returns with TC MPAC +1, which
 * executes the word its QXCH stored (2001 or 2146) as TC. The PREMM1,
 * FCADRMM1 and DNLADMM1 reads take only indices inside their 33-word
 * tables; MMNUMBER is never negative (pinball's verb 37 mode). */
#include "cm_v37.h"
#include "agc_image_ops.h"
#include "cm_v37_operands.inc"

#include "agc_image_w.h"
#define B04(z) (uint16_t)(04u*1024u+((z)-02000u))
#define B20(z) (uint16_t)(020u*1024u+((z)-02000u))
#define A 0u
#define L 1u
#define Q 2u
#define EBANK 3u
#define CYR 020u
#define ITEMP1 061u
#define ITEMP2 062u
#define NEWPRIO 063u
#define NEWLOC 065u
#define NEWJOB 067u
#define FLAGWRD0 074u
#define FLAGWRD1 075u
#define FLAGWRD2 076u
#define FLAGWRD6 0102u
#define FLAGWRD7 0103u
#define FLAGWRD9 0105u
#define MPAC 0154u
#define LOC 0164u
#define PRIORITY 0167u
#define DNLSTCOD 0332u
#define RESTREG 0366u
#define TEMPFLSH 0374u
#define VAC5 0661u
#define MPHASE1 0752u      /* -PHASE1 */
#define PHASE1 0753u
#define MPHASE2 0754u      /* -PHASE2 */
#define MPHASE4 0760u      /* -PHASE4 */
#define MINDEX CM_V37_MINDEX
#define MMNUMBER CM_V37_MMNUMBER
#define MODREG 01011u
#define PHSPRDT2 01056u
#define MMTEMP 01060u
#define BASETEMP 01061u
#define PHSPRDT4 01062u
#define EBANKTEM 01071u
#define OPTIND 01303u
#define IMODES30 01320u
#define CSMMASS 01474u     /* E6 */
#define MASSTMP 01652u     /* E6 */
/* Entries outside the file. */
#define BANKCALL 04662u
#define POSTJUMP 04701u
#define IBNKCALL 04740u
#define SWRETURN 04675u
#define STOPRATE 04526u
#define CLEARMRK 05536u
#define ALARM 05650u
#define RELDSP 04473u
#define PHASCHNG 05412u
#define INTPRET 06006u
#define DOWNFLAG 05560u
#define ENDOFJOB 05217u
#define FALTON 04400u
#define SPVAC 05160u
#define NEWMODEA 05357u
#define UPFLAG 05546u
#define GENTRAN 05606u
static const agc_word neg0[2]={CM_V37_K_NEG0};

static cm_fresh_io *ctx(const apollo_overlay_state *s,void *io){
 cm_fresh_io *x=io;
 if(s==0||x==0||x->control.interrupt_enabled>1u||(x->control.superbank&~0160u)||W[7]!=0)return 0;
 return x;
}
#define TS(a) do{if(ai_ts(s,(a)))return EXEC_OUT_OF_SCOPE;}while(0)
/* TC to an entry outside the file: Q = the word after the TC. */
#define OUT(ret,target) do{ai_tc(s,(ret));W[5]=(target);return EXEC_OK;}while(0)
/* The channels this file reads or writes. */
static agc_word *channel(cm_fresh_io *x,agc_word c){
 switch(c){
 case 05u:return &x->channel5; case 06u:return &x->channel6;
 case 011u:return &x->control.channel11; case 012u:return &x->control.channel12; case 013u:return &x->control.channel13;
 default:return 0;}
}
/* yaAGC CpuWriteIO: channel 13 bits 12-14 arm the HANDRUPT traps and do not
 * stay in the channel. */
static void write_channel(cm_fresh_io *x,agc_word c,agc_word v){
 if(c==013u){if(v&004000u)x->control.trap31a=1u;if(v&010000u)x->control.trap31b=1u;if(v&020000u)x->control.trap32=1u;v&=043777u;}
 *channel(x,c)=v;
}
/* EXTEND; WRITE/WAND/RAND k. */
static void chan_op(apollo_overlay_state *s,cm_fresh_io *x,agc_word c,char op){
 agc_word v=(agc_word)agc_correct(W[0]);
 if(op=='W')write_channel(x,c,v);
 else{v&=*channel(x,c);if(op=='A')write_channel(x,c,v);}
 W[0]=agc_extend(v);
}
/* INDEX k; CAF table: the index must name a word of the table. */
#define TABLE(tab,count,idx) do{agc_word i_=(agc_word)agc_correct(W[idx]);if(i_>=(count))return EXEC_OUT_OF_SCOPE;ai_caf(s,(tab)[i_]);}while(0)

/* Bank 04: GOP00FIX 2000 ... RAKE 2366. */
static enum exec_status bank04(const agc_native_program *p,apollo_overlay_state *s,void *io,agc_word at){
 cm_fresh_io *x=ctx(s,io);uint8_t c;agc_word i;(void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
jump:
 switch(at){
 case 02000u:goto L2000; case 02001u:goto L2001; case 02002u:case 02005u:case 02006u:case 02007u:goto L2002;
 case 02011u:goto L2011; case 02040u:goto L2040; case 02042u:goto L2042; case 02045u:goto L2045;
 case 02047u:goto L2047; case 02055u:goto L2055; case 02073u:goto L2073; case 02074u:goto L2074;
 case 02124u:goto L2124; case 02130u:goto L2130; case 02134u:goto L2134; case 02146u:goto L2146; case 02147u:goto L2147;
 case 02151u:goto L2151; case 02153u:goto L2153; case 02155u:goto L2155; case 02157u:goto L2157;
 case 02162u:goto L2162; case 02207u:goto L2207; case 02213u:goto L2213; case 02266u:goto L2266;
 case 02273u:goto L2273; case 02274u:goto L2274; case 02323u:goto L2323; case 02326u:goto L2326;
 case 02327u:goto L2327; case 02336u:goto L2336; case 02344u:goto L2347; case 02347u:goto L2347;
 case 02364u:goto L2364;
 default:return EXEC_OUT_OF_SCOPE;}
L2000: /* GOP00FIX */
 ai_tc(s,02001u);goto L2330;                                /* TC INITSUB */
L2001:
 OUT(02002u,CLEARMRK+2u);
L2002:
 ai_caf(s,CM_V37_K_V37N99);                                 /* 2002 */
 OUT(02004u,BANKCALL);                                      /* 2003; CADR GOFLASH; 2005-2007 TCF -3/-4/-5 */
L2011: /* V37 */
 TS(MMNUMBER); ai_caf(s,CM_V37_K_PRIO30); TS(RESTREG);      /* 2011-2013 */
 ai_ca(s,IMODES30); ai_maskf(s,CM_V37_K_BIT6);             /* 2014-2015 */
 c=ai_ccs(s,A);                                             /* 2016 */
 if(c==0u)goto L2071;                                       /* 2017 TCF CANTR00 */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a masked bit is never negative */
 ai_caf(s,CM_V37_K_BIT13); chan_op(s,x,011u,'a');           /* 2020-2022 EXTEND; RAND DSALMOUT */
 c=ai_ccs(s,A);                                             /* 2023 */
 if(c==0u)goto L2031;                                       /* 2024 TCF R00TOP00 */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a masked bit is never negative */
 ai_cs(s,FLAGWRD6); ai_maskf(s,CM_V37_K_OCT60000);          /* 2025-2026 */
 if(ai_bzmf(s))goto L2062;                                  /* 2027-2030 BZMF ISITP00 */
L2031: /* R00TOP00 */
 x->control.interrupt_enabled=0u;                           /* INHINT */
 ai_caf(s,CM_V37_K_EBANK6); TS(EBANK);                      /* 2032-2033 */
 ai_ca(s,CSMMASS); TS(MASSTMP);                             /* 2034-2035 */
 OUT(02037u,IBNKCALL);                                      /* 2036; CADR SPSOFF */
L2040:
 OUT(02041u,IBNKCALL);                                      /* CADR MASSPROP */
L2042:
 ai_caf(s,CM_V37_K_3_1SEC);
 OUT(02044u,IBNKCALL);                                      /* 2043; CADR RCSDAPON +1 */
L2045:
 OUT(02046u,IBNKCALL);                                      /* CADR TVCZAP */
L2047:
 ai_caf(s,CM_V37_K_ZERO); TS(MMNUMBER);                     /* 2047-2050 */
 x->control.interrupt_enabled=1u;                           /* 2051 RELINT */
 ai_caf(s,CM_V37_K_FIVE);
 OUT(02054u,BANKCALL);                                      /* 2053; CADR DELAYJOB */
L2055:
 ai_caf(s,CM_V37_K_ZERO); chan_op(s,x,05u,'W'); chan_op(s,x,06u,'W'); /* 2055-2061 */
L2062: /* ISITP00 */
 ai_ca(s,MMNUMBER);
 if(ai_bzf(s))goto L2113;                                   /* 2063-2064 BZF ISSERVON */
 ai_cs(s,FLAGWRD2); ai_maskf(s,CM_V37_K_NODOBIT);           /* 2065-2066 */
 c=ai_ccs(s,A);                                             /* 2067 */
 if(c==0u)goto L2076;                                       /* 2070 TCF CHECKTAB */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a masked bit is never negative */
L2071: /* CANTR00 */
 OUT(02072u,ALARM);                                         /* OCT 1520 */
L2073: /* V37BAD */
 OUT(02074u,RELDSP);
L2074:
 OUT(02075u,POSTJUMP);                                      /* CADR PINBRNCH */
L2076: /* CHECKTAB: find MMNUMBER in PREMM1 */
 ai_caf(s,CM_V37_K_NOV37MM);
L2077: /* AGAINMM */
 TS(MPAC+1u);
 TABLE(cm_v37_premm1,041u,MPAC+1u);                         /* 2100-2101 NDX MPAC +1; CA PREMM1 */
 ai_maskf(s,CM_V37_K_LOW7); ai_cs(s,A); ai_ad(s,MMNUMBER);  /* 2102-2104 */
 c=ai_ccs(s,A);                                             /* 2105 */
 if(c==0u){                                                 /* 2106 CCS MPAC +1 */
  c=ai_ccs(s,MPAC+1u);
  if(c==0u)goto L2077;                                      /* 2107 TCF AGAINMM */
  if(c==1u)goto L2272;                                      /* 2110 TCF V37NONO */
  if(c==2u)goto L2111;
  goto L2112;
 }
 if(c==1u)goto L2077;                                       /* 2107 TCF AGAINMM */
 if(c==2u)goto L2272;                                       /* 2110 TCF V37NONO */
L2111:
 ai_ca(s,MPAC+1u);
L2112:
 TS(MINDEX);
L2113: /* ISSERVON */
 ai_cs(s,FLAGWRD7); ai_maskf(s,CM_V37_K_V37FLBIT);          /* 2113-2114 */
 c=ai_ccs(s,A);                                             /* 2115 */
 if(c==0u)goto L2124;                                       /* 2116 TCF CANV37 */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a masked bit is never negative */
 x->control.interrupt_enabled=0u;                           /* 2117 INHINT */
 ai_csf(s,CM_V37_K_AVEGBIT); ai_mask(s,FLAGWRD1); TS(FLAGWRD1);/* 2120-2122 */
 W[5]=ENDOFJOB;return EXEC_OK;                              /* 2123 TCF ENDOFJOB */
L2124: /* CANV37 */
 ai_caf(s,CM_V37_K_R00AD); TS(TEMPFLSH);                    /* 2124-2125 */
 OUT(02127u,PHASCHNG);                                      /* 2126; OCT 14 */
L2130: /* ROC */
 OUT(02131u,INTPRET);                                       /* CALL INTSTALL; DUMMYAD EXIT */
L2134:
 ai_csf(s,CM_V37_K_OCT1400); chan_op(s,x,011u,'A');          /* 2134-2136 EXTEND; WAND 11 */
 ai_caf(s,CM_V37_K_OCT44571); chan_op(s,x,012u,'A');        /* 2137-2141 EXTEND; WAND 12 */
 ai_csf(s,CM_V37_K_OCT600); chan_op(s,x,013u,'A');          /* 2142-2144 EXTEND; WAND 13 */
 ai_tc(s,02146u);goto L2330;                                /* 2145 TC INITSUB */
L2146:
 OUT(02147u,CLEARMRK);
L2147:
 OUT(02150u,DOWNFLAG);                                      /* ADRES STIKFLAG */
L2151:
 OUT(02152u,BANKCALL);                                      /* CADR UPACTOFF */
L2153:
 OUT(02154u,DOWNFLAG);                                      /* ADRES VHFRFLAG */
L2155:
 OUT(02156u,DOWNFLAG);                                      /* ADRES R21MARK */
L2157:
 c=ai_ccs(s,MMNUMBER);
 if(c==0u)goto L2260;                                       /* 2160 TCF NOUVEAU */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a major mode is never negative */
 OUT(02162u,RELDSP);                                        /* 2161 POOH */
L2162:
 ai_caf(s,CM_V37_K_PRIO5); TS(PHSPRDT2);                    /* 2162-2163 */
 x->control.interrupt_enabled=0u;                           /* 2164 INHINT */
 ai_csf(s,CM_V37_K_NODOBIT); ai_mask(s,FLAGWRD2); TS(FLAGWRD2);/* 2165-2167 */
 ai_caf(s,CM_V37_K_FIVE); TS(L); ai_cs(s,A); ai_dxch(s,MPHASE2);/* 2170-2173 */
 ai_csf(s,CM_V37_K_BIT7_8); ai_mask(s,FLAGWRD0); TS(FLAGWRD0);/* 2174-2176 */
 ai_caf(s,CM_V37_K_DNLADP00);                               /* 2177 */
L2200: /* SEUDOP00 */
 TS(DNLSTCOD); ai_csf(s,CM_V37_K_OCT01120); TS(EBANKTEM);   /* 2200-2202 */
 ai_mask(s,FLAGWRD1); TS(FLAGWRD1);                         /* 2203-2204 */
 OUT(02206u,IBNKCALL);                                      /* 2205 GROUPKIL; CADR V37KLEAN */
L2207:
 c=ai_ccs(s,MMNUMBER);
 if(c==0u)goto L2217;                                       /* 2210 TCF RENDV00 */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a major mode is never negative */
 OUT(02212u,IBNKCALL);                                      /* 2211; CADR P00KLEAN */
L2213: /* GOMOD */
 ai_ca(s,MMNUMBER); TS(MODREG);                             /* 2213-2214 */
L2215: /* GOGOPROG */
 OUT(02216u,POSTJUMP);                                      /* CADR GOPROG2 */
L2217: /* RENDV00 */
 ai_cs(s,MMNUMBER); ai_adf(s,CM_V37_K_OCT24);               /* 2217-2220 */
 if(!ai_bzf(s))goto L2234;                                  /* 2221-2222 BZF RENDN00; 2223 TCF P00FIZZ */
 ai_cs(s,MMNUMBER); ai_ad(s,MODREG);                        /* 2224-2225 RENDN00 */
 if(ai_bzf(s))goto L2240;                                   /* 2226-2227 BZF KILL20 */
 ai_ca(s,FLAGWRD0); ai_maskf(s,CM_V37_K_RNDVZBIT);          /* 2230-2231 */
 c=ai_ccs(s,A);                                             /* 2232 */
 if(c==0u)goto L2251;                                       /* 2233 TCF STATQUO */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a masked bit is never negative */
L2234: /* P00FIZZ */
 ai_caf(s,CM_V37_K_RNDVZBIT); ai_mask(s,FLAGWRD0);          /* 2234-2235 */
 c=ai_ccs(s,A);                                             /* 2236 */
 if(c==0u)goto L2246;                                       /* 2237 TCF REV37 */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a masked bit is never negative */
L2240: /* KILL20 */
 ai_dcaf(s,neg0); ai_dxch(s,MPHASE1);                       /* 2240-2242 */
 ai_dcaf(s,neg0); ai_dxch(s,MPHASE2);                       /* 2243-2245 */
L2246: /* REV37 */
 ai_caf(s,CM_V37_K_V37QCAD); TS(TEMPFLSH);                  /* 2246-2247 */
 goto L2215;                                                /* 2250 TCF GOGOPROG */
L2251: /* STATQUO */
 ai_cs(s,FLAGWRD1); ai_maskf(s,CM_V37_K_OCT120); ai_ads(s,FLAGWRD1);/* 2251-2253 */
 ai_dcaf(s,neg0); ai_dxch(s,MPHASE4);                       /* 2254-2256 */
 goto L2213;                                                /* 2257 TCF GOMOD */
L2260: /* NOUVEAU */
 ai_caf(s,CM_V37_K_RNDVZBIT); ai_mask(s,FLAGWRD0);          /* 2260-2261 */
 c=ai_ccs(s,A);                                             /* 2262 */
 if(c==1u)OUT(02265u,DOWNFLAG);                             /* 2264; ADRES IMUSE */
 if(c!=0u)return EXEC_OUT_OF_SCOPE;                         /* 2263 TCF +3; a masked bit is never negative */
L2266:
 TABLE(cm_v37_dnladmm1,041u,MINDEX);                        /* 2266-2267 INDEX MINDEX; CAF DNLADMM1 */
 x->control.interrupt_enabled=0u;                           /* 2270 INHINT */
 goto L2200;                                                /* 2271 TCF SEUDOP00 */
L2272: /* V37NONO */
 OUT(02273u,FALTON);
L2273:
 goto L2073;                                                /* TCF V37BAD */
L2274: /* V37XEQ */
 x->control.interrupt_enabled=0u;                           /* INHINT */
 TABLE(cm_v37_premm1,041u,MINDEX);                          /* 2275-2276 INDEX MINDEX; CAF PREMM1 */
 TS(MMTEMP); TS(CYR); ai_ca(s,CYR); ai_maskf(s,CM_V37_K_PRIO37);/* 2277-2302 */
 TS(PHSPRDT4); TS(NEWPRIO); ai_ca(s,MMTEMP);                /* 2303-2305 */
 ai_mpf(s,CM_V37_K_BIT8);                                   /* 2306-2307 EXTEND; MP BIT8 */
 ai_maskf(s,CM_V37_K_LOW3); TS(L);                          /* 2310-2311 */
 TABLE(cm_v37_fcadrmm1,041u,MINDEX);                        /* 2312-2313 INDEX MINDEX; CAF FCADRMM1 */
 TS(BASETEMP); ai_maskf(s,CM_V37_K_HI5); ai_ads(s,L);       /* 2314-2316 */
 ai_ca(s,BASETEMP); ai_maskf(s,CM_V37_K_LOW10); ai_adf(s,CM_V37_K_BIT11);/* 2317-2321 */
 OUT(02323u,SPVAC);                                         /* 2322 */
L2323: /* V37XEQC */
 ai_ca(s,MMTEMP); ai_maskf(s,CM_V37_K_LOW7);                /* 2323-2324 */
 OUT(02326u,NEWMODEA);                                      /* 2325 */
L2326:
 OUT(02327u,RELDSP);
L2327:
 OUT(02330u,ENDOFJOB);
L2330: /* INITSUB */
 ai_qxch(s,MPAC+1u);                                        /* 2330-2331 EXTEND; QXCH MPAC +1 */
 ai_caf(s,CM_V37_K_EBANK6); TS(EBANK);                      /* 2332-2333 */
 x->control.interrupt_enabled=0u;                           /* 2334 INHINT */
 OUT(02336u,STOPRATE);                                      /* 2335 */
L2336:
 ai_ca(s,FLAGWRD9); ai_maskf(s,CM_V37_K_MAXDBBIT);          /* 2336-2337 */
 c=ai_ccs(s,A);                                             /* 2340 */
 if(c==0u)OUT(02346u,BANKCALL);                             /* 2341 TCF SETMAXER; 2345 TC BANKCALL; CADR SETMAXDB */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a masked bit is never negative */
 OUT(02343u,BANKCALL);                                      /* 2342; CADR SETMINDB; 2344 TCF RAKE */
L2347: /* RAKE */
 ai_caf(s,CM_V37_K_ELEVEN);
 for(;;){                                                   /* RAKE +1 */
  TS(MPAC); i=W[MPAC];                                      /* 2350 */
  ai_csf(s,cm_v37_flagtabl[i]);                             /* 2351-2352 INDEX MPAC; CS FLAGTABL */
  ai_mask(s,(agc_word)(FLAGWRD0+i));                        /* 2353-2354 INDEX MPAC; MASK FLAGWRD0 */
  TS(FLAGWRD0+i);                                           /* 2355-2356 INDEX MPAC; TS FLAGWRD0 */
  c=ai_ccs(s,MPAC);                                         /* 2357 */
  if(c==1u)break;                                           /* 2360 TCF RAKE +1 */
  if(c!=0u)return EXEC_OUT_OF_SCOPE;                        /* MPAC counts down from 11 */
 }
 x->control.interrupt_enabled=1u;                           /* 2361 RELINT */
 OUT(02363u,UPFLAG);                                        /* 2362; ADRES IMPULSW */
L2364:
 ai_caf(s,CM_V37_K_NEGONE); TS(OPTIND);                     /* 2364-2365 */
 /* 2366 TC MPAC +1: the word INITSUB's QXCH stored executes as TC. */
 ai_tc(s,02367u);
 at=ai_word(s,MPAC+1u);
 if(at!=02001u&&at!=02146u)return EXEC_OUT_OF_SCOPE;
 ai_tc(s,(agc_word)(MPAC+2u));goto jump;
}

/* Bank 20: VAC5STOR 2000 ... TC SWRETURN 2063. */
static enum exec_status bank20(const agc_native_program *p,apollo_overlay_state *s,void *io,agc_word at){
 cm_fresh_io *x=ctx(s,io);agc_word i,j;(void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
 if(at==02033u)goto L2033;
 if(at!=02000u)return EXEC_OUT_OF_SCOPE;
 ai_caf(s,CM_V37_K_ZERO); TS(ITEMP1); TS(ITEMP2);          /* 2000-2002 VAC5STOR */
 for(;;){                                                   /* V5LOOP1: the indices are VAC5STOR's own (0, 12, ... / 0, 3, ...) */
  i=W[ITEMP1];j=W[ITEMP2];
  ai_dca(s,(agc_word)(LOC+i)); ai_dxch(s,(agc_word)(VAC5+j));/* 2003-2007 EXTEND; INDEX ITEMP1; DCA LOC; INDEX ITEMP2; DXCH VAC5 */
  ai_ca(s,(agc_word)(PRIORITY+i)); TS(VAC5+2u+j);           /* 2010-2013 */
  ai_cs(s,ITEMP2); ai_adf(s,CM_V37_K_EIGHTEEN);             /* 2014-2015 */
  if(ai_bzf(s))break;                                       /* 2016-2017 BZF V5OUT1 */
  ai_caf(s,CM_V37_K_TWELVE); ai_ads(s,ITEMP1);              /* 2020-2021 */
  ai_caf(s,CM_V37_K_THREE); ai_ads(s,ITEMP2);               /* 2022-2023; 2024 TCF V5LOOP1 */
 }
 ai_caf(s,CM_V37_K_EBANK3); TS(EBANK);                      /* 2025-2026 V5OUT1 */
 ai_caf(s,CM_V37_K_ELEVEN);
 OUT(02031u,GENTRAN);                                       /* 2030; ADRES PHSNAME1; ADRES VAC5 +21D */
L2033:
 ai_caf(s,CM_V37_K_ZERO); TS(ITEMP1); TS(ITEMP2);          /* 2033-2035 */
 for(;;){                                                   /* V5LOOP2 */
  i=W[ITEMP1];j=W[ITEMP2];
  ai_ca(s,(agc_word)(PHASE1+i)); TS(VAC5+33u+j);            /* 2036-2041 INDEX ITEMP1; CA PHASE1; INDEX ITEMP2; TS VAC5 +33D */
  ai_cs(s,ITEMP2); ai_adf(s,CM_V37_K_FIVE);                 /* 2042-2043 */
  if(ai_bzf(s))break;                                       /* 2044-2045 BZF V5OUT2 */
  ai_caf(s,CM_V37_K_TWO); ai_ads(s,ITEMP1); ai_incr(s,ITEMP2);/* 2046-2050; 2051 TCF V5LOOP2 */
 }
 ai_ca(s,MPAC+3u); TS(VAC5+39u);                            /* 2052-2053 V5OUT2 */
 ai_dca(s,NEWLOC); ai_dxch(s,VAC5+40u);                     /* 2054-2056 */
 ai_ca(s,NEWJOB); TS(VAC5+22u); ai_ca(s,NEWPRIO); TS(VAC5+26u);/* 2057-2062 */
 OUT(02064u,SWRETURN);                                      /* 2063 */
}

const cm_v37_entry cm_v37_entries[CM_V37_ROUTINES]={
 {04,02000},{04,02002},{04,02005},{04,02006},{04,02007},{04,02011},{04,02040},{04,02042},{04,02045},{04,02047},
 {04,02055},{04,02073},{04,02074},{04,02124},{04,02130},{04,02134},{04,02147},{04,02151},{04,02153},{04,02155},{04,02157},
 {04,02162},{04,02207},{04,02213},{04,02266},{04,02273},{04,02274},{04,02323},{04,02326},{04,02327},{04,02336},
 {04,02344},{04,02347},{04,02364},{020,02000},{020,02033}};
#define E04(z) static enum exec_status e##z(const agc_native_program *p,apollo_overlay_state *s,void *io){return bank04(p,s,io,0##z##u);}
#define E20(z) static enum exec_status f##z(const agc_native_program *p,apollo_overlay_state *s,void *io){return bank20(p,s,io,0##z##u);}
E04(2000) E04(2002) E04(2005) E04(2006) E04(2007) E04(2011) E04(2040) E04(2042) E04(2045) E04(2047)
E04(2055) E04(2073) E04(2074) E04(2124) E04(2130) E04(2134) E04(2147) E04(2151) E04(2153) E04(2155) E04(2157)
E04(2162) E04(2207) E04(2213) E04(2266) E04(2273) E04(2274) E04(2323) E04(2326) E04(2327) E04(2336)
E04(2344) E04(2347) E04(2364) E20(2000) E20(2033)
static const agc_routine_entry entries[CM_V37_ROUTINES]={
 {B04(02000),e2000},{B04(02002),e2002},{B04(02005),e2005},{B04(02006),e2006},{B04(02007),e2007},{B04(02011),e2011},
 {B04(02040),e2040},{B04(02042),e2042},{B04(02045),e2045},{B04(02047),e2047},{B04(02055),e2055},{B04(02073),e2073},
 {B04(02074),e2074},{B04(02124),e2124},{B04(02130),e2130},{B04(02134),e2134},{B04(02147),e2147},{B04(02151),e2151},{B04(02153),e2153},
 {B04(02155),e2155},{B04(02157),e2157},{B04(02162),e2162},{B04(02207),e2207},{B04(02213),e2213},{B04(02266),e2266},
 {B04(02273),e2273},{B04(02274),e2274},{B04(02323),e2323},{B04(02326),e2326},{B04(02327),e2327},{B04(02336),e2336},
 {B04(02344),e2344},{B04(02347),e2347},{B04(02364),e2364},{B20(02000),f2000},{B20(02033),f2033}};
const agc_routine_table cm_v37_routines={entries,CM_V37_ROUTINES};
