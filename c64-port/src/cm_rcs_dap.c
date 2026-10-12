/* SPDX-License-Identifier: MIT */
#include "cm_rcs_dap.h"
/* Labels are the original bank-21 addresses. Every CCS takes the slots its
 * operand can produce and the TS words that can overflow take their skip
 * (TS T5TEMP at 2512, TS TIME5 at 3151, TS T5TEMP at 3130 from a caller's
 * raw A); the other TS words store a load, a constant, a mask, an MP or MSU
 * result or a small index. INDEX words add the file's own loop counters
 * (SPNDX, DPNDX), STICKCHK's 0..3 manual-rate indices, ATTKALMN and
 * RATEINDX to the next word's address. */
#define QRUPT 012u
#define BANKRUPT 016u
#define TIME5 030u
#define TIME6 031u
#define CDUX 032u
#define CDUY 033u
#define CDUZ 034u
#define CDUXCMD 050u
#define FLAGWRD0 074u
#define FLAGWRD1 075u
#define FLAGWRD9 0105u
#define RATEINDX 01130u
#define CPHI 01155u
#define CTHETA 01156u
#define T6LOC 01310u
#define T5LOC 01312u
#define IMODES33 01321u
#define HOLDFLAG 01332u
#define CPHIX 01333u
#define RWORD1 01451u
#define BLAST1 01461u
#define BLAST2 01463u
#define T5PHASE 01465u
#define AK 01476u
#define AK1 01477u
#define AK2 01500u
#define RCSFLAGS 01501u
#define T5TEMP 01502u
#define EDRIVEX 01503u
#define SPNDX 01506u
#define DPNDX 01507u
#define KMPAC 01510u
#define WTEMP 01513u
#define DELTEMPX 01515u
#define DELTEMPY 01517u
#define DELTEMPZ 01521u
#define WBODY 01525u
#define ADOT 01533u
#define MERRORX 01541u
#define DFT 01547u
#define DRHO 01552u
#define ATTSEC 01560u
#define TAU 01561u
#define TAU1 01562u
#define TAU2 01563u
#define ERRORX 01567u
#define ERRORY 01570u
#define ERRORZ 01571u
#define THETADX 01572u
#define DELCDUX 01575u
#define ATTKALMN 01617u
#define KMJ 01620u
#define CH31TEMP 01632u
#define CHANTEMP 01633u
#define T5TIME 01634u
#define RHO 01635u
#define RHO1 01636u
#define RHO2 01637u
#define AMGB1 01640u
#define AMGB4 01641u
#define AMGB5 01642u
#define AMGB7 01643u
#define AMGB8 01644u
#define CDUXD 01646u
#define SLOPE 01654u
#define RMANNDX 01656u
#define PMANNDX 01657u
#define YMANNDX 01660u
#define SMALLMP 02042u
#define DPADD 02054u
#define AHFNOROT 03250u
#define JETS 03441u
#define T6PROG 03757u
#define IBNKCALL 04740u
#define NOVAC 05134u
#define RESUME 05333u
static const agc_word rcsloc[2] = { 002122u, 042066u };    /* RCSLOC: 2CADR RCSATT */
static const agc_word t6addr[2] = { 003617u, 036066u };    /* T6ADDR: 2CADR T6START */
/* Words the INDEX instructions read: 3074-3130 (GAIN1-3 through GAIN2+11,
 * which is STICKCHK's first word), MANTABLE-1 through =+14MS (3037-3050),
 * FREETAU (3063-3066), DELAY200+1 through DACLIMIT+1 (2541-2543). */
static const agc_word gains[] = {
    006604u, 032703u, 006604u, 002031u, 012132u, 013030u, 014047u, 015241u, 016650u, 020555u,
    023065u, 026137u, 032053u, 035712u, 000435u, 013412u, 000435u, 000032u, 001350u, 001575u,
    002103u, 002523u, 003327u, 004432u, 006264u, 011351u, 017324u, 033622u, 055502u
};
static const agc_word mantable[] = {
    016000u, 000165u, 077612u, 000722u, 077055u, 002215u, 075562u, 011064u, 066713u, 000027u
};
static const agc_word freetau[] = { 000000u, 000740u, 077037u, 000000u };
static const agc_word daclimit[] = { 077177u, 037200u, 000600u };
static agc_word gain(const agc_image *m, unsigned base) {
    return gains[(unsigned)((int)base + agc_to_int(ai_word(m, ATTKALMN))) - 03074u];
}
/* EXTEND MSU on an ordinary erasable word: two's-complement difference of
 * the 15-bit operands, sign-extended, minus one when negative. */
