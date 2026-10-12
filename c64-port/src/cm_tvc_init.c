/* SPDX-License-Identifier: MIT */
#include "cm_tvc_init.h"
/* Labels are the original bank-17 addresses. Every CCS takes its four-way
 * skip and the TS words that can overflow (MASSTMP, 1/CONACC, VARK) their
 * skips; the other TS words store constants, loads, CCS or MP results. */
#define QRUPT 012u
#define BANKRUPT 016u
#define TIME5 030u
#define CDUX 032u
#define FLAGWRD9 0105u
#define EMDOT 0110u
#define T5LOC 01312u
#define EKPRIME 01414u
#define EKTLX_I 01416u
#define EREPFRAC 01421u
#define PACTOFF 01423u
#define YACTOFF 01424u
#define V97VCNTR 01444u
#define CNTR 01447u
#define DAPDATR1 01466u
#define IXX 01470u
#define IAVG_TLX 01472u
#define CSMMASS 01474u
#define RCSFLAGS 01501u
#define OMEGAYC 01527u
#define PTMP1_1 01540u
#define PERRB 01575u
#define YERRB 01577u
#define DELPBAR 01601u
#define DELYBAR 01603u
#define PDELOFF 01605u
#define YDELOFF 01607u
#define TTMP1 01611u
#define PHASETMP 01611u
#define RTRNLOC 01613u
#define PCMD 01623u
#define YCMD 01624u
#define T5TVCDT 01625u
#define KPRIMEDT 01634u
#define KTLX_I 01636u
#define TENMDOT 01637u
#define CONACC_1 01640u
#define VARK 01641u
#define REPFRAC 01642u
#define VCNTR 01643u
#define TVCPHASE 01644u
#define MASSTMP 01652u
#define STRKTIME 01654u
#define OGANOW 01662u
#define CNTRTMP 01676u
#define ERRBTMP 01702u
#define COEFFADR 01722u
#define N10 01723u
#define POSTJUMP 04701u
#define IBNKCALL 04740u
#define WAITLIST 05245u
#define RESUME 05333u
/* CSMN10 (2500) through D32 (2530): the CSM and low-bandwidth filter
 * coefficients LOADCOEF copies from fixed memory. */
