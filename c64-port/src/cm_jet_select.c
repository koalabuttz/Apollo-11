/* SPDX-License-Identifier: MIT */
#include "cm_jet_select.h"
/* Labels are the original bank-17 addresses. Every CCS takes its four-way
 * skip; TS TIME5 at 2555 (DELTATT3 + T5TIME) takes its overflow skip. The
 * other TS words store masks, MP results, table words, CCS results, small
 * counts or times, none of which overflows. */
#define QRUPT 012u
#define BANKRUPT 016u
#define TIME5 030u
#define TIME6 031u
#define T5LOC 01312u
#define RWORD1 01451u
#define RWORD2 01452u
#define PWORD1 01453u
#define PWORD2 01454u
#define YWORD1 01455u
#define YWORD2 01456u
#define BLAST 01457u
#define BLAST1 01461u
#define BLAST2 01463u
#define T5PHASE 01465u
#define DAPDATR1 01466u
#define RCSFLAGS 01501u
#define T5TEMP 01502u
#define XNDX1 01513u
#define XNDX2 01514u
#define YNDX 01515u
#define ZNDX 01516u
#define RINDEX 01517u
#define PINDEX 01520u
#define YINDEX 01521u
#define NRJETS 01522u
#define NPJETS 01523u
#define NYJETS 01524u
#define DFT 01547u
#define DFT1 01550u
#define DFT2 01551u
#define TAU 01561u
#define TAU1 01562u
#define TAU2 01563u
#define ATTKALMN 01617u
#define RACFAIL 01626u
#define RBDFAIL 01627u
#define ACORBD 01630u
#define XTRANS 01631u
#define T5TIME 01634u
#define RESUME 05333u
static const agc_word rcs2cadr[2] = { 002122u, 042066u };  /* RCS2CADR: 2CADR RCSATT */
/* The tables the INDEX words read: XLNNDX (2660-2663), PYTABLE (2714-2732),
 * XLN1NDX (2771-2774), RTABLE (3130-3146), YZTABLE (3151-3161), and NJET
 * and DFTMAX with their signed neighbours (3350-3356, 3304-3312). */
static const agc_word xlnndx[] = { 000000u, 000003u, 000006u, 000000u };
static const agc_word pytable[] = {
    000000u, 005125u, 005252u, 000231u, 002421u, 002610u, 000146u, 002504u,
    002442u, 000000u, 002421u, 002442u, 000000u, 002504u, 002610u
};
static const agc_word xln1ndx[] = { 000000u, 000001u, 000002u, 000000u };
static const agc_word rtable[] = {
    011000u, 022125u, 000252u, 011231u, 015421u, 004610u, 011146u, 015504u,
    004442u, 011000u, 015504u, 004610u, 011000u, 015421u, 004442u
};
static const agc_word yztable[] = {
    011000u, 011231u, 011146u, 011000u, 004610u, 015504u, 011000u, 015421u, 004442u
};
static const agc_word njet[] = { 065252u, 057777u, 040000u, 000000u, 037777u, 020000u, 012525u };
static const agc_word dftmax[] = { 077037u, 077277u, 077537u, 000000u, 000240u, 000500u, 000740u };
/* An INDEX value (an erasable word, or A's corrected sign) added to a table's address: the
 * word at that offset, or a fault outside the table. */
