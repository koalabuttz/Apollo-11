/* SPDX-License-Identifier: MIT */
/* Luminary099 FRESH_START_AND_RESTART, bank 04: GOP00FIX, V37 and its
 * major-mode table search, the servicer wait (ISSERVON/V37RET), CANV37, R00,
 * POOH, SEUDOP00, GOMOD/GOGOPROG, the rendezvous checks (RENDV00, RESET22,
 * KILL2, P00FIZZ, RENDN00, STATQUO), NOUVEAU, V37NONO, V37XEQ/V37XEQC and
 * SETUP70/71. One agc_image_ops call per original instruction, under labels
 * L<address>. Every TC here leaves the bank (OUT); the routine resumes at
 * the return point as its own entry. The PREMM1, FCADRMM1 and DNLADMM1
 * reads take only indices inside their 30-word tables; MMNUMBER is never
 * negative (pinball's verb 37 mode). */
#include "lm_v37.h"
#include "agc_image_ops.h"
#include "lm_v37_operands.inc"

#include "agc_image_w.h"
#define B04(z) (uint16_t)(04u*1024u+((z)-02000u))
#define A 0u
#define L 1u
#define Q 2u
#define MMNUMBER LM_V37_MMNUMBER
/* Entries outside the bank. */
#define DOWNFLAG 05516u
#define CLEARMRK 05474u
#define BANKCALL 04616u
#define POSTJUMP 04635u
#define IBNKCALL 04674u
#define ALARM 05567u
#define RELDSP 04457u
#define ENDOFJOB 05155u
#define PHSCHNGA 05357u
#define PHASCHNG 05353u
#define INTPRET 06037u
#define CLRADMOD 06011u
#define FALTON 04364u
#define SPVAC 05116u
#define NEWMODEA 05314u
static const agc_word neg0[2]={LM_V37_K_NEG0};
static const agc_word p70cadr[2]={LM_V37_K_P70CADR};

static p66_bias_io *ctx(const apollo_overlay_state *s,void *io){
 p66_bias_io *x=io;
 if(s==0||x==0||x->interrupt_enabled>1u||(x->superbank&~0160u)||W[7]!=0)return 0;
 return x;
}
#define TS(a) do{if(ai_ts(s,(a)))return EXEC_OUT_OF_SCOPE;}while(0)
/* TC to an entry outside the bank: Q = the word after the TC. */
#define OUT(ret,target) do{ai_tc(s,(ret));W[5]=(target);return EXEC_OK;}while(0)