static const agc_word coef[] = {
    037777u, 067657u, 001703u, 047472u, 027656u, 037777u, 060361u, 000000u, 067304u, 000000u,
    037777u, 065371u, 065337u, 042700u, 033045u, 001000u, 000000u, 000000u, 042700u, 033045u,
    020000u, 060730u, 017145u, 041323u, 035773u
};
static const agc_word initloc2[2] = { 002171u, 036066u };   /* INITLOC2: 2CADR TVCINIT1 */
static const agc_word dapinit5[2] = { 003107u, 040066u };   /* DAPINIT5: 2CADR DAPINIT */
/* EXTEND RAND 11 (DSALMOUT): overflow-corrected A AND the channel. */
static void rand11(agc_image *m, agc_word channel) {
    m->words[0] = agc_extend((agc_word)(agc_correct(m->words[0]) & channel & 077777u));
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static enum cm_tvc_init_status leave(agc_image *m, agc_word z, agc_word q) {
    if (q) ai_tc(m, q);
    m->words[2] &= 077777u;                          /* canonical frontier Q */
    m->words[5] = z;
    return CM_TVC_INIT_OK;
}
/* LOADCOEF's INDEX COEFFADR word: COEFFADR is CSMN10 or LBN10 (fixed) or
 * HBN10 (erasable), as set by this file. */
static void dca_coef(agc_image *m, unsigned k) {
    unsigned a = ai_word(m, COEFFADR) + k;
    if (a >= 02000u) ai_dcaf(m, &coef[a - 02500u]);
    else ai_dca(m, a);
}
enum cm_tvc_init_status cm_tvc_init(agc_image *m, cm_tvc_init_io *io) {
    uint8_t ccs;
    unsigned k, a;
    if (m->words[4] != 036000u || m->words[3] != 03000u || m->words[6] != 036006u) return CM_TVC_INIT_OUT_OF_SCOPE;
    switch (m->words[5]) {
    case 02147u: goto L2147;
    case 02152u: goto L2152;
    case 02171u: goto L2171;
    case 02176u: goto L2176;
    case 02324u: goto L2324;
    case 02343u: goto L2343;
    case 02351u: goto L2351;
    case 02361u: goto L2361;
    case 02462u:
        if (m->words[2] < 04000u || m->words[2] > 07777u) return CM_TVC_INIT_OUT_OF_SCOPE;
        goto L2462;
    default: return CM_TVC_INIT_OUT_OF_SCOPE;
    }
L2147:                                               /* TVCDAPON */
    ai_lxch(m, BANKRUPT);
    ai_qxch(m, QRUPT);
L2152:                                               /* MRCLEAN */
    ai_caf(m, 063u);                                 /* NZERO */
L2153:
    ccs = ai_ccs(m, 0u);
    if (ccs == 1u) goto L2155;
    if (ccs == 2u) goto L2156;
    if (ccs == 3u) goto L2157;
    ai_ts(m, CNTR);
L2155:
    ai_caf(m, 0u);                                   /* ZERO */
L2156:
    ai_ts(m, 1u);                                    /* TS L */
L2157:
    ai_dxch(m, OMEGAYC + ai_word(m, CNTR));          /* INDEX CNTR */
    ccs = ai_ccs(m, CNTR);
    if (ccs == 0u) goto L2153;                       /* TCF MRCLEAN +1 */
    if (ccs == 2u) goto L2164;                       /* lands on DCA: runs as CA INITLOC2 +1 */
    if (ccs == 3u) goto L2165;
    ai_dcaf(m, initloc2);
L2165:
    ai_dxch(m, T5LOC);
    ai_caf(m, 037777u);                              /* POSMAX */
    ai_ts(m, TIME5);
    return leave(m, RESUME, 0u);
L2164:
    ai_caf(m, initloc2[1]);
    goto L2165;
L2171:                                               /* TVCINIT1 */
    ai_lxch(m, BANKRUPT);
    ai_qxch(m, QRUPT);
    return leave(m, IBNKCALL, 02175u);               /* CADR MASSPROP */
L2176:
    ai_ca(m, EMDOT);
    ai_mpf(m, 03720u);                               /* ONETHOU */
    ai_ts(m, TENMDOT);
    ai_com(m);
    ai_ad(m, CSMMASS);
    if (ai_ts(m, MASSTMP)) goto L2206;
    ai_ca(m, DAPDATR1);
L2206:
    ai_maskf(m, 020000u);                            /* BIT14 */
    ccs = ai_ccs(m, 0u);
    if (ccs == 0u) ai_caf(m, 1u);                    /* BIT1 */
    ai_ts(m, CNTR);                                  /* A is BIT1 or +0 here */
    ai_caf(m, ai_word(m, CNTR) ? 01425u : 02500u);   /* INDEX CNTR; CSMCFADR */
    ai_ts(m, COEFFADR);
    ai_tc(m, 02216u);                                /* TC LOADCOEF */
    goto L2422;
L2216:
    ai_ca(m, EKTLX_I + ai_word(m, CNTR));            /* INDEX CNTR */
    ai_ts(m, KTLX_I);
    ai_tc(m, 02222u);                                /* TCR S40.15 */
    goto L2462;
L2222:                                               /* TVCINIT2 */
    ai_cs(m, CNTR);
    ai_caf(m, agc_to_int(m->words[0] & 077777u) ? 04u : 02u);   /* INDEX A; BIT2 (BIT3 at -1) */
    ai_ts(m, KPRIMEDT);
    ai_com(m);
    ai_adf(m, 037777u);                              /* POSMAX */
    ai_adf(m, 1u);                                   /* BIT1 */
    ai_ts(m, T5TVCDT);
    ai_csf(m, 040000u);                              /* BIT15 */
    ai_mask(m, FLAGWRD9);
    ai_ts(m, FLAGWRD9);
    ai_ca(m, EKPRIME + ai_word(m, CNTR));            /* INDEX CNTR */
    ai_mp(m, KPRIMEDT);
    ai_lxch(m, 0u);
    ai_dxch(m, KPRIMEDT);
    ai_ca(m, EREPFRAC + ai_word(m, CNTR));           /* INDEX CNTR */
    ai_ts(m, REPFRAC);
    ai_caf(m, ai_word(m, CNTR) ? 0u : 05u);          /* INDEX CNTR; TCORR */
    ai_ts(m, CNTR);
    ai_caf(m, 077776u);                              /* NEGONE */
    ai_ts(m, STRKTIME);
    ai_caf(m, 023u);                                 /* NINETEEN */
    ai_ts(m, VCNTR);
    ai_ts(m, V97VCNTR);
    ai_ca(m, PACTOFF);                               /* TVCINIT3 */
    ai_ts(m, PDELOFF);
    ai_ts(m, PCMD);
    ai_ts(m, DELPBAR);
    ai_ca(m, YACTOFF);
    ai_ts(m, YDELOFF);
    ai_ts(m, YCMD);
    ai_ts(m, DELYBAR);
    ai_ca(m, DAPDATR1);                              /* ATTINIT */
    ai_maskf(m, 010000u);                            /* BIT13 */
    if (ai_bzf(m)) goto L2317;                       /* EXTEND; BZF NEEDLEIN */
    ai_caf(m, 1u);                                   /* BIT1 */
L2273:
    ai_ts(m, TTMP1);
    ai_ca(m, ERRBTMP + ai_word(m, TTMP1));           /* INDEX TTMP1 */
    ai_mpf(m, 0170u);                                /* 1/ATTLIM */
    if (ai_bzf(m)) goto L2311;                       /* EXTEND; BZF +8D */
    /* EXTEND; BZMF +3: CS ATTLIM, else CA ATTLIM; TCF +2 */
    if (ai_bzmf(m)) ai_csf(m, 0210u);                /* ATTLIM */
    else ai_caf(m, 0210u);                           /* ATTLIM */
    ai_ts(m, ERRBTMP + ai_word(m, TTMP1));           /* INDEX TTMP1 */
L2311:
    ccs = ai_ccs(m, TTMP1);
    if (ccs == 0u) goto L2273;                       /* TCF ATTINIT +5 */
    if (ccs == 2u) goto L2314;                       /* TTMP1 is 1 or +0 here */
    if (ccs == 3u) goto L2315;
    ai_ca(m, ERRBTMP);
L2314:
    ai_ts(m, PERRB);
L2315:
    ai_ca(m, ERRBTMP + 1u);
    ai_ts(m, YERRB);
L2317:                                               /* NEEDLEIN */
    ai_cs(m, RCSFLAGS);
    ai_maskf(m, 4u);                                 /* BIT3 */
    ai_ads(m, RCSFLAGS);
    return leave(m, IBNKCALL, 02323u);               /* CADR NEEDLER */
L2324:                                               /* TVCINIT4 */
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, TVCPHASE);
    ai_ca(m, CDUX);
    ai_ts(m, OGANOW);
    ai_caf(m, 010000u);                              /* BIT13 */
    rand11(m, io->channel11);
    if (ai_bzf(m)) goto L2337;                       /* EXTEND; BZF +3 */
    ai_ca(m, MASSTMP);
    ai_ts(m, CSMMASS);
L2337:
    ai_caf(m, 062u);                                 /* .5SEC */
    return leave(m, WAITLIST, 02341u);               /* 2CADR TVCEXEC */
L2343:
    ai_dcaf(m, dapinit5);
    ai_dxch(m, T5LOC);
    ai_ca(m, T5TVCDT);
    ai_ts(m, TIME5);
    return leave(m, RESUME, 0u);
L2351:                                               /* PRESWTCH: TCR SWICHOVR */
    ai_tc(m, 02352u);
    io->interrupt_enabled = 0u;                      /* SWICHOVR: INHINT */
    ai_ca(m, TVCPHASE);
    ai_ts(m, PHASETMP);
    ai_csf(m, 2u);                                   /* BIT2 */
    ai_ts(m, TVCPHASE);
L2361:
    ai_qxch(m, RTRNLOC);
    ai_caf(m, 027u);                                 /* NZEROJR */
L2364:
    ai_ts(m, CNTRTMP);                               /* then MCLEANJR */
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, 1u);                                    /* TS L */
    ai_dxch(m, PTMP1_1 + ai_word(m, CNTRTMP));       /* INDEX CNTRTMP */
    ccs = ai_ccs(m, CNTRTMP);                        /* CNTRTMP is odd and positive here */
    if (ccs == 1u) goto L2364;
    if (ccs == 2u) goto L2374;
    if (ccs == 3u) goto L2375;
    ccs = ai_ccs(m, 0u);                             /* A is CNTRTMP - 1 */
    if (ccs == 0u) goto L2364;
    if (ccs == 2u) goto L2375;
    if (ccs == 3u) goto L2376;