static int value(agc_word v) { return agc_to_int(v); }
static int areg(const agc_image *m) { return agc_to_int(agc_correct(m->words[0])); }
static agc_word at(const agc_word *table, unsigned size, int origin, int i, uint8_t *bad) {
    i += origin;
    if (i < 0 || i >= (int)size) { *bad = 1; return 0; }
    return table[i];
}
#define TABLE(t, origin, i) at((t), (unsigned)(sizeof (t) / sizeof (t)[0]), (origin), (i), &bad)
static void rxor(agc_image *m, agc_word channel) {
    m->words[0] = agc_extend((agc_word)((agc_correct(m->words[0]) ^ channel) & 077777u));
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static void rand(agc_image *m, agc_word channel) {
    m->words[0] = agc_extend((agc_word)(agc_correct(m->words[0]) & channel & 077777u));
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static void write(agc_image *m, agc_word *channel) {
    *channel = (agc_word)(agc_correct(m->words[0]) & 077777u);
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static void wor(agc_image *m, agc_word *channel) {
    agc_word v = (agc_word)((agc_correct(m->words[0]) | *channel) & 077777u);
    *channel = v;
    m->words[0] = agc_extend(v);
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static enum cm_jet_select_status leave(agc_image *m) {
    m->words[2] &= 077777u;                          /* canonical frontier Q */
    m->words[5] = RESUME;
    return CM_JET_SELECT_OK;
}
static uint8_t offset(const agc_image *m, unsigned k) {
    agc_word v = ai_word(m, k);
    return v == 0u || v == 4u || v == 013u;
}
enum cm_jet_select_status cm_jet_select(agc_image *m, cm_jet_select_io *io) {
    uint8_t ccs, bad = 0;
    unsigned r;
    if (m->words[4] != 036000u || m->words[3] != 03000u || m->words[6] != 036006u) return CM_JET_SELECT_OUT_OF_SCOPE;
    if (m->words[5] == 03617u) {
        if (!offset(m, BLAST + 1u) || !offset(m, BLAST1 + 1u) || !offset(m, BLAST2 + 1u)) return CM_JET_SELECT_OUT_OF_SCOPE;
        goto L3617;
    }
    if (m->words[5] == 02552u && ai_word(m, XTRANS) != 077777u && value(ai_word(m, XTRANS)) >= -4 && value(ai_word(m, XTRANS)) <= 4)
        goto L2552;
    return CM_JET_SELECT_OUT_OF_SCOPE;
L2552:                                               /* JETSLECT */
    ai_lxch(m, BANKRUPT);
    ai_caf(m, 037772u);                              /* DELTATT3 */
    ai_ad(m, T5TIME);
    if (ai_ts(m, TIME5)) {                           /* skips TCF +3 */
        ai_caf(m, 037776u);                          /* DELATT20 */
        ai_ts(m, TIME5);
    }
    ai_caf(m, 027u);                                 /* =14MS */
    ai_ts(m, TIME6);
    ai_caf(m, 040000u);                              /* NEGMAX */
    wor(m, &io->channel13);                          /* EXTEND; WOR CHAN13 */
    ai_qxch(m, QRUPT);                               /* EXTEND; QXCH QRUPT */
    ai_caf(m, 07700u);                               /* XLNMASK */
    rxor(m, io->channel31);                          /* EXTEND; RXOR CHAN31 */
    ai_maskf(m, 07700u);                             /* XLNMASK */
    if (ai_bzf(m)) {                                 /* EXTEND; BZF NOXLNCMD */
        ai_ts(m, XNDX1);                             /* NOXLNCMD */
        ai_ts(m, XNDX2);
        ai_ts(m, YNDX);
        ai_ts(m, ZNDX);
        goto L2642;
    }
    ai_ts(m, T5TEMP);
    ai_mpf(m, 0400u);                                /* BIT9 */
    ai_maskf(m, 03u);                                /* THREE */
    ai_ts(m, XNDX1);
    ai_ts(m, XNDX2);
    ai_ca(m, T5TEMP);
    ai_mpf(m, 0100u);                                /* BIT7 */
    ai_maskf(m, 03u);                                /* THREE */
    ai_ts(m, YNDX);
    ai_ca(m, T5TEMP);
    ai_mpf(m, 020u);                                 /* BIT5 */
    ai_maskf(m, 03u);                                /* THREE */
    ai_ts(m, ZNDX);
    ai_ca(m, DAPDATR1);
    ai_maskf(m, 020000u);                            /* BIT14 */
    if (ai_bzf(m)) ai_csf(m, 02u);                   /* EXTEND; BZF NOLEM; NOLEM: CS TWO */
    else ai_csf(m, 03u);                             /* THREE */
    ai_ts(m, ATTKALMN);
    ccs = ai_ccs(m, XTRANS);
    if (ccs == 0u) ai_ts(m, XNDX1);
    else if (ccs == 2u) ai_ts(m, XNDX2);
L2642:                                               /* PWORD */
    ccs = ai_ccs(m, TAU1);
    if (ccs == 0u) ai_caf(m, 1u);                    /* ONE */
    else if (ccs == 2u) ai_caf(m, 2u);               /* TWO */
    ai_ts(m, PINDEX);
    ccs = ai_ccs(m, RACFAIL);
    if (ccs == 0u) ai_caf(m, 011u);                  /* AFAILP: NINE */
    else if (ccs == 2u) ai_caf(m, 014u);             /* CFAILP: TWELVE */
    else ai_caf(m, TABLE(xlnndx, 0, value(ai_word(m, XNDX1))));   /* TABPCOM: INDEX XNDX1; CA XLNNDX */
    ai_ad(m, PINDEX);
    ai_caf(m, TABLE(pytable, 0, areg(m)));   /* INDEX A; CA PYTABLE */
    ai_maskf(m, 01417u);                             /* PJETS */
    ai_ts(m, PWORD1);
    ai_mpf(m, 0100u);                                /* BIT7 */
    ai_ts(m, NPJETS);
    ccs = ai_ccs(m, TAU2);                           /* YWORD */
    if (ccs == 0u) ai_caf(m, 1u);                    /* ONE */
    else if (ccs == 2u) ai_caf(m, 2u);               /* TWO */
    ai_ts(m, YINDEX);
    ccs = ai_ccs(m, RBDFAIL);
    if (ccs == 0u) ai_caf(m, 011u);                  /* BFAILY: NINE */
    else if (ccs == 2u) ai_caf(m, 014u);             /* DFAILY: TWELVE */
    else ai_caf(m, TABLE(xlnndx, 0, value(ai_word(m, XNDX2))));   /* TABYCOM: INDEX XNDX2; CA XLNNDX */
    ai_ad(m, YINDEX);
    ai_caf(m, TABLE(pytable, 0, areg(m)));   /* INDEX A; CA PYTABLE */
    ai_maskf(m, 06360u);                             /* YJETS */
    ai_ts(m, YWORD1);
    ai_mpf(m, 020u);                                 /* BIT5 */
    ai_ts(m, NYJETS);
    ccs = ai_ccs(m, TAU);                            /* RWORD */
    if (ccs == 0u) ai_caf(m, 1u);                    /* ONE */
    else if (ccs == 2u) ai_caf(m, 2u);               /* TWO */
    ai_ts(m, RINDEX);
    if (ai_ccs(m, ACORBD) <= 1u) goto L3046;         /* TCF BDROLL */
    ccs = ai_ccs(m, RACFAIL);                        /* ACROLL */
    if (ccs == 0u) ai_caf(m, 011u);                  /* RAFAIL: NINE */
    else if (ccs == 2u) ai_caf(m, 014u);             /* RCFAIL: TWELVE */
    else ai_caf(m, TABLE(xlnndx, 0, value(ai_word(m, YNDX))));   /* RXLNS: INDEX YNDX; CA XLNNDX */
    ai_ad(m, RINDEX);                                /* TABRCOM */
    ai_caf(m, TABLE(rtable, 0, areg(m)));    /* INDEX A; CA RTABLE */
    ai_maskf(m, 03760u);                             /* ACRJETS */
    ai_ts(m, RWORD1);
    ai_ca(m, ZNDX);                                  /* BDZCHECK */
    if (ai_bzmf(m)) goto L3040;                      /* EXTEND; BZMF NOBDZ */
    ccs = ai_ccs(m, RBDFAIL);
    if (ccs == 0u) ai_caf(m, 03u);                   /* THREE */
    else if (ccs == 2u) ai_caf(m, 06u);              /* SIX */
    ai_adf(m, TABLE(xln1ndx, 0, value(ai_word(m, ZNDX))));   /* INDEX ZNDX; AD XLN1NDX */
    ai_caf(m, TABLE(yztable, 0, areg(m)));   /* INDEX A; CA YZTABLE */
    ai_maskf(m, 03417u);                             /* BDZJETS */
    ai_ad(m, RWORD1);
    ai_ts(m, T5TEMP);
    ai_mpf(m, 0100u);                                /* BIT7 */
    ai_adf(m, 077773u);                              /* =-4 */
    ai_ts(m, NRJETS);
    if (ai_bzf(m)) {                                 /* EXTEND; BZF TAUCHECK */
        ccs = ai_ccs(m, TAU);                        /* TAUCHECK */
        if (ccs == 0u || ccs == 2u) goto L3040;      /* TCF NOBDZ */
    }
    ai_ca(m, T5TEMP);                                /* ACRBDZ */
    ai_ts(m, RWORD1);
    goto L3274;                                      /* TCF ROLLTIME */
L3040:                                               /* NOBDZ */
    ai_ca(m, RWORD1);
    ai_mpf(m, 0100u);                                /* BIT7 */
    ai_adf(m, 077775u);                              /* =-2 */
    ai_ts(m, NRJETS);
    goto L3274;                                      /* TCF ROLLTIME */
L3046:                                               /* BDROLL */
    ccs = ai_ccs(m, RBDFAIL);
    if (ccs == 0u) ai_caf(m, 011u);                  /* RBFAIL: NINE */
    else if (ccs == 2u) ai_caf(m, 014u);             /* RDFAIL: TWELVE */
    else ai_caf(m, TABLE(xlnndx, 0, value(ai_word(m, ZNDX))));   /* RZXLNS: INDEX ZNDX; CA XLNNDX */
    ai_ad(m, RINDEX);                                /* TABRZCMD */
    ai_caf(m, TABLE(rtable, 0, areg(m)));    /* INDEX A; CA RTABLE */
    ai_maskf(m, 034017u);                            /* BDRJETS */
    ai_ts(m, RWORD1);
    ai_ca(m, YNDX);                                  /* ACYCHECK */
    if (ai_bzf(m)) goto L3122;                       /* EXTEND; BZF NOACY */
    ccs = ai_ccs(m, RACFAIL);
    if (ccs == 0u) ai_caf(m, 03u);                   /* THREE */
    else if (ccs == 2u) ai_caf(m, 06u);              /* SIX */
    ai_adf(m, TABLE(xln1ndx, 0, value(ai_word(m, YNDX))));   /* INDEX YNDX; AD XLN1NDX */
    ai_caf(m, TABLE(yztable, 0, areg(m)));   /* INDEX A; CA YZTABLE */
    ai_maskf(m, 034360u);                            /* ACYJETS */
    ai_ad(m, RWORD1);
    ai_ts(m, T5TEMP);
    ai_mpf(m, 010u);                                 /* BIT4 */
    ai_adf(m, 077773u);                              /* =-4 */
    ai_ts(m, NRJETS);
    if (ai_bzf(m)) {                                 /* EXTEND; BZF TAUCHCK */
        ccs = ai_ccs(m, TAU);                        /* TAUCHCK */
        if (ccs == 0u || ccs == 2u) goto L3122;      /* TCF NOACY */
    }
    ai_ca(m, T5TEMP);                                /* BDRACZ */
    ai_ts(m, RWORD1);
    goto L3274;                                      /* TCF ROLLTIME */
L3122:                                               /* NOACY */
    ai_ca(m, RWORD1);
    ai_mpf(m, 010u);                                 /* BIT4 */
    ai_adf(m, 077775u);                              /* =-2 */
    ai_ts(m, NRJETS);
L3274:                                               /* ROLLTIME */
    ccs = ai_ccs(m, TAU);
    if (ccs == 1u || ccs == 3u) {
        ai_caf(m, TABLE(dftmax, 3, value(ai_word(m, NRJETS))));   /* INDEX NRJETS; CA DFTMAX */
        ai_ts(m, DFT);
        ai_ca(m, RWORD1);                            /* RBYPASS */
        ai_ts(m, RWORD2);
        ai_caf(m, 0u);                               /* ZERO */
        ai_ts(m, BLAST);
        goto L3164;                                  /* TCF PITCHTIM */
    }
    ai_ca(m, TAU);                                   /* RBLAST */
    ai_mpf(m, TABLE(njet, 3, value(ai_word(m, NRJETS))));   /* EXTEND; INDEX NRJETS; MP NJET */
    ai_ts(m, BLAST);
    ai_adf(m, 077537u);                              /* =-.1SEC */
    if (!ai_bzmf(m)) {                               /* EXTEND; BZMF AD14MSR */
        ai_caf(m, TABLE(dftmax, 3, value(ai_word(m, NRJETS))));   /* INDEX NRJETS; CA DFTMAX */
        ai_ts(m, DFT);
        ai_com(m);
        ai_ads(m, TAU);
        ai_caf(m, 0240u);                            /* =+.1SEC */
        ai_ts(m, BLAST);
    } else {
        ai_cs(m, BLAST);                             /* AD14MSR */
        ai_adf(m, 027u);                             /* =14MS */
        if (!ai_bzmf(m)) {                           /* EXTEND; BZMF RBLASTOK */
            ai_caf(m, 027u);                         /* =14MS */
            ai_ts(m, BLAST);
        }
        ai_ca(m, BLAST);                             /* RBLASTOK */
        ai_mp(m, NRJETS);                            /* EXTEND */
        ai_lxch(m, DFT);
        ai_ts(m, TAU);
    }
    /* ASMBLWR: second roll word from the other quad pair. */
    if (ai_ccs(m, YNDX) == 0u) {                     /* TCF ACBD2Y */
        if (ai_ccs(m, ACORBD) <= 1u) {               /* AC2Y */
            ccs = ai_ccs(m, RACFAIL);
            if (ccs == 0u) ai_caf(m, 03u);           /* THREE */
            else if (ccs == 2u) ai_caf(m, 06u);      /* SIX */
            ai_adf(m, TABLE(xln1ndx, 0, value(ai_word(m, YNDX))));   /* INDEX YNDX; AD XLN1NDX */
            ai_caf(m, TABLE(yztable, 0, areg(m)));   /* INDEX A; CA YZTABLE */
            ai_maskf(m, 034360u);                    /* ACYJETS */
            ai_ts(m, RWORD2);
            ai_mpf(m, 010u);                         /* BIT4 */
            ai_adf(m, 077775u);                      /* =-2 */
            ai_ts(m, NRJETS);
            ai_cs(m, BLAST);
            ai_adf(m, 0240u);                        /* =+.1SEC */
            ai_mp(m, NRJETS);                        /* EXTEND */
            ai_ca(m, 1u);                            /* CA L */
            ai_ads(m, DFT);
            goto L3403;                              /* TCF NO2Y +2 */
        }
        ccs = ai_ccs(m, RACFAIL);
        if (ccs == 1u || ccs == 3u) {
            ai_caf(m, TABLE(xlnndx, 0, value(ai_word(m, YNDX))));   /* INDEX YNDX; CA XLNNDX */
            ai_caf(m, TABLE(rtable, 0, areg(m)));   /* INDEX A; CA RTABLE */
            ai_maskf(m, 03760u);                     /* ACRJETS */
            goto L3402;                              /* TCF NO2Y +1 */
        }
    }
    ai_caf(m, 0u);                                   /* NO2Y: ZERO */
L3402:
    ai_ts(m, RWORD2);
L3403:
    if (ai_ccs(m, ZNDX) == 0u) {                     /* TCF ACBD2Z */
        if (ai_ccs(m, ACORBD) >= 2u) {
            ccs = ai_ccs(m, RBDFAIL);
            if (ccs == 0u) ai_caf(m, 03u);           /* THREE */
            else if (ccs == 2u) ai_caf(m, 06u);      /* SIX */
            ai_adf(m, TABLE(xln1ndx, 0, value(ai_word(m, ZNDX))));   /* INDEX ZNDX; AD XLN1NDX */
            ai_caf(m, TABLE(yztable, 0, areg(m)));   /* INDEX A; CA YZTABLE */
            ai_maskf(m, 03417u);                     /* BDZJETS */
            ai_ads(m, RWORD2);
            ai_mpf(m, 0100u);                        /* BIT7 */
            ai_adf(m, 077775u);                      /* =-2 */
            ai_ts(m, NRJETS);
            ai_cs(m, BLAST);
            ai_adf(m, 0240u);                        /* =+.1SEC */
            ai_mp(m, NRJETS);                        /* EXTEND */
            ai_ca(m, 1u);                            /* CA L */
            ai_ads(m, DFT);
            goto L3164;                              /* TCF PITCHTIM */
        }
        ccs = ai_ccs(m, RBDFAIL);                    /* BDF2Z */
        if (ccs == 1u || ccs == 3u) {
            ai_caf(m, TABLE(xlnndx, 0, value(ai_word(m, ZNDX))));   /* INDEX ZNDX; CA XLNNDX */
            ai_caf(m, TABLE(rtable, 0, areg(m)));   /* INDEX A; CA RTABLE */
            ai_maskf(m, 034017u);                    /* BDRJETS */
            goto L3406;                              /* TCF NO2Z +1 */
        }
    }
    ai_caf(m, 0u);                                   /* NO2Z: ZERO */
L3406:
    ai_ads(m, RWORD2);
L3164:                                               /* PITCHTIM */
    ccs = ai_ccs(m, TAU1);
    if (ccs == 1u || ccs == 3u) {
        ai_ts(m, DFT1);
        ai_ca(m, PWORD1);                            /* PBYPASS */
        ai_ts(m, PWORD2);
        ai_caf(m, 0u);                               /* ZERO */
        ai_ts(m, BLAST1);
        goto L3230;                                  /* TCF YAWTIME */
    }
    if (ccs == 2u) {                                 /* PTAUNEG */
        ai_cs(m, NPJETS);
        ai_ts(m, NPJETS);
    }
    ai_ca(m, TAU1);                                  /* PTAUPOS */
    ai_mpf(m, TABLE(njet, 3, value(ai_word(m, NPJETS))));   /* EXTEND; INDEX NPJETS; MP NJET */
    ai_ts(m, BLAST1);
    ai_adf(m, 077537u);                              /* =-.1SEC */
    if (!ai_bzmf(m)) {                               /* EXTEND; BZMF AD14MSP */
        ai_caf(m, TABLE(dftmax, 3, value(ai_word(m, NPJETS))));   /* INDEX NPJETS; CA DFTMAX */
        ai_ts(m, DFT1);
        ai_com(m);
        ai_ads(m, TAU1);
        ai_caf(m, 0240u);                            /* =+.1SEC */
        ai_ts(m, BLAST1);
    } else {
        ai_cs(m, BLAST1);                            /* AD14MSP */
        ai_adf(m, 027u);                             /* =14MS */
        if (!ai_bzmf(m)) {                           /* EXTEND; BZMF PBLASTOK */
            ai_caf(m, 027u);                         /* =14MS */
            ai_ts(m, BLAST1);
        }
        ai_ca(m, BLAST1);                            /* PBLASTOK */
        ai_mp(m, NPJETS);                            /* EXTEND */
        ai_lxch(m, DFT1);
        ai_ts(m, TAU1);
    }
    ccs = ai_ccs(m, RACFAIL);                        /* ASMBLWP */
    if (ccs == 1u || ccs == 3u) {
        ai_caf(m, TABLE(xlnndx, 0, value(ai_word(m, XNDX1))));   /* INDEX XNDX1; CA XLNNDX */
        ai_caf(m, TABLE(pytable, 0, areg(m)));   /* INDEX A; FPX2: CA PYTABLE */
    } else ai_caf(m, pytable[0]);                    /* FPX2: CA PYTABLE */
    ai_maskf(m, 01417u);                             /* PJETS */
    ai_ts(m, PWORD2);
L3230:                                               /* YAWTIME */
    ccs = ai_ccs(m, TAU2);
    if (ccs == 1u || ccs == 3u) {
        ai_ts(m, DFT2);
        ai_ca(m, YWORD1);                            /* YBYPASS */
        ai_ts(m, YWORD2);
        ai_caf(m, 0u);                               /* ZERO */
        ai_ts(m, BLAST2);
        goto L3542;                                  /* T6SETUP */
    }
    if (ccs == 2u) {                                 /* YTAUNEG */
        ai_cs(m, NYJETS);
        ai_ts(m, NYJETS);
    }
    ai_ca(m, TAU2);                                  /* YTAUPOS */
    ai_mpf(m, TABLE(njet, 3, value(ai_word(m, NYJETS))));   /* EXTEND; INDEX NYJETS; MP NJET */
    ai_ts(m, BLAST2);
    ai_adf(m, 077537u);                              /* =-.1SEC */
    if (!ai_bzmf(m)) {                               /* EXTEND; BZMF AD14MSY */
        ai_caf(m, TABLE(dftmax, 3, value(ai_word(m, NYJETS))));   /* INDEX NYJETS; CA DFTMAX */
        ai_ts(m, DFT2);
        ai_com(m);
        ai_ads(m, TAU2);
        ai_caf(m, 0240u);                            /* =+.1SEC */
        ai_ts(m, BLAST2);
    } else {
        ai_cs(m, BLAST2);                            /* AD14MSY */
        ai_adf(m, 027u);                             /* =14MS */
        if (!ai_bzmf(m)) {                           /* EXTEND; BZMF YBLASTOK */
            ai_caf(m, 027u);                         /* =14MS */
            ai_ts(m, BLAST2);
        }
        ai_ca(m, BLAST2);                            /* YBLASTOK */
        ai_mp(m, NYJETS);                            /* EXTEND */
        ai_lxch(m, DFT2);
        ai_ts(m, TAU2);
    }
    ccs = ai_ccs(m, RBDFAIL);                        /* ASMBLWY */
    if (ccs == 1u || ccs == 3u) {
        ai_caf(m, TABLE(xlnndx, 0, value(ai_word(m, XNDX2))));   /* INDEX XNDX2; CA XLNNDX */
        ai_caf(m, TABLE(pytable, 0, areg(m)));   /* INDEX A; FYX2: CA PYTABLE */
    } else ai_caf(m, pytable[0]);                    /* FYX2: CA PYTABLE */
    ai_maskf(m, 06360u);                             /* YJETS */
    ai_ts(m, YWORD2);
L3542:                                               /* T6SETUP */
    if (bad) return CM_JET_SELECT_FAULT;
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, BLAST + 1u);
    ai_caf(m, 04u);                                  /* FOUR */
    ai_ts(m, BLAST1 + 1u);
    ai_caf(m, 013u);                                 /* ELEVEN */
    ai_ts(m, BLAST2 + 1u);
    ai_cs(m, BLAST);
    ai_ad(m, BLAST1);
    if (ai_bzmf(m)) {                                /* EXTEND; BZMF DXCHT12 */
        ai_dxch(m, BLAST);                           /* DXCHT12 */
        ai_dxch(m, BLAST1);
        ai_dxch(m, BLAST);
    }
    ai_cs(m, BLAST1);                                /* CHECKT23 */
    ai_ad(m, BLAST2);
    if (ai_bzmf(m)) {                                /* EXTEND; BZMF DXCHT23 */
        ai_dxch(m, BLAST1);                          /* DXCHT23 */
        ai_dxch(m, BLAST2);
        ai_dxch(m, BLAST1);
        ai_cs(m, BLAST);
        ai_ad(m, BLAST1);
        if (ai_bzmf(m)) {                            /* EXTEND; BZMF +2 */
            ai_dxch(m, BLAST);
            ai_dxch(m, BLAST1);
            ai_dxch(m, BLAST);
        }
    }
    ai_cs(m, BLAST1);                                /* CALCDT6 */
    ai_ads(m, BLAST2);
    ai_cs(m, BLAST);
    ai_ads(m, BLAST1);
    ai_dcaf(m, rcs2cadr);                            /* EXTEND; DCA RCS2CADR */
    ai_dxch(m, T5LOC);
    ai_csf(m, 1u);                                   /* ENDJETS: BIT1 */
    ai_mask(m, RCSFLAGS);
    ai_ts(m, RCSFLAGS);
    ai_csf(m, 0u);                                   /* ZERO */
    ai_ts(m, T5PHASE);
    return leave(m);
L3617:                                               /* T6START */
    ai_lxch(m, BANKRUPT);
    ai_qxch(m, QRUPT);                               /* EXTEND; QXCH QRUPT */
    ccs = ai_ccs(m, TIME6);
    if (ccs == 0u || ccs == 2u) return leave(m);     /* TCF RESUME */
    ai_cs(m, RCSFLAGS);
    ai_maskf(m, 1u);                                 /* BIT1 */
    if (!ai_bzf(m)) {                                /* EXTEND; BZF T6RUPTOR */
        ai_ads(m, RCSFLAGS);
        ai_ca(m, RWORD1);
        write(m, &io->channel6);                     /* EXTEND; WRITE CHAN6 */
        ai_ca(m, PWORD1);
        ai_ad(m, YWORD1);
        write(m, &io->channel5);                     /* EXTEND; WRITE CHAN5 */
    }
    /* T6RUPTOR, T6L1, T6L2: the first blast still pending (+) restarts T6
     * with it; each one that has run out (+0, -0) has its replacement word
     * written by REPLACER, REPLACEP or REPLACEY (INDEX BLASTn+1; TC REPLACER)
     * and is marked done (-1); a done one (-) is passed over. */
    for (r = 0; r < 3u; ++r) {
        unsigned blast = BLAST + 2u * r;
        ccs = ai_ccs(m, blast);
        if (ccs == 0u) {                             /* ZBLAST, ZBLAST1, ZBLAST2 */
            ai_caf(m, 0u);                           /* ZERO */
            ai_xch(m, blast);
            ai_ts(m, TIME6);                         /* ENABT6 */
            ai_caf(m, 040000u);                      /* NEGMAX */
            wor(m, &io->channel13);                  /* EXTEND; WOR CHAN13 */
            return leave(m);
        }
        if (ccs == 2u) continue;
        ai_tc(m, (agc_word)(03663u + 5u * r));                   /* INDEX BLASTn +1; TC REPLACER */
        switch (ai_word(m, blast + 1u)) {
        case 0u:                                     /* REPLACER */
            ai_ca(m, RWORD2);
            write(m, &io->channel6);                 /* EXTEND; WRITE CHAN6 */
            break;
        case 4u:                                     /* REPLACEP */
            ai_caf(m, 06360u);                       /* YJETS */
            rand(m, io->channel5);                   /* EXTEND; RAND CHAN5 */
            ai_ad(m, PWORD2);
            write(m, &io->channel5);                 /* EXTEND; WRITE CHAN5 */
            break;
        default:                                     /* REPLACEY */
            ai_caf(m, 01417u);                       /* PJETS */
            rand(m, io->channel5);                   /* EXTEND; RAND CHAN5 */
            ai_ad(m, YWORD2);
            write(m, &io->channel5);                 /* EXTEND; WRITE CHAN5 */
            break;
        }
        ai_tc(m, 3u);                                /* TC Q */
        ai_csf(m, 1u);                               /* ONE */
        ai_ts(m, blast);
    }
    return leave(m);                                 /* TCF RESUME */
}
