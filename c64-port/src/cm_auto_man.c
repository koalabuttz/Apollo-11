/* SPDX-License-Identifier: MIT */
#include "cm_auto_man.h"
/* Labels are the original bank-21 addresses. Every CCS takes its four-way
 * skip. TS T5TEMP at 3443 (ADB + 4) overflows when ADB is within 3 of
 * POSMAX: its skip runs JLOOP with SPNDX = +1 (ADB + 4 cannot overflow
 * negatively, so SPNDX never goes negative and ZEROCMDS, the CCS SPNDX
 * negative slots, is translated but unreachable; the INDEX tables still
 * carry the word below each table that a -1 would read). The other TS
 * words store loads, masks, MP or MSU results or loop counters. */
#define CDUX 032u
#define CDUZ 034u
#define T5LOC 01312u
#define HOLDFLAG 01332u
#define RCSFLAGS 01501u
#define T5TEMP 01502u
#define SPNDX 01506u
#define DPNDX 01507u
#define KMPAC 01510u
#define WTEMP 01513u
#define EDOT 01515u
#define AERR 01517u
#define EDOTVEL 01520u
#define AERRVEL 01522u
#define ADBVEL 01523u
#define WBODY 01525u
#define ADOT 01533u
#define TAU 01561u
#define BIAS 01564u
#define ERRORX 01567u
#define THETADX 01572u
#define J_M 01623u
#define CHANTEMP 01633u
#define AMGB1 01640u
#define AMGB4 01641u
#define AMGB5 01642u
#define AMGB7 01643u
#define AMGB8 01644u
#define SLOPE 01654u
#define ADB 01655u
#define RMANNDX 01656u
#define PMANNDX 01657u
#define YMANNDX 01660u
#define SMALLMP 02042u
#define REINIT 02534u
#define T6PROGM 03067u
#define STICKCHK 03130u
#define RESUME 05333u
static const agc_word wlh[2] = { 000022u, 006426u };       /* WLH */
static const agc_word wlmh[2] = { 000012u, 035415u };      /* WLMH */
static const agc_word wl[2] = { 000016u, 022021u };        /* WL */
static const agc_word jetaddr[2] = { 002552u, 036066u };   /* JETADDR: 2CADR JETSLECT */
/* Words the INDEX instructions read: MINTAU (3316-3321), OCT01760 through
 * OCT00300 (3032-3035: OCT01400 and its neighbours) and BIT14 through
 * BIT11 (5002-5005: BIT13 and its neighbours). */
static const agc_word mintau[] = { 000000u, 000027u, 077750u, 000000u };
static const agc_word octs[] = { 001760u, 001400u, 000060u, 000300u };
static const agc_word bits[] = { 020000u, 010000u, 004000u, 002000u };
/* EXTEND MSU on an ordinary erasable word: two's-complement difference of
 * the 15-bit operands, sign-extended, minus one when negative. */
