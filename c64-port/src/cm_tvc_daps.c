/* SPDX-License-Identifier: MIT */
#include "cm_tvc_daps.h"
/* Labels are the original bank-20 addresses. Every CCS takes the skips its
 * operand can produce and the two TS words that can overflow (TS L at 3141
 * after DOUBLE, TS TIME5 at 3114 after the AD chain) take their skip; the
 * other TS words store an MSU or MP result, a load, a constant or a CCS
 * result, none of which overflows. */
#define QRUPT 012u
#define BANKRUPT 016u
#define TIME5 030u
#define CDUY 033u
#define CDUZ 034u
#define TVCYAW 053u
#define TVCPITCH 054u
#define SINCDUX 0742u
#define COSCDUZ 0746u
#define COSCDUX 0750u
#define T5LOC 01312u
#define DAPDATR1 01466u
#define AK1 01477u
#define AK2 01500u
#define OMEGAYC 01527u
#define OMEGAZC 01531u
#define OMEGAYB 01535u
#define OMEGAZB 01537u
#define PTMP1 01541u
#define PTMP2 01543u
#define PTMP3 01545u
#define PTMP4 01547u
#define PTMP5 01551u
#define PTMP6 01553u
#define YTMP1 01555u
#define YTMP2 01557u
#define YTMP3 01561u
#define YTMP4 01563u
#define YTMP5 01565u
#define YTMP6 01567u
#define STROKER 01574u
#define PERRB 01575u
#define YERRB 01577u
#define DELPBAR 01601u
#define DELYBAR 01603u
#define PDELOFF 01605u
#define YDELOFF 01607u
#define TTMP1 01611u
#define TTMP2 01613u
#define DAP1 01615u
#define DAP2 01617u
#define DAP3 01621u
#define PCMD 01623u
#define YCMD 01624u
#define T5TVCDT 01625u
#define VARK 01641u
#define TVCPHASE 01644u
#define PCDUYPST 01645u
#define PCDUZPST 01646u
#define MCDUYDOT 01647u
#define MCDUZDOT 01650u
#define DELBRTMP 01700u
#define ERRBTMP 01702u
#define CMDTMP 01704u
#define TMP1 01706u
#define TMP2 01710u
#define TMP3 01712u
#define TMP4 01714u
#define TMP5 01716u
#define TMP6 01720u
#define N10 01723u
#define HACK 03533u
#define RESUME 05333u
#define NOQRSM 05335u
/* EXTEND MSU on an ordinary erasable word: two's-complement difference of
 * the 15-bit operands, sign-extended, minus one when negative. */