/* Bank 04: GOP00FIX 2024 ... V37XEQC 2363. */
static enum exec_status bank04(const agc_native_program *p,apollo_overlay_state *s,void *io,agc_word at){
 p66_bias_io *x=ctx(s,io);uint8_t c;agc_word ins;(void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
 switch(at){
 case 02024u:goto L2024; case 02026u:goto L2026; case 02030u:goto L2030; case 02031u:goto L2031;
 case 02034u:case 02035u:case 02036u:goto L2031; case 02040u:goto L2040; case 02067u:goto L2067;
 case 02070u:goto L2070; case 02115u:goto L2115; case 02120u:goto L2120; case 02134u:goto L2134;
 case 02143u:goto L2143; case 02147u:goto L2147; case 02151u:goto L2151; case 02155u:goto L2155;
 case 02157u:goto L2157; case 02161u:goto L2161; case 02164u:goto L2164; case 02167u:goto L2167;
 case 02206u:goto L2206; case 02210u:goto L2210; case 02216u:goto L2216; case 02222u:goto L2222;
 case 02254u:goto L2254; case 02316u:goto L2316; case 02323u:goto L2323; case 02330u:goto L2330;
 case 02357u:goto L2357; case 02362u:goto L2362; case 02363u:goto L2363;
 default:return EXEC_OUT_OF_SCOPE;}
L2024: /* GOP00FIX */
 OUT(02025u,DOWNFLAG);                                      /* ADRES XOVINFLG */
L2026:
 OUT(02027u,DOWNFLAG);                                      /* ADRES ULLAGFLG */
L2030:
 OUT(02031u,CLEARMRK+2u);
L2031:
 ai_caf(s,LM_V37_K_V37N99);                                 /* 2031 */
 OUT(02033u,BANKCALL);                                      /* 2032; CADR GOFLASH; 2034-2036 TCF -3/-4/-5 */
L2040: /* V37 */
 TS(0775u); ai_caf(s,LM_V37_K_PRIO30); TS(0366u); ai_ca(s,01302u);/* 2040-2043 */
 ai_maskf(s,LM_V37_K_BIT6);                                /* 2044 */
 c=ai_ccs(s,A);                                             /* 2045 */
 if(c==0u)goto L2065;                                       /* 2046 TCF CANTROD */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a masked bit is never negative */
 ai_cs(s,0775u); ai_adf(s,LM_V37_K_DEC70);                 /* 2047-2050 */
 if(ai_bzf(s))goto L2373;                                   /* 2051-2052 BZF SETUP70 */
 ai_adf(s,LM_V37_K_ONE);                                   /* 2053 */
 if(ai_bzf(s))goto L2372;                                   /* 2054-2055 BZF SETUP71 */
 ai_ca(s,0775u);                                           /* 2056 */
 if(ai_bzf(s))goto L2107;                                   /* 2057-2060 BZF ISSERVON */
 ai_cs(s,076u); ai_maskf(s,LM_V37_K_NODOBIT);              /* 2061-2062 */
 c=ai_ccs(s,A);                                             /* 2063 */
 if(c==0u)goto L2072;                                       /* 2064 TCF CHECKTAB */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a masked bit is never negative */
L2065: /* CANTROD */
 OUT(02066u,ALARM);                                         /* OCT 1520 */
L2067: /* V37BAD */
 OUT(02070u,RELDSP);
L2070:
 OUT(02071u,POSTJUMP);                                      /* CADR PINBRNCH */
L2072: /* CHECKTAB: find MMNUMBER in PREMM1 */
 ai_caf(s,LM_V37_K_NOV37MM);
L2073: /* AGAINMM */
 TS(0155u);
 if(W[0]>035u)return EXEC_OUT_OF_SCOPE;                     /* 2074-2075 NDX MPAC +1; CA PREMM1: the table's 30 words */
 ai_caf(s,lm_v37_premm1[W[0]]);
 ai_maskf(s,LM_V37_K_LOW7); ai_cs(s,A); ai_ad(s,0775u);    /* 2076-2100 */
 c=ai_ccs(s,A);                                             /* 2101 */
 if(c==0u){                                                 /* 2102 CCS MPAC +1 */
  c=ai_ccs(s,0155u);
  if(c==0u)goto L2073;                                      /* 2103 TCF AGAINMM */
  if(c==1u)goto L2322;                                      /* 2104 TCF V37NONO */
  if(c==2u)goto L2105;
  goto L2106;
 }
 if(c==1u)goto L2073;                                       /* 2103 TCF AGAINMM */
 if(c==2u)goto L2322;                                       /* 2104 TCF V37NONO */
L2105:
 ai_ca(s,0155u);
L2106:
 TS(0774u);                                                 /* MINDEX */
L2107: /* ISSERVON */
 ai_cs(s,0103u); ai_maskf(s,LM_V37_K_V37FLBIT);            /* 2107-2110 */
 c=ai_ccs(s,A);                                             /* 2111 */
 if(c==0u)goto L2134;                                       /* 2112 TCF CANV37 */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a masked bit is never negative */
 OUT(02114u,DOWNFLAG);                                      /* 2113; ADRES AVEGFLAG */
L2115:
 ai_caf(s,LM_V37_K_V37RETAD); TS(01260u);                  /* 2115-2116 */
 W[5]=ENDOFJOB;return EXEC_OK;                              /* 2117 TCF ENDOFJOB */
L2120: /* V37RET */
 ai_cs(s,074u); ai_maskf(s,LM_V37_K_RNDVZBIT);             /* 2120-2121 */
 c=ai_ccs(s,A);                                             /* 2122 */
 if(c==1u)goto L2132;                                       /* 2124 TCF 2.7SPT */
 if(c!=0u)return EXEC_OUT_OF_SCOPE;                         /* a masked bit is never negative */
 ai_cs(s,074u); ai_maskf(s,LM_V37_K_P25FLBIT);             /* 2125-2126 */
 c=ai_ccs(s,A);                                             /* 2127 */
 if(c==0u)ai_caf(s,LM_V37_K_OCT37667);                      /* 2130 2.0SPT */
 else if(c!=1u)return EXEC_OUT_OF_SCOPE;                    /* a masked bit is never negative */
 ai_adf(s,LM_V37_K_BIT5);                                   /* 2131 2.11SPT */
L2132: /* 2.7SPT */
 ai_adf(s,LM_V37_K_OCT40072);
 OUT(02134u,PHSCHNGA);                                      /* 2133 */
L2134: /* CANV37 */
 ai_caf(s,LM_V37_K_ZERO);x->superbank=0u;                   /* 2134-2136 EXTEND; WRITE SUPERBNK */
 ai_caf(s,LM_V37_K_R00AD); TS(0374u);                      /* 2137-2140 */
 OUT(02142u,PHASCHNG);                                      /* 2141; OCT 14 */
L2143: /* R00 */
 OUT(02144u,INTPRET);                                       /* CALL INTSTALL; DUMMYAD EXIT */
L2147:
 OUT(02150u,DOWNFLAG);                                      /* ADRES 3AXISFLG */
L2151:
 ai_caf(s,LM_V37_K_LRBYBIT); TS(0107u);                    /* 2151-2152 */
 OUT(02154u,DOWNFLAG);                                      /* 2153; ADRES R04FLAG */
L2155:
 OUT(02156u,DOWNFLAG);                                      /* ADRES MUNFLAG */
L2157:
 OUT(02160u,DOWNFLAG);                                      /* ADRES XOVINFLG */
L2161:
 c=ai_ccs(s,MMNUMBER);
 if(c==0u)goto L2310;                                       /* 2162 TCF NOUVEAU */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a major mode is never negative */
 OUT(02164u,RELDSP);                                        /* 2163 POOH */
L2164:
 ai_caf(s,LM_V37_K_PRIO5); TS(01056u);                     /* 2164-2165 */
 OUT(02167u,CLRADMOD);                                      /* 2166 */
L2167:
 ai_csf(s,LM_V37_K_NODOBIT); ai_mask(s,076u); TS(076u); ai_caf(s,LM_V37_K_FIVE);/* 2167-2172 */
 TS(01u); ai_cs(s,A); ai_dxch(s,0754u); ai_csf(s,LM_V37_K_OCT700);/* 2173-2176 */
 ai_mask(s,074u); TS(074u); ai_caf(s,LM_V37_K_DNLADP00);   /* 2177-2201 */
L2202: /* SEUDOP00 */
 TS(0332u); TS(01324u);                                    /* 2202-2203 */
 OUT(02205u,IBNKCALL);                                      /* 2204; CADR ENGINOF1 */
L2206:
 OUT(02207u,IBNKCALL);                                      /* CADR ALLCOAST */
L2210:
 ai_csf(s,LM_V37_K_OCT120); TS(01072u); ai_mask(s,075u); TS(075u);/* 2210-2213 */
 OUT(02215u,IBNKCALL);                                      /* 2214; CADR V37KLEAN */
L2216:
 c=ai_ccs(s,MMNUMBER);
 if(c==0u)goto L2226;                                       /* 2217 TCF RENDV00 */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a major mode is never negative */
L2220: /* GOMOD */
 OUT(02221u,IBNKCALL);                                      /* CADR P00KLEAN */
L2222:
 ai_ca(s,0775u); TS(01011u);                               /* 2222-2223 */
L2224: /* GOGOPROG */
 OUT(02225u,POSTJUMP);                                      /* CADR GOPROG2 */
L2226: /* RENDV00 */
 ai_cs(s,01011u); ai_adf(s,LM_V37_K_OCT26);                /* 2226-2227 */
 if(ai_bzf(s))goto L2250;                                   /* 2230-2231 BZF RESET22 */
 ai_cs(s,0775u); ai_adf(s,LM_V37_K_OCT26);                 /* 2232-2233 */
 if(ai_bzf(s))goto L2250;                                   /* 2234-2235 BZF RESET22 */
 ai_adf(s,LM_V37_K_NEG2);                                  /* 2236 */
 if(ai_bzf(s))goto L2262;                                   /* 2237-2240 BZF RENDN00 */
 ai_adf(s,LM_V37_K_FIVE);                                  /* 2241 */
 if(ai_bzf(s))goto L2262;                                   /* 2242-2243 BZF RENDN00 */
 ai_caf(s,LM_V37_K_OCT500); ai_mask(s,074u);               /* 2244-2245 */
 c=ai_ccs(s,A);                                             /* 2246 */
 if(c==0u)goto L2257;                                       /* 2247 TCF P00FIZZ */
 if(c!=1u)return EXEC_OUT_OF_SCOPE;                         /* a masked bit is never negative */
L2250: /* RESET22 */
 ai_csf(s,LM_V37_K_OCT700); ai_mask(s,074u); TS(074u);     /* 2250-2252 */
 OUT(02254u,CLRADMOD);                                      /* 2253 */
L2254: /* KILL2 */
 ai_dcaf(s,neg0);ai_dxch(s,0754u);                          /* 2254-2256 -PHASE2 */
L2257: /* P00FIZZ */
 ai_caf(s,LM_V37_K_V37QCAD); TS(0374u);                    /* 2257-2260 */
 goto L2224;                                                /* 2261 TCF GOGOPROG */
L2262: /* RENDN00 */
 ai_cs(s,01011u); ai_adf(s,LM_V37_K_OCT24);                /* 2262-2263 */
 if(ai_bzf(s))goto L2254;                                   /* 2264-2265 BZF KILL2 */
 ai_adf(s,LM_V37_K_FIVE);                                  /* 2266 */
 if(ai_bzf(s))goto L2254;                                   /* 2267-2270 BZF KILL2 */
 ai_caf(s,LM_V37_K_OCT500); ai_mask(s,074u); ai_ad(s,0775u);/* 2271-2273 */
 ai_cs(s,A); ai_adf(s,LM_V37_K_P20REG);                    /* 2274-2275 */
 if(ai_bzf(s))goto L2304;                                   /* 2276-2277 BZF STATQUO */
 ai_adf(s,LM_V37_K_OCT305);                                /* 2300 */
 if(ai_bzf(s))goto L2304;                                   /* 2301-2302 BZF STATQUO */
 goto L2254;                                                /* 2303 TCF KILL2 */
L2304: /* STATQUO */
 ai_cs(s,075u); ai_maskf(s,LM_V37_K_OCT120);               /* 2304-2305 */
 ai_ads(s,075u);                                            /* 2306 ADS FLAGWRD1 */
 goto L2220;                                                /* 2307 TCF GOMOD */
L2310: /* NOUVEAU */
 ai_caf(s,LM_V37_K_OCT500); ai_mask(s,074u);               /* 2310-2311 */
 c=ai_ccs(s,A);                                             /* 2312 */
 if(c==1u)OUT(02315u,DOWNFLAG);                             /* 2314; ADRES IMUSE */
 else if(c!=0u)return EXEC_OUT_OF_SCOPE;                    /* 2313 TCF +3; a masked bit is never negative */
L2316:
 if((ins=ai_word(s,0774u))>035u)return EXEC_OUT_OF_SCOPE;   /* 2316-2317 INDEX MINDEX; CAF DNLADMM1 */
 ai_caf(s,lm_v37_dnladmm1[ins]);
 x->interrupt_enabled=0u;                                   /* 2320 INHINT */
 goto L2202;                                                /* 2321 TCF SEUDOP00 */
L2322: /* V37NONO */
 OUT(02323u,FALTON);
L2323:
 goto L2067;                                                /* TCF V37BAD */
L2330: /* V37XEQ */
 x->interrupt_enabled=0u;                                   /* INHINT */
 if((ins=ai_word(s,0774u))>035u)return EXEC_OUT_OF_SCOPE;   /* 2331-2332 INDEX MINDEX; CAF PREMM1 */
 ai_caf(s,lm_v37_premm1[ins]);
 TS(01060u); TS(020u); ai_ca(s,020u); ai_maskf(s,LM_V37_K_PRIO37);/* 2333-2336 */
 TS(01062u); TS(063u); ai_ca(s,01060u);                    /* 2337-2341 */
 ai_mpf(s,LM_V37_K_BIT8);                                   /* 2342-2343 EXTEND; MP BIT8 */
 ai_maskf(s,LM_V37_K_LOW3); TS(01u);                       /* 2344-2345 */
 if((ins=ai_word(s,0774u))>035u)return EXEC_OUT_OF_SCOPE;   /* 2346-2347 INDEX MINDEX; CAF FCADRMM1 */
 ai_caf(s,lm_v37_fcadrmm1[ins]);
 TS(01061u); ai_maskf(s,LM_V37_K_HI5);                     /* 2350-2351 */
 ai_ads(s,L);                                               /* 2352 ADS L */
 ai_ca(s,01061u); ai_maskf(s,LM_V37_K_LOW10); ai_adf(s,LM_V37_K_BIT11);/* 2353-2355 */
 OUT(02357u,SPVAC);                                         /* 2356 */
L2357: /* V37XEQC */
 ai_ca(s,01060u); ai_maskf(s,LM_V37_K_LOW7);               /* 2357-2360 */
 OUT(02362u,NEWMODEA);                                      /* 2361 */
L2362:
 OUT(02363u,RELDSP);
L2363:
 OUT(02364u,ENDOFJOB);
L2372: /* SETUP71 */
 ai_caf(s,LM_V37_K_THREE);
L2373: /* SETUP70 */
 TS(Q);ai_dcaf(s,p70cadr);ai_ad(s,Q);                       /* 2373-2376 */
 W[5]=02400u;ai_dxch(s,5u);return EXEC_OK;                  /* 2377 DTCB: Z and BB from the 2CADR P70 (+3 for P71) */
}

const uint16_t lm_v37_entries[LM_V37_ROUTINES]={02024u,02026u,02030u,02031u,02034u,02035u,02036u,02040u,02067u,02070u,02115u,02120u,02134u,02143u,02147u,02151u,02155u,02157u,02161u,02164u,02167u,02206u,02210u,02216u,02222u,02254u,02316u,02323u,02330u,02357u,02362u,02363u};
#define E04(z) static enum exec_status e##z(const agc_native_program *p,apollo_overlay_state *s,void *io){return bank04(p,s,io,0##z##u);}
E04(2024) E04(2026) E04(2030) E04(2031) E04(2034) E04(2035) E04(2036) E04(2040)
E04(2067) E04(2070) E04(2115) E04(2120) E04(2134) E04(2143) E04(2147) E04(2151)
E04(2155) E04(2157) E04(2161) E04(2164) E04(2167) E04(2206) E04(2210) E04(2216)
E04(2222) E04(2254) E04(2316) E04(2323) E04(2330) E04(2357) E04(2362) E04(2363)
static const agc_routine_entry entries[LM_V37_ROUTINES]={
 {B04(02024),e2024},{B04(02026),e2026},{B04(02030),e2030},{B04(02031),e2031},{B04(02034),e2034},{B04(02035),e2035},
 {B04(02036),e2036},{B04(02040),e2040},{B04(02067),e2067},{B04(02070),e2070},{B04(02115),e2115},{B04(02120),e2120},
 {B04(02134),e2134},{B04(02143),e2143},{B04(02147),e2147},{B04(02151),e2151},{B04(02155),e2155},{B04(02157),e2157},
 {B04(02161),e2161},{B04(02164),e2164},{B04(02167),e2167},{B04(02206),e2206},{B04(02210),e2210},{B04(02216),e2216},
 {B04(02222),e2222},{B04(02254),e2254},{B04(02316),e2316},{B04(02323),e2323},{B04(02330),e2330},{B04(02357),e2357},
 {B04(02362),e2362},{B04(02363),e2363},
};
const agc_routine_table lm_v37_routines={entries,LM_V37_ROUTINES};