static void msu(agc_image *m, unsigned k) {
    unsigned a = agc_correct(m->words[0]) & 077777u, b = ~(unsigned)ai_word(m, k) & 077777u, d = a + b + 1u;
    if (d & 040000u) d = (d | 0100000u) - 1u;
    m->words[0] = agc_extend((agc_word)(d & 077777u));
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static void read(agc_image *m, agc_word channel) {
    m->words[0] = agc_extend((agc_word)(channel & 077777u));
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static enum cm_auto_man_status leave(agc_image *m, agc_word z, agc_word q) {
    if (q) ai_tc(m, q);
    m->words[2] &= 077777u;                          /* canonical frontier Q */
    m->words[5] = z;
    return CM_AUTO_MAN_OK;
}
static unsigned at(unsigned k, int i) { return (unsigned)((int)k + i); }
static uint8_t small(const agc_image *m, unsigned k, unsigned hi) {
    return ai_word(m, k) <= hi;
}
enum cm_auto_man_status cm_auto_man(agc_image *m, const cm_auto_man_io *io) {
    int sp = 0, dp;
    uint8_t ccs;
    if (m->words[4] != 042000u || m->words[3] != 03000u || m->words[6] != 042006u) return CM_AUTO_MAN_OUT_OF_SCOPE;
    switch (m->words[5]) {
    case 03250u: break;
    case 03441u: goto L3441;
    case 03757u: goto L3757;
    case 03304u:
        if (!small(m, RMANNDX, 3u) || !small(m, PMANNDX, 3u) || !small(m, YMANNDX, 3u)) return CM_AUTO_MAN_OUT_OF_SCOPE;
        goto L3304;
    case 03732u: case 03734u:
        if (!small(m, SPNDX, 2u)) return CM_AUTO_MAN_OUT_OF_SCOPE;
        sp = (int)ai_word(m, SPNDX);
        if (m->words[5] == 03732u) goto L3732;
        goto L3734;
    default: return CM_AUTO_MAN_OUT_OF_SCOPE;
    }
/* AHFNOROT */
    read(m, io->channel31);                          /* EXTEND; READ CHAN31 */
    ai_maskf(m, 020000u);                            /* BIT14 */
    if (ai_bzmf(m)) goto L3272;                      /* EXTEND; BZMF FREECONT */
    ai_ca(m, RCSFLAGS);
    ai_maskf(m, 020000u);                            /* BIT14 */
    /* CCS A: the mask leaves +0 or BIT14. */
    if (ai_ccs(m, 0u) == 0u) return leave(m, REINIT, 0u);   /* TCF REINIT */
    read(m, io->channel31);                          /* EXTEND; READ CHAN31 */
    ai_maskf(m, 010000u);                            /* BIT13 */
    if (ai_bzmf(m)) goto L3372;                      /* EXTEND; BZMF HOLDFUNC */
    ai_ca(m, HOLDFLAG);                              /* AUTOCONT */
    if (ai_bzmf(m)) goto L3322;                      /* EXTEND; BZMF ATTHOLD */
    goto L3376;                                      /* TCF GRABANG */
L3272:                                               /* FREECONT */
    ai_caf(m, 1u);                                   /* ONE */
    ai_ts(m, HOLDFLAG);
    read(m, io->channel32);                          /* EXTEND; READ CHAN32 */
    ai_ts(m, 1u);                                    /* TS L */
    ai_com(m);
    ai_maskf(m, 077u);                               /* MANROT */
    ai_mask(m, CHANTEMP);
    ai_lxch(m, CHANTEMP);
    return leave(m, STICKCHK, 03304u);               /* TC STICKCHK */
L3304:
    ai_caf(m, mintau[ai_word(m, RMANNDX)]);          /* INDEX RMANNDX; CA MINTAU */
    ai_ts(m, TAU);
    ai_caf(m, mintau[ai_word(m, PMANNDX)]);          /* INDEX PMANNDX; CA MINTAU */
    ai_ts(m, TAU + 1u);
    ai_caf(m, mintau[ai_word(m, YMANNDX)]);          /* INDEX YMANNDX; CA MINTAU */
    ai_ts(m, TAU + 2u);
    return leave(m, T6PROGM, 0u);                    /* TCF T6PROGM */
L3322:                                               /* ATTHOLD */
    ai_ca(m, CDUX);
    msu(m, THETADX);                                 /* EXTEND; MSU THETADX */
    ai_ts(m, ERRORX);
    ai_ca(m, CDUX + 1u);
    msu(m, THETADX + 1u);                            /* EXTEND; MSU THETADY */
    ai_ts(m, T5TEMP);
    ai_mp(m, AMGB1);
    ai_ads(m, ERRORX);
    ai_ca(m, T5TEMP);
    ai_mp(m, AMGB4);
    ai_ts(m, ERRORX + 1u);
    ai_ca(m, T5TEMP);
    ai_mp(m, AMGB7);
    ai_ts(m, ERRORX + 2u);
    ai_ca(m, CDUZ);
    msu(m, THETADX + 2u);                            /* EXTEND; MSU THETADZ */
    ai_ts(m, T5TEMP);
    ai_mp(m, AMGB5);
    ai_ads(m, ERRORX + 1u);
    ai_ca(m, T5TEMP);
    ai_mp(m, AMGB8);
    ai_ads(m, ERRORX + 2u);
    ai_cs(m, HOLDFLAG);
    if (ai_bzmf(m)) goto L3441;                      /* EXTEND; BZMF JETS */
    ai_ca(m, BIAS);
    ai_ads(m, ERRORX);
    ai_ca(m, BIAS + 1u);
    ai_ads(m, ERRORX + 1u);
    ai_ca(m, BIAS + 2u);
    ai_ads(m, ERRORX + 2u);
    goto L3441;                                      /* TCF JETS */
L3372:                                               /* HOLDFUNC */
    if (ai_ccs(m, HOLDFLAG) == 1u) goto L3322;       /* +0: TCF ATTHOLD; else GRABANG */
L3376:                                               /* GRABANG */
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, WBODY);
    ai_ts(m, WBODY + 1u);
    ai_ts(m, WBODY + 2u);
    ai_ts(m, WBODY + 3u);
    ai_ts(m, WBODY + 4u);
    ai_ts(m, WBODY + 5u);
    ai_ts(m, BIAS);
    ai_ts(m, BIAS + 1u);
    ai_ts(m, BIAS + 2u);
    ai_ca(m, RCSFLAGS);
    ai_maskf(m, 016000u);                            /* OCT16000 */
    if (!ai_bzf(m)) {                                /* EXTEND; BZF ENDDAMP */
        ai_caf(m, 0u);                               /* ZERO */
        ai_ts(m, ERRORX);
        ai_ts(m, ERRORX + 1u);
        ai_ts(m, ERRORX + 2u);
        goto L3441;                                  /* TCF JETS */
    }
    ai_ts(m, HOLDFLAG);                              /* ENDDAMP */
    ai_dca(m, CDUX);                                 /* EXTEND */
    ai_dxch(m, THETADX);
    ai_ca(m, CDUZ);
    ai_ts(m, THETADX + 2u);
    goto L3322;                                      /* TCF ATTHOLD */
L3441:                                               /* JETS */
    ai_ca(m, ADB);
    ai_adf(m, 04u);                                  /* FOUR */
    if (ai_ts(m, T5TEMP)) goto L3445;                /* the skip enters JLOOP with A = +-1 */
    ai_caf(m, 2u);                                   /* TWO */
L3445:                                               /* JLOOP */
    ai_ts(m, SPNDX);
    sp = agc_to_int(ai_word(m, SPNDX));
    ai_double(m);
    ai_ts(m, DPNDX);
    dp = agc_to_int(ai_word(m, DPNDX));
    ai_dca(m, at(ADOT, dp));                         /* EXTEND; INDEX A */
    ai_dxch(m, EDOT);
    ai_ca(m, HOLDFLAG);
    if (!ai_bzf(m)) {                                /* EXTEND; BZF INHOLD */
        ai_dcs(m, at(WBODY, dp));                    /* EXTEND; INDEX DPNDX */
        ai_das(m, EDOT);
    }
    ai_ca(m, at(ERRORX, sp));                        /* INHOLD: INDEX SPNDX */
    ai_ts(m, AERR);
    ccs = ai_ccs(m, EDOT);
    if (ccs == 1u || ccs == 3u) ccs = ai_ccs(m, EDOT + 1u);   /* SIGNCK1 */
    if (ccs <= 1u) {                                 /* POSVEL */
        ai_dca(m, EDOT);                             /* EXTEND */
        ai_dxch(m, EDOTVEL);
        ai_ca(m, T5TEMP);
        ai_ts(m, ADBVEL);
        ai_ca(m, AERR);
        ai_ts(m, AERRVEL);
        ai_tc(m, 03507u);                            /* TC J6. */
    } else {                                         /* NEGVEL */
        ai_dcs(m, EDOT);                             /* EXTEND */
        ai_dxch(m, EDOTVEL);
        ai_cs(m, T5TEMP);
        ai_ts(m, ADBVEL);
        ai_cs(m, AERR);
        ai_ts(m, AERRVEL);
    }
    ai_su(m, ADB);                                   /* J6.: EXTEND */
    ai_adf(m, 0114u);                                /* WLH/SLOP */
    if (ai_bzmf(m)) goto L3551;                      /* EXTEND; BZMF J8 */
    ai_cs(m, T5TEMP);
    ai_ad(m, AERRVEL);
    if (!ai_bzmf(m)) goto L3542;                     /* EXTEND; BZMF +2; TCF J7 */
    ai_dcs(m, EDOTVEL);                              /* EXTEND */
    ai_dv(m, SLOPE);                                 /* EXTEND */
    ai_su(m, AERRVEL);                               /* EXTEND */
    ai_ad(m, ADB);
    if (ai_bzmf(m)) goto L3630;                      /* EXTEND; BZMF J18 */
    goto L3704;                                      /* TCF J23 */
L3542:                                               /* J7 */
    ai_csf(m, 055u);                                 /* WL-H/SLP */
    ai_su(m, T5TEMP);                                /* EXTEND */
    ai_ad(m, AERRVEL);
    if (ai_bzmf(m)) goto L3634;                      /* EXTEND; BZMF J20 */
    goto L3645;                                      /* TCF J21 */
L3551:                                               /* J8 */
    ai_dcsf(m, wlh);                                 /* EXTEND; DCS WLH */
    ai_dxch(m, WTEMP);
    ai_dca(m, EDOTVEL);                              /* EXTEND */
    ai_das(m, WTEMP);
    ccs = ai_ccs(m, WTEMP);
    if (ccs == 1u || ccs == 3u) ccs = ai_ccs(m, WTEMP + 1u) <= 1u ? 0u : 2u;   /* SIGNCK2 */
    if (ccs == 0u) goto L3673;                       /* TCF J22 */
    ai_dca(m, EDOTVEL);                              /* NJ22: EXTEND */
    ai_dv(m, SLOPE);                                 /* EXTEND */
    ai_ad(m, T5TEMP);
    ai_ad(m, AERRVEL);
    if (ai_ccs(m, 0u) != 2u) goto L3704;             /* CCS A: TCF J23 unless negative */
    ai_dcsf(m, wlmh);                                /* EXTEND; DCS WLMH */
    ai_dxch(m, WTEMP);
    ai_dca(m, EDOTVEL);                              /* EXTEND */
    ai_das(m, WTEMP);
    ccs = ai_ccs(m, WTEMP);
    if (ccs == 1u || ccs == 3u) ccs = ai_ccs(m, WTEMP + 1u) <= 1u ? 0u : 2u;   /* SIGNCK3 */
    if (ccs == 0u) goto L3704;                       /* TCF J23 */
    ai_ca(m, AERRVEL);                               /* NJ23 */
    ai_ad(m, T5TEMP);
    ai_adf(m, 055u);                                 /* WL-H/SLP */
    if (ai_ccs(m, 0u) <= 1u) goto L3716;             /* CCS A: TCF J24 */
    goto L3673;                                      /* TCF J22 */
L3630:                                               /* J18 */
    ai_dcs(m, EDOT);                                 /* EXTEND */
    ai_dxch(m, KMPAC);
    goto L3727;                                      /* TCF JTIME */
L3634:                                               /* J20 */
    ai_cs(m, AERR);
    ai_ad(m, ADBVEL);
    ai_mpf(m, 012173u);                              /* SLOPE2 */
    ai_dxch(m, KMPAC);
    ai_dcs(m, EDOT);                                 /* EXTEND */
    ai_das(m, KMPAC);
    goto L3727;                                      /* TCF JTIME */
L3645:                                               /* J21 */
    ccs = ai_ccs(m, EDOT);
    if (ccs == 1u || ccs == 3u) ccs = ai_ccs(m, EDOT + 1u) <= 1u ? 0u : 2u;   /* SIGNCK4 */
    if (ccs == 0u) goto L3664;                       /* TCF JP */
L3655:                                               /* JN */
    ai_dcs(m, EDOT);                                 /* EXTEND */
    ai_dxch(m, KMPAC);
    ai_dcaf(m, wl);                                  /* EXTEND; DCA WL */
    ai_das(m, KMPAC);
    goto L3727;                                      /* TCF JTIME */
L3664:                                               /* JP */
    ai_dcs(m, EDOT);                                 /* EXTEND */
    ai_dxch(m, KMPAC);
    ai_dcsf(m, wl);                                  /* EXTEND; DCS WL */
    ai_das(m, KMPAC);
    goto L3727;                                      /* TCF JTIME */
L3673:                                               /* J22 */
    ccs = ai_ccs(m, EDOT);
    if (ccs == 1u || ccs == 3u) ccs = ai_ccs(m, EDOT + 1u) <= 1u ? 0u : 2u;   /* SIGNCK5 */
    if (ccs == 0u) goto L3655;                       /* TCF JN */
    goto L3664;                                      /* TCF JP */
L3704:                                               /* J23 */
    ai_csf(m, bits[sp + 1]);                         /* INDEX SPNDX; CS BIT13 */
    ai_mask(m, RCSFLAGS);
    ai_ts(m, RCSFLAGS);
    ai_caf(m, octs[sp + 1]);                         /* INDEX SPNDX; CAF OCT01400 */
    ai_mask(m, RCSFLAGS);
    if (ai_bzf(m)) goto L3750;                       /* EXTEND; BZF DOJET +2 */
    goto L3630;                                      /* TCF J18 */
L3716:                                               /* J24 */
    ai_cs(m, AERR);
    ai_su(m, ADBVEL);                                /* EXTEND */
    ai_mpf(m, 012173u);                              /* SLOPE2 */
    ai_dxch(m, KMPAC);
    ai_dcs(m, EDOT);                                 /* EXTEND */
    ai_das(m, KMPAC);
L3727:                                               /* JTIME */
    ai_ca(m, at(J_M, sp));                           /* INDEX SPNDX */
    return leave(m, SMALLMP, 03732u);                /* TC SMALLMP */
L3732:
    ai_caf(m, 02000u);                               /* BIT11 */
    return leave(m, SMALLMP, 03734u);                /* TC SMALLMP */
L3734:
    ccs = ai_ccs(m, KMPAC);
    if (ccs == 0u) ai_caf(m, 037777u);               /* POSMAX */
    else if (ccs == 2u) ai_caf(m, 040000u);          /* NEGMAX */
    else ai_ca(m, KMPAC + 1u);                       /* TAUNORM */
    ai_ts(m, at(TAU, sp));                           /* DOJET: INDEX SPNDX */
L3750:
    ccs = ai_ccs(m, SPNDX);
    if (ccs == 0u) goto L3445;                       /* TCF JLOOP */
    if (ccs == 2u) {                                 /* ZEROCMDS */
        ai_caf(m, 0u);                               /* ZERO */
        ai_ts(m, TAU);
    }
    if (ccs >= 2u) {                                 /* the -0 slot enters at TS TAU with +0 */
        if (ccs == 3u) ai_ts(m, TAU);
        ai_ts(m, TAU + 1u);
        ai_ts(m, TAU + 2u);
    }
L3757:                                               /* T6PROG */
    ai_dcaf(m, jetaddr);                             /* EXTEND; DCA JETADDR */
    ai_dxch(m, T5LOC);
    return leave(m, RESUME, 0u);
}