static void msu(agc_image *m, unsigned k) {
    unsigned a = agc_correct(m->words[0]) & 077777u, b = ~(unsigned)ai_word(m, k) & 077777u, d = a + b + 1u;
    if (d & 040000u) d = (d | 0100000u) - 1u;
    m->words[0] = agc_extend((agc_word)(d & 077777u));
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
/* EXTEND READ/RAND/WAND/WOR on an I/O channel, and RXOR LCHAN (channel 1
 * is L: the raw 16-bit registers), as yaAGC defines them. */
static void read(agc_image *m, agc_word channel) {
    m->words[0] = agc_extend((agc_word)(channel & 077777u));
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static void rand(agc_image *m, agc_word channel) {
    m->words[0] = agc_extend((agc_word)(agc_correct(m->words[0]) & channel & 077777u));
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static void wand(agc_image *m, agc_word *channel) {
    agc_word v = (agc_word)(agc_correct(m->words[0]) & *channel & 077777u);
    *channel = v;
    m->words[0] = agc_extend(v);
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static void wor(agc_image *m, agc_word *channel) {
    agc_word v = (agc_word)((agc_correct(m->words[0]) | *channel) & 077777u);
    *channel = v;
    m->words[0] = agc_extend(v);
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static void rxorl(agc_image *m) {
    m->words[0] = (agc_word)(m->words[0] ^ m->words[1]);
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static enum cm_rcs_dap_status leave(agc_image *m, agc_word z, agc_word q) {
    if (q) ai_tc(m, q);
    m->words[2] &= 077777u;                          /* canonical frontier Q */
    m->words[5] = z;
    return CM_RCS_DAP_OK;
}
static uint8_t in(const agc_image *m, unsigned k, int lo, int hi) {
    agc_word v = ai_word(m, k);
    return v != 077777u && agc_to_int(v) >= lo && agc_to_int(v) <= hi;
}
enum cm_rcs_dap_status cm_rcs_dap(agc_image *m, cm_rcs_dap_io *io) {
    unsigned a, sp, dp;
    uint8_t ccs;
    if (m->words[4] != 042000u || m->words[3] != 03000u || m->words[6] != 042006u) return CM_RCS_DAP_OUT_OF_SCOPE;
    switch (m->words[5]) {
    case 02105u: case 02122u:
        if (!in(m, ATTKALMN, -1, 11) || !in(m, RATEINDX, 0, 6)) return CM_RCS_DAP_OUT_OF_SCOPE;
        break;
    case 02236u:
        if (!in(m, ATTKALMN, -1, 11)) return CM_RCS_DAP_OUT_OF_SCOPE;
        /* fall through */
    case 02401u:
        if (!in(m, SPNDX, 0, 2) || ai_word(m, DPNDX) != 2u * ai_word(m, SPNDX)) return CM_RCS_DAP_OUT_OF_SCOPE;
        break;
    case 02360u: case 02616u: case 02550u: case 02552u: case 02534u: case 03067u: break;
    case 02420u: case 02443u: case 02476u: case 02632u: case 03130u:
        a = m->words[2];
        if (!((a >= 03250u && a <= 03764u) || (a >= 04000u && a <= 07777u))) return CM_RCS_DAP_OUT_OF_SCOPE;
        break;
    default: return CM_RCS_DAP_OUT_OF_SCOPE;
    }
    switch (m->words[5]) {
    case 02122u: goto L2122;
    case 02236u: sp = ai_word(m, SPNDX); dp = 2u * sp; goto L2236;
    case 02401u: sp = ai_word(m, SPNDX); dp = 2u * sp; goto L2401;
    case 02360u: goto L2360;
    case 02616u: goto L2616;
    case 02550u: goto L2550;
    case 02552u: goto L2552;
    case 02534u: goto L2534;
    case 03067u: goto L3067;
    case 02420u: goto L2420;
    case 02443u: goto L2443;
    case 02476u: goto L2476;
    case 02632u: goto L2632;
    case 03130u: goto L3130;
    default: break;
    }
/* REDORCS */
    ai_lxch(m, BANKRUPT);
    ai_ca(m, T5PHASE);
    if (ai_bzmf(m)) {                                /* EXTEND; BZMF +2 */
        ai_csf(m, 1u);                               /* ONE */
        ai_ts(m, T5PHASE);
    }
    ai_dcaf(m, rcsloc);                              /* EXTEND; DCA RCSLOC */
    ai_dxch(m, T5LOC);
    goto L2123;                                      /* TCF RCSATT +1 */
L2122:                                               /* RCSATT */
    ai_lxch(m, BANKRUPT);
L2123:
    ai_qxch(m, QRUPT);                               /* EXTEND; QXCH QRUPT */
    ai_caf(m, 040000u);                              /* BIT15 */
    rand(m, io->channel31);                          /* EXTEND; RAND CHAN31 */
    if (ai_bzf(m)) goto L2160;                       /* EXTEND; BZF SETT5 */
    ai_cs(m, RCSFLAGS);
    ai_maskf(m, 020000u);                            /* BIT14 */
    ai_ads(m, RCSFLAGS);
    ai_caf(m, 037777u);                              /* POSMAX */
    ai_ts(m, HOLDFLAG);
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, ERRORX);
    ai_ts(m, ERRORY);
    ai_ts(m, ERRORZ);
    ai_caf(m, 020000u);                              /* BIT14 */
    rand(m, io->channel31);                          /* EXTEND; RAND CHAN31 */
    if (ai_bzf(m)) goto L2160;                       /* EXTEND; BZF SETT5 */
    ai_ts(m, T5PHASE);
    ai_caf(m, 037766u);                              /* OCT37766 */
    ai_ts(m, TIME5);
    ai_tc(m, 02154u);                                /* TCR ZEROJET */
    goto L2632;
L2154:
    goto L2350;                                      /* TCF KMATRIX */
L2160:                                               /* SETT5 */
    ccs = ai_ccs(m, T5PHASE);
    if (ccs == 0u) goto L2544;                       /* TCF FRESHDAP */
    if (ccs == 1u) goto L2661;                       /* TCF T5PHASE2 */
    if (ccs == 2u) goto L2546;                       /* TCF REDAP */
    ai_ts(m, T5PHASE);
    ai_ca(m, TIME5);
    ai_ts(m, T5TIME);
    ai_caf(m, 037776u);                              /* DELTATT2 */
    ai_ts(m, TIME5);
    ai_cs(m, IMODES33);
    ai_maskf(m, 040u);                               /* BIT6 */
    /* CCS A: the mask leaves +0 or BIT6. */
    if (ai_ccs(m, 0u) == 0u) goto L2210;             /* TCF RATEFILT */
    ai_cs(m, RCSFLAGS);                              /* FREECHK */
    ai_maskf(m, 020000u);                            /* BIT14 */
    ai_ads(m, RCSFLAGS);
    ai_caf(m, 020000u);                              /* BIT14 */
    ai_ts(m, HOLDFLAG);
    rand(m, io->channel31);                          /* EXTEND; RAND CHAN31 */
    if (ai_bzf(m)) goto L2417;                       /* EXTEND; BZF KRESUME1 */
    goto L2534;                                      /* TCF REINIT */
L2210:                                               /* RATEFILT */
    ai_ca(m, RCSFLAGS);
    ai_maskf(m, 020000u);                            /* BIT14 */
    if (!ai_bzf(m)) goto L2350;                      /* EXTEND; BZF +2; TCF KMATRIX */
    ai_caf(m, 2u);                                   /* TWO */
L2216:                                               /* DRHOLOOP */
    ai_ts(m, SPNDX);
    sp = ai_word(m, SPNDX);
    ai_double(m);
    ai_ts(m, DPNDX);
    dp = ai_word(m, DPNDX);
    ai_cs(m, DRHO + dp);                             /* INDEX DPNDX */
    ai_mpf(m, gain(m, 03077u));                      /* EXTEND; INDEX ATTKALMN; MP GAIN1 */
    ai_das(m, DRHO + dp);                            /* INDEX DPNDX */
    ai_dcs(m, ADOT + dp);                            /* EXTEND; INDEX DPNDX */
    ai_dxch(m, KMPAC);
    ai_caf(m, 010000u);                              /* QUARTER */
    return leave(m, SMALLMP, 02236u);                /* TC SMALLMP */
L2236:
    ai_dxch(m, KMPAC);
    ai_das(m, DRHO + dp);                            /* INDEX DPNDX */
    if (ai_ccs(m, SPNDX) == 0u) goto L2216;          /* TCF DRHOLOOP */
    ai_ca(m, CDUX);
    ai_xch(m, RHO);
    msu(m, RHO);                                     /* EXTEND; MSU RHO */
    ai_com(m);
    ai_zl(m);
    ai_dxch(m, DELTEMPX);
    ai_ca(m, CDUY);
    ai_xch(m, RHO1);
    msu(m, RHO1);                                    /* EXTEND; MSU RHO1 */
    ai_com(m);
    ai_ts(m, T5TEMP);
    ai_mp(m, AMGB1);
    ai_das(m, DELTEMPX);
    ai_ca(m, AMGB4);
    ai_mp(m, T5TEMP);
    ai_dxch(m, DELTEMPY);
    ai_ca(m, AMGB7);
    ai_mp(m, T5TEMP);
    ai_dxch(m, DELTEMPZ);
    ai_ca(m, CDUZ);
    ai_xch(m, RHO2);
    msu(m, RHO2);                                    /* EXTEND; MSU RHO2 */
    ai_com(m);
    ai_ts(m, T5TEMP);
    ai_mp(m, AMGB5);
    ai_das(m, DELTEMPY);
    ai_ca(m, AMGB8);
    ai_mp(m, T5TEMP);
    ai_das(m, DELTEMPZ);
    ai_caf(m, 2u);                                   /* TWO */
L2311:                                               /* ADOTLOOP */
    ai_ts(m, SPNDX);
    sp = ai_word(m, SPNDX);
    ai_double(m);
    ai_ts(m, DPNDX);
    dp = ai_word(m, DPNDX);
    ai_dca(m, DELTEMPX + dp);                        /* EXTEND; INDEX DPNDX */
    ai_das(m, DRHO + dp);                            /* INDEX DPNDX */
    ai_dca(m, DELTEMPX + dp);                        /* EXTEND; INDEX DPNDX */
    ai_das(m, MERRORX + dp);                         /* INDEX DPNDX */
    ai_ca(m, DRHO + dp);                             /* INDEX DPNDX */
    ai_double(m);
    ai_double(m);
    ai_mpf(m, gain(m, 03115u));                      /* EXTEND; INDEX ATTKALMN; MP GAIN2 */
    ai_das(m, ADOT + dp);                            /* INDEX DPNDX */
    ai_ca(m, KMJ + sp);                              /* INDEX SPNDX */
    ai_mp(m, DFT + sp);                              /* EXTEND; INDEX SPNDX */
    ai_das(m, ADOT + dp);                            /* INDEX DPNDX */
    if (ai_ccs(m, SPNDX) == 0u) goto L2311;          /* TCF ADOTLOOP */
L2350:                                               /* KMATRIX */
    ai_ca(m, ATTSEC);
    ai_maskf(m, 017u);                               /* LOW4 */
    /* CCS A: the mask leaves +0 or a positive count. */
    if (ai_ccs(m, 0u) == 0u) goto L2361;             /* TCF TENTHSEK */
    ai_caf(m, 034000u);                              /* PRIO34 */
    return leave(m, NOVAC, 02356u);                  /* TC NOVAC; 2CADR AMBGUPDT */
L2360:
    ai_caf(m, 011u);                                 /* NINE */
L2361:                                               /* TENTHSEK */
    ai_ts(m, ATTSEC);
    ai_cs(m, HOLDFLAG);
    if (ai_bzmf(m)) goto L2411;                      /* EXTEND; BZMF DACNDLS */
    ai_caf(m, 2u);                                   /* DCDUINCR: TWO */
L2366:                                               /* DELOOP */
    ai_ts(m, SPNDX);
    sp = ai_word(m, SPNDX);
    ai_double(m);
    ai_ts(m, DPNDX);
    dp = ai_word(m, DPNDX);
    ai_dca(m, CDUXD + dp);                           /* EXTEND; INDEX A */
    ai_dxch(m, KMPAC);
    ai_dca(m, DELCDUX + dp);                         /* EXTEND; INDEX DPNDX */
    return leave(m, DPADD, 02401u);                  /* TC DPADD */
L2401:
    ai_dca(m, KMPAC);                                /* EXTEND */
    ai_ts(m, THETADX + sp);                          /* INDEX SPNDX */
    ai_dxch(m, CDUXD + dp);                          /* INDEX DPNDX */
    if (ai_ccs(m, SPNDX) == 0u) goto L2366;          /* TCF DELOOP */
L2411:                                               /* DACNDLS */
    ai_cs(m, RCSFLAGS);
    ai_maskf(m, 010u);                               /* BIT4 */
    if (ai_bzf(m)) goto L3160;                       /* EXTEND; BZF FDAIDSP2 */
    ai_ads(m, RCSFLAGS);                             /* FDAIDSP1 */
    ai_tc(m, 02417u);                                /* TC NEEDLER */
    goto L2420;
L2417:                                               /* KRESUME1 */
    return leave(m, RESUME, 0u);
L2420:                                               /* NEEDLER */
    ai_caf(m, 010u);                                 /* BIT4 */
    rand(m, io->channel12);                          /* EXTEND; RAND CHAN12 */
    if (ai_bzf(m)) goto L2431;                       /* EXTEND; BZF NEEDLER1 */
L2425:
    ai_cs(m, RCSFLAGS);
    ai_maskf(m, 04u);                                /* BIT3 */
    ai_ads(m, RCSFLAGS);
    goto TCQ;
L2431:                                               /* NEEDLER1 */
    ai_ca(m, RCSFLAGS);
    ai_maskf(m, 06u);                                /* SIX */
    if (ai_bzf(m)) goto L2471;                       /* EXTEND; BZF NEEDLES3 */
    ai_maskf(m, 04u);                                /* BIT3 */
    if (ai_bzf(m)) goto L2462;                       /* EXTEND; BZF NEEDLER2 */
    ai_csf(m, 040u);                                 /* BIT6 */
    wand(m, &io->channel12);                         /* EXTEND; WAND CHAN12 */
L2443:                                               /* NEEDLE11 */
    ai_csf(m, 0u);                                   /* ZERO */
    ai_ts(m, AK);
    ai_ts(m, AK1);
    ai_ts(m, AK2);
    ai_ts(m, EDRIVEX);
    ai_ts(m, EDRIVEX + 1u);
    ai_ts(m, EDRIVEX + 2u);
    ai_ts(m, CDUXCMD);
    ai_ts(m, CDUXCMD + 1u);
    ai_ts(m, CDUXCMD + 2u);
    ai_csf(m, 06u);                                  /* SIX */
    ai_mask(m, RCSFLAGS);
    ai_adf(m, 02u);                                  /* BIT2 */
    ai_ts(m, RCSFLAGS);
    goto TCQ;
L2462:                                               /* NEEDLER2 */
    ai_caf(m, 040u);                                 /* BIT6 */
    wor(m, &io->channel12);                          /* EXTEND; WOR CHAN12 */
    ai_csf(m, 06u);                                  /* SIX */
    ai_mask(m, RCSFLAGS);
    ai_ts(m, RCSFLAGS);
    goto TCQ;
L2471:                                               /* NEEDLES3 */
    ai_caf(m, 040u);                                 /* BIT6 */
    rand(m, io->channel12);                          /* EXTEND; RAND CHAN12 */
    if (ai_bzf(m)) goto L2425;                       /* EXTEND; BZF NEEDLER +5 */
L2476:                                               /* NEEDLES */
    ai_caf(m, 2u);                                   /* TWO */
L2477:                                               /* DACLOOP */
    ai_ts(m, SPNDX);
    sp = ai_word(m, SPNDX);
    ai_csf(m, 010000u);                              /* QUARTER */
    ai_mp(m, AK + sp);                               /* EXTEND; INDEX SPNDX */
    ai_ts(m, 1u);                                    /* TS L */
    ccs = ai_ccs(m, 0u);
    if (ccs == 0u) ai_caf(m, 037200u);               /* DACLIMIT */
    else if (ccs == 2u) ai_csf(m, 037200u);          /* DACLIMIT */
    ai_ad(m, 1u);                                    /* AD L */
    if (ai_ts(m, T5TEMP)) {                          /* skips TCF +4: INDEX A; CAF DACLIMIT; TS L */
        ai_caf(m, daclimit[(m->words[0] & 0100000u) ? 0u : 2u]);
        ai_ts(m, 1u);
    }
    ai_cs(m, EDRIVEX + sp);                          /* INDEX SPNDX */
    ai_ad(m, 1u);                                    /* AD L */
    ai_ads(m, CDUXCMD + sp);                         /* INDEX SPNDX */
    ai_lxch(m, EDRIVEX + sp);                        /* INDEX SPNDX */
    if (ai_ccs(m, SPNDX) == 0u) goto L2477;          /* TCF DACLOOP */
    ai_caf(m, 070000u);                              /* 13,14,15 */
    wor(m, &io->channel14);                          /* EXTEND; WOR CHAN14 */
    goto TCQ;
L2534:                                               /* REINIT */
    ai_caf(m, 037754u);                              /* DELAY200 */
    ai_ts(m, TIME5);
    ai_ts(m, T5PHASE);
    return leave(m, RESUME, 0u);
L2544:                                               /* FRESHDAP */
    ai_caf(m, 1u);                                   /* ONE */
    ai_ts(m, HOLDFLAG);
L2546:                                               /* REDAP */
    return leave(m, IBNKCALL, 02547u);               /* TC IBNKCALL; CADR S41.2 */
L2550:
    return leave(m, IBNKCALL, 02551u);               /* TC IBNKCALL; CADR S40.14 */
L2552:
    ai_caf(m, 044u);                                 /* NO.T5VAR */
L2553:                                               /* ZEROT5 */
    ai_ts(m, SPNDX);
    sp = ai_word(m, SPNDX);
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, WBODY + sp);                            /* INDEX SPNDX */
    if (ai_ccs(m, SPNDX) == 0u) goto L2553;          /* TCF ZEROT5 */
    ai_tc(m, 02562u);                                /* TCR ZEROJET */
    goto L2632;
L2562:
    ai_csf(m, 0u);                                   /* ZERO */
    ai_ts(m, CHANTEMP);
    ai_ts(m, CH31TEMP);
    ai_caf(m, 07534u);                               /* =.24 */
    ai_ts(m, SLOPE);
    ai_caf(m, 04u);                                  /* FOUR */
    ai_ts(m, T5TIME);
    ai_caf(m, 013u);                                 /* ELEVEN */
    ai_ts(m, ATTKALMN);
    ai_ca(m, CDUX);
    ai_ts(m, RHO);
    ai_ca(m, CDUY);
    ai_ts(m, RHO1);
    ai_ca(m, CDUZ);
    ai_ts(m, RHO2);
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, T5PHASE);
    ai_cs(m, IMODES33);
    ai_maskf(m, 040u);                               /* BIT6 */
    /* CCS A: the mask leaves +0 or BIT6. */
    if (ai_ccs(m, 0u) == 0u) goto L2612;             /* TCF IMUAOK */
    ai_ts(m, ATTKALMN);
    ai_caf(m, 020004u);                              /* RCSINITB */
    goto L2617;                                      /* TCF RCSSWIT */
L2612:                                               /* IMUAOK */
    ai_caf(m, 034000u);                              /* PRIO34 */
    return leave(m, NOVAC, 02614u);                  /* TC NOVAC; 2CADR AMBGUPDT */
L2616:
    ai_caf(m, 04u);                                  /* RCSINIT */
L2617:                                               /* RCSSWIT */
    ai_ts(m, RCSFLAGS);
    ai_caf(m, 037772u);                              /* T5WAIT60 */
    ai_ts(m, TIME5);
    return leave(m, RESUME, 02623u);                 /* TC RESUME */
L2632:                                               /* ZEROJET */
    ai_caf(m, 013u);                                 /* ELEVEN */
L2633:
    ai_ts(m, SPNDX);
    sp = ai_word(m, SPNDX);
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, RWORD1 + sp);                           /* INDEX SPNDX */
    if (ai_ccs(m, SPNDX) == 0u) goto L2633;          /* TCF ZEROJET +1 */
    ai_caf(m, 04u);                                  /* FOUR */
    ai_ts(m, BLAST1 + 1u);
    ai_caf(m, 013u);                                 /* ELEVEN */
    ai_ts(m, BLAST2 + 1u);
    ai_csf(m, 1u);                                   /* BIT1 */
    ai_mask(m, RCSFLAGS);
    ai_ts(m, RCSFLAGS);
    ai_dcaf(m, t6addr);                              /* EXTEND; DCA T6ADDR */
    ai_dxch(m, T6LOC);
    ai_caf(m, 027u);                                 /* =+14MS */
    ai_ts(m, TIME6);
    ai_caf(m, 040000u);                              /* BIT15 */
    wor(m, &io->channel13);                          /* EXTEND; WOR CHAN13 */
    goto TCQ;
L2661:                                               /* T5PHASE2 */
    ccs = ai_ccs(m, ATTKALMN);
    if (ccs == 0u) goto L3146;                       /* TCF KALUPDT */
    ai_caf(m, 037776u);                              /* DELTATT2 */
    ai_xch(m, TIME5);
    ai_ads(m, T5TIME);
    ai_ca(m, RCSFLAGS);
    ai_maskf(m, 040000u);                            /* BIT15 */
    if (!ai_bzf(m)) ai_csf(m, 1u);                   /* EXTEND; BZF NOHIAUTO; CS ONE */
    ai_ts(m, ATTKALMN);                              /* NOHIAUTO */
    ai_csf(m, 01760u);                               /* OCT01760 */
    ai_mask(m, RCSFLAGS);
    ai_ts(m, RCSFLAGS);
    read(m, io->channel31);                          /* EXTEND; READ CHAN31 */
    ai_ts(m, 1u);                                    /* TS L */
    ai_ca(m, CH31TEMP);
    rxorl(m);                                        /* EXTEND; RXOR LCHAN */
    ai_maskf(m, 077u);                               /* MANROT */
    if (ai_bzmf(m)) goto L2724;                      /* EXTEND; BZMF NOCHANGE */
    ai_lxch(m, 0u);                                  /* LXCH A */
    ai_ts(m, CH31TEMP);
    ai_ca(m, 1u);                                    /* CA L */
    ai_mpf(m, 020u);                                 /* BIT5 */
    ai_ca(m, 1u);                                    /* CA L */
    ai_ads(m, RCSFLAGS);
    ai_cs(m, RCSFLAGS);
    ai_maskf(m, 016000u);                            /* OCT16000 */
    ai_ads(m, RCSFLAGS);
L2724:                                               /* NOCHANGE */
    ai_cs(m, CH31TEMP);
    ai_maskf(m, 077u);                               /* MANROT */
    if (ai_bzmf(m)) return leave(m, AHFNOROT, 0u);   /* EXTEND; BZMF AHFNOROT */
    ai_ts(m, HOLDFLAG);
    ai_tc(m, 02732u);                                /* TC STICKCHK */
    goto L3130;
L2732:
    ai_cs(m, FLAGWRD1);
    ai_maskf(m, 020000u);                            /* BIT14 */
    ai_ads(m, FLAGWRD1);
    ai_caf(m, 020000u);                              /* BIT14 */
    rand(m, io->channel31);                          /* EXTEND; RAND CHAN31 */
    if (ai_bzmf(m)) goto L3051;                      /* EXTEND; BZMF FREEFUNC */
    ai_ca(m, RCSFLAGS);
    ai_maskf(m, 020000u);                            /* BIT14 */
    /* CCS A: the mask leaves +0 or BIT14. */
    if (ai_ccs(m, 0u) == 0u) goto L2534;             /* TCF REINIT */
    ai_csf(m, 05u);                                  /* FIVE */
    ai_ad(m, RATEINDX);
    if (!ai_bzmf(m)) {                               /* EXTEND; BZMF +3 */
        ai_csf(m, 1u);                               /* ONE */
        ai_ts(m, ATTKALMN);
    }
    ai_caf(m, 2u);                                   /* TWO */
L2755:                                               /* SETWBODY */
    ai_ts(m, SPNDX);
    sp = ai_word(m, SPNDX);
    ai_double(m);
    ai_ts(m, DPNDX);
    dp = ai_word(m, DPNDX);
    ai_ca(m, RMANNDX + sp);                          /* INDEX SPNDX */
    if (ai_bzf(m)) goto L3005;                       /* EXTEND; BZF NORATE */
    ai_ad(m, RATEINDX);
    ai_ts(m, 2u);                                    /* TS Q */
    a = (unsigned)agc_to_int(agc_correct(m->words[2]));
    ai_caf(m, mantable[a]);                          /* INDEX Q; CA MANTABLE -1 */
    ai_mpf(m, 0400u);                                /* BIT9 */
    ai_dxch(m, WBODY + dp);                          /* INDEX DPNDX */
    ai_ca(m, RCSFLAGS);
    ai_maskf(m, 016000u);                            /* OCT16000 */
    if (ai_bzf(m)) {                                 /* EXTEND; BZF MERUPDAT */
        ai_csf(m, mantable[a]);                      /* INDEX Q; CS MANTABLE -1 */
        ai_mpf(m, 0100u);                            /* BIT7 */
        ai_das(m, MERRORX + dp);                     /* INDEX DPNDX */
        goto L3023;
    }
L3000:                                               /* ZEROER */
    ai_caf(m, 0u);                                   /* ZERO */
    ai_zl(m);
    ai_dxch(m, MERRORX + dp);                        /* INDEX DPNDX */
    goto L3023;                                      /* TCF SPNDXCHK */
L3005:                                               /* NORATE */
    ai_zl(m);
    ai_dxch(m, WBODY + dp);                          /* INDEX DPNDX */
    ai_ca(m, RCSFLAGS);
    ai_maskf(m, 016000u);                            /* OCT16000 */
    if (!ai_bzf(m)) goto L3000;                      /* EXTEND; BZF SPNDXCHK; TCF ZEROER */
L3023:                                               /* SPNDXCHK */
    ai_ca(m, MERRORX + dp);                          /* INDEX DPNDX */
    ai_ts(m, ERRORX + sp);                           /* INDEX SPNDX */
    if (ai_ccs(m, SPNDX) == 0u) goto L2755;          /* TCF SETWBODY */
    return leave(m, JETS, 0u);                       /* TCF JETS */
L3051:                                               /* FREEFUNC */
    ai_caf(m, freetau[ai_word(m, RMANNDX)]);         /* INDEX RMANNDX; CA FREETAU */
    ai_ts(m, TAU);
    ai_caf(m, freetau[ai_word(m, PMANNDX)]);         /* INDEX PMANNDX; CA FREETAU */
    ai_ts(m, TAU1);
    ai_caf(m, freetau[ai_word(m, YMANNDX)]);         /* INDEX YMANNDX; CA FREETAU */
    ai_ts(m, TAU2);
L3067:                                               /* T6PROGM */
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, ERRORX);
    ai_ts(m, ERRORY);
    ai_ts(m, ERRORZ);
    return leave(m, T6PROG, 0u);                     /* TCF T6PROG */
L3130:                                               /* STICKCHK */
    if (ai_ts(m, T5TEMP)) goto L3132;                /* a caller's raw A can overflow */
    ai_maskf(m, 03u);                                /* THREE */
L3132:
    ai_ts(m, PMANNDX);
    ai_ca(m, T5TEMP);
    ai_mpf(m, 010000u);                              /* QUARTER */
    ai_ts(m, T5TEMP);
    ai_maskf(m, 03u);                                /* THREE */
    ai_ts(m, YMANNDX);
    ai_ca(m, T5TEMP);
    ai_mpf(m, 010000u);                              /* QUARTER */
    ai_ts(m, RMANNDX);
    goto TCQ;
L3146:                                               /* KALUPDT */
    ai_ts(m, ATTKALMN);
    ai_caf(m, 037770u);                              /* DELTATT */
    ai_ad(m, T5TIME);
    if (ai_ts(m, TIME5)) {                           /* skips TCF +3 */
        ai_caf(m, 037776u);                          /* DELTATT2 */
        ai_ts(m, TIME5);
    }
    ai_csf(m, 0u);                                   /* KRESUME2: ZERO */
    ai_ts(m, T5PHASE);
    return leave(m, RESUME, 0u);
L3160:                                               /* FDAIDSP2 */
    ai_csf(m, 010u);                                 /* BIT4 */
    ai_mask(m, RCSFLAGS);
    ai_ts(m, RCSFLAGS);
    ai_cs(m, FLAGWRD0);
    ai_maskf(m, 0400u);                              /* BIT9 */
    if (ai_bzf(m)) goto L3175;                       /* EXTEND; BZF FDAITOTL */
    ai_dcs(m, ERRORX);                               /* EXTEND */
    ai_dxch(m, AK);
    ai_cs(m, ERRORZ);
    ai_ts(m, AK2);
    return leave(m, RESUME, 0u);
L3175:                                               /* FDAITOTL */
    ai_ca(m, FLAGWRD9);
    ai_maskf(m, 040u);                               /* BIT6 */
    if (ai_bzf(m)) {                                 /* EXTEND; BZF WRTN17 */
        ai_dca(m, CPHIX + 1u);                       /* EXTEND */
        ai_dxch(m, WTEMP);
        ai_ca(m, CPHIX);
    } else {
        ai_dca(m, CTHETA);                           /* WRTN22: EXTEND */
        ai_dxch(m, WTEMP);
        ai_ca(m, CPHI);
    }
    msu(m, CDUX);                                    /* GETAKS: EXTEND; MSU CDUX */
    ai_ts(m, AK);
    ai_ca(m, WTEMP);
    msu(m, CDUY);                                    /* EXTEND; MSU CDUY */
    ai_ts(m, T5TEMP);
    ai_mp(m, AMGB1);
    ai_ads(m, AK);
    ai_ca(m, T5TEMP);
    ai_mp(m, AMGB4);
    ai_ts(m, AK1);
    ai_ca(m, T5TEMP);
    ai_mp(m, AMGB7);
    ai_ts(m, AK2);
    ai_ca(m, WTEMP + 1u);
    msu(m, CDUZ);                                    /* EXTEND; MSU CDUZ */
    ai_ts(m, T5TEMP);
    ai_mp(m, AMGB5);
    ai_ads(m, AK1);
    ai_ca(m, T5TEMP);
    ai_mp(m, AMGB8);
    ai_ads(m, AK2);
    return leave(m, RESUME, 0u);
TCQ:                                                 /* TC Q runs the TC held in Q */
    a = m->words[2] & 077777u;
    ai_tc(m, 3u);
    switch (a) {
    case 02154u: goto L2154;
    case 02417u: goto L2417;
    case 02562u: goto L2562;
    case 02732u: goto L2732;
    default: break;
    }
    m->words[5] = (agc_word)a;
    return CM_RCS_DAP_OK;
}