L2374:
    ai_cs(m, FLAGWRD9);
L2375:
    ai_maskf(m, 040000u);                            /* BIT15 */
L2376:
    ai_ads(m, FLAGWRD9);
    ai_ca(m, EKTLX_I + 2u);
    ai_ts(m, KTLX_I);
    ai_tc(m, 02402u);                                /* TCR S40.15 +7 */
    goto L2471;
L2402:
    ai_caf(m, 0247u);                                /* FKPRIMDT */
    ai_ts(m, KPRIMEDT);
    ai_caf(m, 0232u);                                /* FREPFRAC */
    ai_ts(m, REPFRAC);
    ai_dca(m, DELPBAR);
    ai_dxch(m, PDELOFF);
    ai_dca(m, DELYBAR);
    ai_dxch(m, YDELOFF);
    ai_caf(m, 02512u);                               /* LBCFADR */
    ai_ts(m, COEFFADR);
    ai_tc(m, 02417u);                                /* TC LOADCOEF */
    goto L2422;
L2417:
    ai_ca(m, PHASETMP);
    ai_ts(m, TVCPHASE);
    return leave(m, RTRNLOC, 02422u);                /* TC RTRNLOC */
L2422:                                               /* LOADCOEF */
    for (k = 0; k < 14u; k += 2u) {
        dca_coef(m, k);
        ai_dxch(m, N10 + k);
    }
    a = ai_word(m, COEFFADR) + 14u;
    if (a >= 02000u) ai_caf(m, coef[a - 02500u]);
    else ai_ca(m, a);
    ai_ts(m, N10 + 14u);
    goto TCQ;
L2462:                                               /* S40.15 */
    ai_ca(m, IXX);
    ai_mpf(m, 033074u);                              /* 2PI/M */
    ai_ddoubl(m);
    ai_ddoubl(m);
    ai_ddoubl(m);
    if (ai_ts(m, CONACC_1)) goto L2472;
L2471:
    ai_ca(m, KTLX_I);
L2472:
    ai_mp(m, IAVG_TLX);
    ai_ddoubl(m);
    ai_ddoubl(m);
    if (ai_ts(m, VARK)) {
        /* Overflow skips TC Q into the CSMN10 data: CA 7777, AD 7657, TC 1703. */
        ai_caf(m, 072635u);
        ai_adf(m, 022000u);
        return leave(m, 01703u, 02503u);
    }
TCQ:                                                 /* TC Q runs the TC held in Q */
    a = m->words[2] & 077777u;
    ai_tc(m, 3u);
    if (a == 02216u) goto L2216;
    if (a == 02222u) goto L2222;
    if (a == 02402u) goto L2402;
    if (a == 02417u) goto L2417;
    m->words[5] = (agc_word)a;
    return CM_TVC_INIT_OK;
}