static void msu(agc_image *m, unsigned k) {
    unsigned a = agc_correct(m->words[0]) & 077777u, b = ~(unsigned)ai_word(m, k) & 077777u, d = a + b + 1u;
    if (d & 040000u) d = (d | 0100000u) - 1u;
    m->words[0] = agc_extend((agc_word)(d & 077777u));
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
/* EXTEND WOR: overflow-corrected A OR the channel, to both. */
static void wor(agc_image *m, agc_word *channel) {
    agc_word v = (agc_word)((agc_correct(m->words[0]) | *channel) & 077777u);
    *channel = v;
    m->words[0] = agc_extend(v);
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static enum cm_tvc_daps_status leave(agc_image *m, agc_word z, agc_word q) {
    if (q) ai_tc(m, q);
    m->words[2] &= 077777u;                          /* canonical frontier Q */
    m->words[5] = z;
    return CM_TVC_DAPS_OK;
}
enum cm_tvc_daps_status cm_tvc_daps(agc_image *m, cm_tvc_daps_io *io) {
    unsigned a;
    uint8_t ccs;
    if (m->words[4] != 040000u || m->words[3] != 03000u || m->words[6] != 040006u) return CM_TVC_DAPS_OUT_OF_SCOPE;
    switch (m->words[5]) {
    case 02530u: break;
    case 02541u: goto L2541;
    case 02543u: goto L2543;
    case 02732u: goto L2732;
    case 02743u: goto L2743;
    case 02745u: goto L2745;
    case 03107u: goto L3107;
    case 02675u: case 03051u:
        if (m->words[2] < 04000u || m->words[2] > 07777u) return CM_TVC_DAPS_OUT_OF_SCOPE;
        if (m->words[5] == 02675u) goto L2675;
        goto L3051;
    default: return CM_TVC_DAPS_OUT_OF_SCOPE;
    }
/* PITCHDAP */
    ai_lxch(m, BANKRUPT);
    ai_qxch(m, QRUPT);                               /* EXTEND; QXCH QRUPT */
    ai_caf(m, 02732u);                               /* YAWT5 */
    ai_ts(m, T5LOC);
    ai_ca(m, T5TVCDT);
    ai_ts(m, TIME5);
    ccs = ai_ccs(m, STROKER);                        /* PSTROKER */
    if (ccs == 0u) return leave(m, HACK, 02541u);    /* TC HACK */
    if (ccs == 2u) return leave(m, HACK, 02543u);    /* TC HACK */
    goto L2543;
L2541:                                               /* TCF +2 */
L2543:                                               /* PCDUDOTS */
    ai_ca(m, CDUY);
    ai_xch(m, PCDUYPST);
    msu(m, PCDUYPST);
    ai_tc(m, 02550u);                                /* TCR RLIMTEST */
    goto L2560;
L2550:
    ai_ts(m, MCDUYDOT);
    ai_ca(m, CDUZ);
    ai_xch(m, PCDUZPST);
    msu(m, PCDUZPST);
    ai_tc(m, 02556u);                                /* TCR RLIMTEST */
    goto L2560;
L2556:
    ai_ts(m, MCDUZDOT);
    goto L2571;
L2560:                                               /* RLIMTEST */
    ai_ts(m, TTMP1);
    ai_mpf(m, 0115u);                                /* 1/RTLIM */
    if (!ai_bzf(m)) {                                /* EXTEND; BZF +3 */
        ai_caf(m, 0u);                               /* ZERO */
        ai_ts(m, TTMP1);
    }
    ai_ca(m, TTMP1);
    goto TCQ;
L2571:                                               /* PINTEGRL */
    ai_dca(m, PERRB);
    ai_dxch(m, ERRBTMP);
    ai_dca(m, OMEGAYC);
    ai_das(m, ERRBTMP);
    ai_cs(m, COSCDUZ);
    ai_mp(m, COSCDUX);
    ai_ddoubl(m);
    ai_mp(m, MCDUYDOT);
    ai_ddoubl(m);
    ai_dxch(m, OMEGAYB);
    ai_cs(m, MCDUZDOT);
    ai_mp(m, SINCDUX);
    ai_ddoubl(m);
    ai_das(m, OMEGAYB);
    ai_dcs(m, OMEGAYB);
    ai_das(m, ERRBTMP);
    ai_tc(m, 02620u);                                /* PERORLIM: TCR ERRORLIM */
    goto L3124;
L2620:                                               /* PFORWARD */
    ai_dca(m, PTMP1);
    ai_dxch(m, TMP1);
    ai_dca(m, PTMP3);
    ai_dxch(m, TMP3);
    ai_dca(m, PTMP5);
    ai_dxch(m, TMP5);
    ai_tc(m, 02632u);                                /* TCR FWDFLTR */
    goto L3156;
L2632:                                               /* POFFSET */
    ai_dca(m, PDELOFF);
    ai_das(m, CMDTMP);
    ai_tc(m, 02636u);                                /* PACLIM: TCR ACTLIM */
    goto L3137;
L2636:                                               /* POUT */
    ai_cs(m, PCMD);
    ai_ad(m, CMDTMP);
    ai_ads(m, TVCPITCH);
    ai_caf(m, 02000u);                               /* BIT11 */
    wor(m, &io->channel14);                          /* EXTEND; WOR CHAN14 */
    ai_dca(m, PTMP2);                                /* PPRECOMP */
    ai_dxch(m, TMP2);
    ai_dca(m, PTMP4);
    ai_dxch(m, TMP4);
    ai_dca(m, PTMP6);
    ai_dxch(m, TMP6);
    ai_tc(m, 02656u);                                /* TCR PRECOMP */
    goto L3247;
L2656:                                               /* DELBARP */
    ai_ca(m, DELPBAR + 1u);
    ai_mpf(m, 037535u);                              /* E(-AT) */
    ai_ts(m, DELBRTMP + 1u);
    ai_ca(m, DELPBAR);
    ai_mpf(m, 037535u);                              /* E(-AT) */
    ai_das(m, DELBRTMP);
    ai_ca(m, CMDTMP);
    ai_mpf(m, 0243u);                                /* 1-E(-AT) */
    ai_das(m, DELBRTMP);
    ai_tc(m, 02673u);                                /* PCOPYCYC: TCR PCOPY */
    goto L2674;
L2673:                                               /* PDAPEND: TCF RESUME */
    return leave(m, RESUME, 0u);
L2674:                                               /* PCOPY */
    ai_incr(m, TVCPHASE);
L2675:
    ai_dca(m, TMP1);
    ai_dxch(m, PTMP1);
    ai_dca(m, TMP2);
    ai_dxch(m, PTMP2);
    ai_dca(m, TMP3);
    ai_dxch(m, PTMP3);
    ai_dca(m, TMP4);
    ai_dxch(m, PTMP4);
    ai_dca(m, TMP5);
    ai_dxch(m, PTMP5);
    ai_dca(m, TMP6);
    ai_dxch(m, PTMP6);
    ai_dca(m, ERRBTMP);                              /* PMISC */
    ai_ts(m, AK1);
    ai_dxch(m, PERRB);
    ai_ca(m, CMDTMP);
    ai_ts(m, PCMD);
    ai_dca(m, DELBRTMP);
    ai_dxch(m, DELPBAR);
    ai_incr(m, TVCPHASE);
    goto TCQ;
L2732:                                               /* YAWDAP */
    ai_lxch(m, BANKRUPT);
    ai_qxch(m, QRUPT);                               /* EXTEND; QXCH QRUPT */
    ai_caf(m, 02530u);                               /* PITCHT5 */
    ai_ts(m, T5LOC);
    ai_ca(m, T5TVCDT);
    ai_ts(m, TIME5);
    ccs = ai_ccs(m, STROKER);                        /* YSTROKER */
    if (ccs == 0u) return leave(m, HACK, 02743u);    /* TC HACK */
    if (ccs == 2u) return leave(m, HACK, 02745u);    /* TC HACK */
    goto L2745;
L2743:                                               /* TCF +2 */
L2745:                                               /* YINTEGRL */
    ai_dca(m, YERRB);
    ai_dxch(m, ERRBTMP);
    ai_dca(m, OMEGAZC);
    ai_das(m, ERRBTMP);
    ai_ca(m, COSCDUZ);
    ai_mp(m, SINCDUX);
    ai_ddoubl(m);
    ai_mp(m, MCDUYDOT);
    ai_ddoubl(m);
    ai_dxch(m, OMEGAZB);
    ai_cs(m, MCDUZDOT);
    ai_mp(m, COSCDUX);
    ai_ddoubl(m);
    ai_das(m, OMEGAZB);
    ai_dcs(m, OMEGAZB);
    ai_das(m, ERRBTMP);
    ai_tc(m, 02774u);                                /* YERORLIM: TCR ERRORLIM */
    goto L3124;
L2774:                                               /* YFORWARD */
    ai_dca(m, YTMP1);
    ai_dxch(m, TMP1);
    ai_dca(m, YTMP3);
    ai_dxch(m, TMP3);
    ai_dca(m, YTMP5);
    ai_dxch(m, TMP5);
    ai_tc(m, 03006u);                                /* TCR FWDFLTR */
    goto L3156;
L3006:                                               /* YOFFSET */
    ai_dca(m, YDELOFF);
    ai_das(m, CMDTMP);
    ai_tc(m, 03012u);                                /* YACLIM: TCR ACTLIM */
    goto L3137;
L3012:                                               /* YOUT */
    ai_cs(m, YCMD);
    ai_ad(m, CMDTMP);
    ai_ads(m, TVCYAW);
    ai_caf(m, 04000u);                               /* BIT12 */
    wor(m, &io->channel14);                          /* EXTEND; WOR CHAN14 */
    ai_dca(m, YTMP2);                                /* YPRECOMP */
    ai_dxch(m, TMP2);
    ai_dca(m, YTMP4);
    ai_dxch(m, TMP4);
    ai_dca(m, YTMP6);
    ai_dxch(m, TMP6);
    ai_tc(m, 03032u);                                /* TCR PRECOMP */
    goto L3247;
L3032:                                               /* DELBARY */
    ai_ca(m, DELYBAR + 1u);
    ai_mpf(m, 037535u);                              /* E(-AT) */
    ai_ts(m, DELBRTMP + 1u);
    ai_ca(m, DELYBAR);
    ai_mpf(m, 037535u);                              /* E(-AT) */
    ai_das(m, DELBRTMP);
    ai_ca(m, CMDTMP);
    ai_mpf(m, 0243u);                                /* 1-E(-AT) */
    ai_das(m, DELBRTMP);
    ai_tc(m, 03047u);                                /* YCOPYCYC: TCR YCOPY */
    goto L3050;
L3047:                                               /* YDAPEND: TCF RESUME */
    return leave(m, RESUME, 0u);
L3050:                                               /* YCOPY */
    ai_incr(m, TVCPHASE);
L3051:
    ai_dca(m, TMP1);
    ai_dxch(m, YTMP1);
    ai_dca(m, TMP2);
    ai_dxch(m, YTMP2);
    ai_dca(m, TMP3);
    ai_dxch(m, YTMP3);
    ai_dca(m, TMP4);
    ai_dxch(m, YTMP4);
    ai_dca(m, TMP5);
    ai_dxch(m, YTMP5);
    ai_dca(m, TMP6);
    ai_dxch(m, YTMP6);
    ai_dca(m, ERRBTMP);                              /* YMISC */
    ai_ts(m, AK2);
    ai_dxch(m, YERRB);
    ai_ca(m, CMDTMP);
    ai_ts(m, YCMD);
    ai_dca(m, DELBRTMP);
    ai_dxch(m, DELYBAR);
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, TVCPHASE);
    goto TCQ;
L3107:                                               /* DAPINIT */
    ai_lxch(m, BANKRUPT);
    ai_caf(m, 077776u);                              /* NEGONE */
    ai_ad(m, T5TVCDT);
    ai_adf(m, 040000u);                              /* NEGMAX */
    ai_ad(m, T5TVCDT);
    if (ai_ts(m, TIME5)) goto L3116;                 /* the skip leaves A = +-1 */
    ai_caf(m, 02530u);                               /* PITCHT5 */
L3116:
    ai_ts(m, T5LOC);
    ai_ca(m, CDUY);
    ai_ts(m, PCDUYPST);
    ai_ca(m, CDUZ);
    ai_ts(m, PCDUZPST);
    return leave(m, NOQRSM, 0u);
L3124:                                               /* ERRORLIM */
    ai_ca(m, ERRBTMP);
    ai_mpf(m, 04u);                                  /* 1/ERRLIM */
    if (!ai_bzf(m)) {                                /* EXTEND; BZF +6 */
        ccs = ai_ccs(m, ERRBTMP);
        if (ccs == 0u) ai_caf(m, 010000u);           /* ERRLIM */
        else if (ccs == 2u) ai_csf(m, 010000u);      /* ERRLIM */
        ai_ts(m, ERRBTMP);                           /* +0 and -0 store the CCS +0 */
    }
    goto TCQ;
L3137:                                               /* ACTLIM */
    ai_ca(m, CMDTMP + 1u);
    ai_double(m);
    if (ai_ts(m, 1u)) goto L3143;                    /* TS L: the skip keeps A = +-1 */
    ai_caf(m, 0u);                                   /* ZERO */
L3143:
    ai_ad(m, CMDTMP);
    ai_mpf(m, 0101u);                                /* 1/ACTSAT */
    if (!ai_bzf(m)) {                                /* EXTEND; BZF +6 */
        ccs = ai_ccs(m, CMDTMP);
        if (ccs == 0u) ai_caf(m, 0375u);             /* ACTSAT */
        else if (ccs == 2u) ai_csf(m, 0375u);        /* ACTSAT */
        ai_ts(m, CMDTMP);
    }
    goto TCQ;
L3156:                                               /* FWDFLTR */
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, DAP1);
    ai_ts(m, DAP2);
    ai_ts(m, DAP3);
    ai_ts(m, CMDTMP);
    ai_ts(m, DELBRTMP);
    ai_ca(m, ERRBTMP + 1u);                          /* 1DAPCAS */
    ai_mp(m, N10);
    ai_ts(m, DAP1 + 1u);
    ai_ca(m, ERRBTMP);
    ai_mp(m, N10);
    ai_das(m, DAP1);
    ai_dxch(m, TMP1);
    ai_das(m, DAP1);
    ai_ca(m, DAP1 + 1u);                             /* 2DAPCAS */
    ai_mp(m, N10 + 5u);
    ai_ts(m, DAP2 + 1u);
    ai_ca(m, DAP1);
    ai_mp(m, N10 + 5u);
    ai_das(m, DAP2);
    ai_dxch(m, TMP3);
    ai_das(m, DAP2);
    ai_ca(m, DAPDATR1);
    ai_maskf(m, 020000u);                            /* BIT14 */
    /* CCS A: the mask leaves +0 or BIT14, so only slots 0 and 1 occur. */
    if (ai_ccs(m, 0u) == 0u) goto L3220;             /* TCF 3DAPCAS */
    ai_dca(m, DAP2);
    ai_dxch(m, DAP3);
    goto L3232;                                      /* TCF OPTVARK */
L3220:                                               /* 3DAPCAS */
    ai_ca(m, DAP2 + 1u);
    ai_mp(m, N10 + 10u);
    ai_ts(m, DAP3 + 1u);
    ai_ca(m, DAP2);
    ai_mp(m, N10 + 10u);
    ai_das(m, DAP3);
    ai_dxch(m, TMP5);
    ai_das(m, DAP3);
L3232:                                               /* OPTVARK */
    ai_cs(m, DAP3 + 1u);
    ai_mp(m, VARK);
    ai_ts(m, CMDTMP + 1u);
    ai_cs(m, DAP3);
    ai_mp(m, VARK);
    ai_das(m, CMDTMP);
    ai_dxch(m, CMDTMP);
    ai_ddoubl(m);
    ai_ddoubl(m);
    ai_dxch(m, CMDTMP);
    goto TCQ;
L3247:                                               /* PRECOMP */
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, TTMP1);
    ai_ts(m, TTMP2);
    ai_ca(m, ERRBTMP + 1u);
    ai_mp(m, N10 + 1u);
    ai_ts(m, TTMP1 + 1u);
    ai_ca(m, ERRBTMP);
    ai_mp(m, N10 + 1u);
    ai_das(m, TTMP1);
    ai_cs(m, DAP1 + 1u);
    ai_mp(m, N10 + 3u);
    ai_ts(m, TTMP2 + 1u);
    ai_cs(m, DAP1);
    ai_mp(m, N10 + 3u);
    ai_das(m, TTMP2);
    ai_dxch(m, TTMP2);
    ai_das(m, TTMP1);
    ai_dxch(m, TTMP1);
    ai_ddoubl(m);
    ai_das(m, TMP2);
    ai_dxch(m, TMP2);
    ai_dxch(m, TMP1);
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, TTMP1);
    ai_ts(m, TMP2);
    ai_ca(m, ERRBTMP + 1u);
    ai_mp(m, N10 + 2u);
    ai_ts(m, TTMP1 + 1u);
    ai_ca(m, ERRBTMP);
    ai_mp(m, N10 + 2u);
    ai_das(m, TTMP1);
    ai_cs(m, DAP1 + 1u);
    ai_mp(m, N10 + 4u);
    ai_ts(m, TMP2 + 1u);
    ai_cs(m, DAP1);
    ai_mp(m, N10 + 4u);
    ai_das(m, TMP2);
    ai_dxch(m, TTMP1);
    ai_das(m, TMP2);
    ai_caf(m, 0u);                                   /* 2CASFLTR: ZERO */
    ai_ts(m, TTMP1);
    ai_ts(m, TTMP2);
    ai_ca(m, DAP1 + 1u);
    ai_mp(m, N10 + 6u);
    ai_ts(m, TTMP1 + 1u);
    ai_ca(m, DAP1);
    ai_mp(m, N10 + 6u);
    ai_das(m, TTMP1);
    ai_cs(m, DAP2 + 1u);
    ai_mp(m, N10 + 8u);
    ai_ts(m, TTMP2 + 1u);
    ai_cs(m, DAP2);
    ai_mp(m, N10 + 8u);
    ai_das(m, TTMP2);
    ai_dxch(m, TTMP2);
    ai_das(m, TTMP1);
    ai_dxch(m, TTMP1);
    ai_ddoubl(m);
    ai_das(m, TMP4);
    ai_dxch(m, TMP4);
    ai_dxch(m, TMP3);
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, TTMP1);
    ai_ts(m, TMP4);
    ai_ca(m, DAP1 + 1u);
    ai_mp(m, N10 + 7u);
    ai_ts(m, TTMP1 + 1u);
    ai_ca(m, DAP1);
    ai_mp(m, N10 + 7u);
    ai_das(m, TTMP1);
    ai_cs(m, DAP2 + 1u);
    ai_mp(m, N10 + 9u);
    ai_ts(m, TMP4 + 1u);
    ai_cs(m, DAP2);
    ai_mp(m, N10 + 9u);
    ai_das(m, TMP4);
    ai_dxch(m, TTMP1);
    ai_das(m, TMP4);
    ai_ca(m, DAPDATR1);
    ai_maskf(m, 010000u);                            /* BIT13 */
    /* CCS A: the mask leaves +0 or BIT13, so only slots 0 and 1 occur. */
    if (ai_ccs(m, 0u) == 0u) goto TCQ;               /* TC Q */
    ai_caf(m, 0u);                                   /* 3CASFLTR: ZERO */
    ai_ts(m, TTMP1);
    ai_ts(m, TTMP2);
    ai_ca(m, DAP2 + 1u);
    ai_mp(m, N10 + 11u);
    ai_ts(m, TTMP1 + 1u);
    ai_ca(m, DAP2);
    ai_mp(m, N10 + 11u);
    ai_das(m, TTMP1);
    ai_cs(m, DAP3 + 1u);
    ai_mp(m, N10 + 13u);
    ai_ts(m, TTMP2 + 1u);
    ai_cs(m, DAP3);
    ai_mp(m, N10 + 13u);
    ai_das(m, TTMP2);
    ai_dxch(m, TTMP2);
    ai_das(m, TTMP1);
    ai_dxch(m, TTMP1);
    ai_ddoubl(m);
    ai_das(m, TMP6);
    ai_dxch(m, TMP6);
    ai_dxch(m, TMP5);
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, TTMP1);
    ai_ts(m, TMP6);
    ai_ca(m, DAP2 + 1u);
    ai_mp(m, N10 + 12u);
    ai_ts(m, TTMP1 + 1u);
    ai_ca(m, DAP2);
    ai_mp(m, N10 + 12u);
    ai_das(m, TTMP1);
    ai_cs(m, DAP3 + 1u);
    ai_mp(m, N10 + 14u);
    ai_ts(m, TMP6 + 1u);
    ai_cs(m, DAP3);
    ai_mp(m, N10 + 14u);
    ai_das(m, TMP6);
    ai_dxch(m, TTMP1);
    ai_das(m, TMP6);
TCQ:                                                 /* TC Q runs the TC held in Q */
    a = m->words[2] & 077777u;
    ai_tc(m, 3u);
    switch (a) {
    case 02550u: goto L2550;
    case 02556u: goto L2556;
    case 02620u: goto L2620;
    case 02632u: goto L2632;
    case 02636u: goto L2636;
    case 02656u: goto L2656;
    case 02673u: goto L2673;
    case 02774u: goto L2774;
    case 03006u: goto L3006;
    case 03012u: goto L3012;
    case 03032u: goto L3032;
    case 03047u: goto L3047;
    default: break;
    }
    m->words[5] = (agc_word)a;
    return CM_TVC_DAPS_OK;
}
