/* SPDX-License-Identifier: MIT */
#include "cm_tvc_roll.h"
/* Labels are the original bank-16 addresses. Every CCS takes its four-way
 * skip and every TS that can overflow its skip; the TS words with no skip
 * store an MP high word, a load, a constant, a CCS result or ±1. */
#define BANKRUPT 016u
#define TIME6 031u
#define T6LOC 01310u
#define OGARATE 01533u
#define ROLLFIRE 01571u
#define ROLLWORD 01572u
#define TEMREG 01573u
#define CONACC_1 01640u
#define OGANOW 01662u
#define OGAPAST 01663u
#define OGA 01664u
#define DELOGART 01665u
#define SGNRT 01666u
#define DELOGA 01667u
#define I 01670u
#define IOGARATE 01671u
#define TASKOVER 05324u
#define NOQRSM 05335u
static const agc_word norol1t6[2] = { 003676u, 034066u };   /* NOROL1T6: 2CADR NOROLL1 */
/* EXTEND MSU on an ordinary erasable word: two's-complement difference of
 * the 15-bit operands, sign-extended, minus one when negative. */
static void msu(agc_image *m, unsigned k) {
    unsigned a = agc_correct(m->words[0]) & 077777u, b = ~(unsigned)ai_word(m, k) & 077777u, d = a + b + 1u;
    if (d & 040000u) d = (d | 0100000u) - 1u;
    m->words[0] = agc_extend((agc_word)(d & 077777u));
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
/* EXTEND WRITE: the overflow-corrected A to the channel; A unchanged. */
static void write(agc_image *m, agc_word *channel) {
    *channel = (agc_word)(agc_correct(m->words[0]) & 077777u);
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
/* EXTEND WOR: overflow-corrected A OR the channel, to both. */
static void wor(agc_image *m, agc_word *channel) {
    agc_word v = (agc_word)((agc_correct(m->words[0]) | *channel) & 077777u);
    *channel = v;
    m->words[0] = agc_extend(v);
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
/* INDEX I on a word with a zero opcode field: I is PRIO30 (CA) or BIT15
 * (CS), as the file sets it at 3343. */
static void indexed(agc_image *m, unsigned k) {
    if (ai_word(m, I) & 040000u) ai_cs(m, k);
    else ai_ca(m, k);
}
static void indexedf(agc_image *m, agc_word k) {
    if (ai_word(m, I) & 040000u) ai_csf(m, k);
    else ai_caf(m, k);
}
static enum cm_tvc_roll_status leave(agc_image *m, agc_word z) {
    m->words[2] &= 077777u;                          /* canonical frontier Q */
    m->words[5] = z;
    return CM_TVC_ROLL_OK;
}
enum cm_tvc_roll_status cm_tvc_roll(agc_image *m, cm_tvc_roll_io *io) {
    uint8_t ccs;
    if (m->words[4] != 034000u || m->words[3] != 03000u || m->words[6] != 034006u) return CM_TVC_ROLL_OUT_OF_SCOPE;
    if (m->words[5] == 03676u) goto L3676;
    if (m->words[5] != 03272u) return CM_TVC_ROLL_OUT_OF_SCOPE;
/* ROLLDAP */
    ai_ca(m, OGANOW);
    msu(m, OGAPAST);
    ai_mpf(m, 020u);                                 /* BIT5 */
    ai_lxch(m, 0u);
    ai_ts(m, OGARATE);
    ai_ca(m, ROLLFIRE);                              /* DURATION */
    if (!ai_bzf(m)) goto L3313;                      /* EXTEND; BZF +2; TCF ROLLOGIC */
    ai_ca(m, TEMREG);
    if (ai_bzf(m)) goto L3313;                       /* EXTEND; BZF ROLLOGIC */
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, TEMREG);
    return leave(m, TASKOVER);                       /* WAIT1/2 */
L3313:                                               /* ROLLOGIC */
    ai_cs(m, OGARATE);
    ai_mp(m, CONACC_1);
    ai_mp(m, OGARATE);
    ai_adf(m, 0344u);                                /* DB */
    if (ai_ts(m, TEMREG)) goto L3323;
    ai_ca(m, OGARATE);
L3323:
    /* EXTEND; BZMF +3: CS BIT1, else CA BIT1; TCF +2 */
    if (ai_bzmf(m)) ai_csf(m, 1u);                   /* BIT1 */
    else ai_caf(m, 1u);                              /* BIT1 */
    ai_ts(m, SGNRT);
    ai_mp(m, TEMREG);
    ai_cs(m, 1u);                                    /* CS L */
    ai_ad(m, OGA);
    if (ai_ts(m, DELOGA)) {                          /* DELOGAC: the skip runs BZMF +3 as AD 3342 */
        ai_adf(m, 035001u);
        goto L3340;
    }
    if (ai_bzmf(m)) {                                /* EXTEND; BZMF +3 */
        ai_caf(m, 040000u);                          /* BIT15 */
        goto L3343;
    }
L3340:
    ai_caf(m, 030000u);                              /* PRIO30 */
L3343:
    ai_ts(m, I);
    indexed(m, SGNRT);                               /* INDEX I; 0 SGNRT */
    ai_com(m);
    if (ai_bzmf(m)) goto L3421;                      /* REG1TST */
    indexed(m, OGARATE);                             /* INDEX I; 0 OGARATE */
    ai_ts(m, IOGARATE);
    ai_adf(m, 07071u);                               /* MAXLIM */
    if (ai_bzmf(m)) goto L3500;                      /* REG3TST */
    ai_ca(m, OGARATE);
    ai_mpf(m, 020000u);                              /* BIT14 */
    ai_ts(m, TEMREG);
    ai_ca(m, OGA);
    ai_mpf(m, 06315u);                               /* -SLOPE */
    ai_ddoubl(m);
    ai_ddoubl(m);
    ai_ddoubl(m);
    ai_ad(m, TEMREG);
    indexed(m, 0u);                                  /* INDEX I; 0 A */
    ai_com(m);
    ai_adf(m, 0510u);                                /* INTERCEP */
    ai_com(m);
    if (ai_bzmf(m)) goto L3611;                      /* REG2TST */
    ai_ca(m, IOGARATE);
    ai_adf(m, 01330u);                               /* MINLIM */
    if (ai_bzmf(m)) goto L3611;                      /* REG4TST */
    indexed(m, OGA);                                 /* INDEX I; 0 OGA */
    ai_com(m);
    ai_adf(m, 0344u);                                /* DB */
    ai_com(m);
    if (ai_bzmf(m)) goto L3611;                      /* REG5TST */
    ai_ca(m, OGA);                                   /* REGION6 */
    ai_ts(m, DELOGA);
    ai_ca(m, OGARATE);
    ai_ts(m, DELOGART);
    goto L3430;
L3421:                                               /* ROLLON */
    indexedf(m, 0344u);                              /* INDEX I; 0 DB */
    ai_ads(m, DELOGA);
    ai_csf(m, 0111u);                                /* LMCRATE */
    indexed(m, 0u);                                  /* INDEX I; 0 A */
    ai_ts(m, DELOGART);
L3430:                                               /* ONROLL */
    ai_mp(m, CONACC_1);
    ai_double(m);
    ai_mpf(m, 06315u);                               /* -SLOPE */
    ai_ts(m, TEMREG);
    ai_mp(m, DELOGART);
    ai_ts(m, DELOGART);
    ai_csf(m, 02000u);                               /* BIT11 */
    indexed(m, 0u);                                  /* INDEX I; 0 A */
    ai_ads(m, TEMREG);                               /* RATEDEN */
    indexed(m, DELOGA);                              /* INDEX I; 0 DELOGA */
    ai_com(m);
    ai_adf(m, 0344u);                                /* DB */
    ai_com(m);
    ai_mpf(m, 06315u);                               /* -SLOPE */
    ai_ads(m, DELOGART);
    ai_caf(m, 0111u);                                /* LMCRATE */
    ai_mpf(m, 02000u);                               /* BIT11 */
    ai_ad(m, DELOGART);                              /* RATENUM */
    ai_xch(m, 1u);
    ai_caf(m, 0u);                                   /* ZERO */
    ai_dv(m, TEMREG);
    if (ai_bzf(m)) goto L3474;                       /* EXTEND; BZF DVOK */
    ccs = ai_ccs(m, 0u);                             /* MINLIMAP */
    if (ccs == 0u) ai_caf(m, 037777u);               /* POSMAX */
    else if (ccs == 2u) ai_csf(m, 037777u);          /* POSMAX */
    goto L3503;
L3474:                                               /* DVOK */
    ai_lxch(m, 0u);
    ai_dv(m, TEMREG);
    goto L3503;
L3500:                                               /* RATELIM */
    ai_csf(m, 07071u);                               /* MAXLIM */
    indexed(m, 0u);                                  /* INDEX I; 0 A */
L3503:                                               /* ROLLSET */
    if (ai_ts(m, TEMREG)) ai_ad(m, OGARATE);         /* the skip runs SU OGARATE as AD */
    else ai_su(m, OGARATE);
    if (ai_ts(m, TEMREG))                            /* skips TCF +3: INDEX A; CS LIMITS */
        ai_csf(m, (m->words[0] & 0100000u) ? 037777u : 040000u);
    ai_mpf(m, 031000u);                              /* T6SCALE */
    ai_mp(m, CONACC_1);
    ai_ddoubl(m);
    ai_ddoubl(m);
    if (ai_ts(m, TEMREG))                            /* skips TCF +3: INDEX A; CS LIMITS */
        ai_csf(m, (m->words[0] & 0100000u) ? 037777u : 040000u);
    ai_ts(m, TEMREG);
    if (ai_bzf(m)) goto L3611;                       /* EXTEND; BZF NOROLL */
    ai_ca(m, TEMREG);
    ai_mp(m, ROLLFIRE);
    ccs = ai_ccs(m, 0u);
    if (ccs == 2u) goto L3611;                       /* TCF NOROLL */
    if (ccs != 0u) goto L3542;                       /* TCF NEWROLL */
    ai_caf(m, 0u);                                   /* MOREROLL: ZERO */
    ai_ts(m, I);
    goto L3553;
L3542:                                               /* NEWROLL */
    ccs = ai_ccs(m, TEMREG);
    if (ccs == 0u || ccs == 2u) ai_adf(m, 1u);       /* ONE */
    ai_com(m);
    ai_adf(m, 030u);                                 /* TMINFIRE */
    ai_com(m);
    if (ai_bzmf(m)) goto L3611;                      /* MINTST */
L3553:                                               /* MAXTFIRE */
    ai_ca(m, TEMREG);
    ai_mpf(m, 4u);                                   /* 1/TMXFIR */
    if (ai_bzf(m)) goto L3565;                       /* MAXTST */
    ccs = ai_ccs(m, 0u);
    if (ccs == 0u) ai_caf(m, 07640u);                /* TMAXFIRE */
    else if (ccs == 2u) ai_csf(m, 07640u);           /* TMAXFIRE */
    ai_ts(m, TEMREG);
L3565:                                               /* NOMXFIRE */
    ccs = ai_ccs(m, TEMREG);
    if (ccs == 0u) ai_caf(m, 037777u);               /* POSMAX */
    else if (ccs == 2u) ai_caf(m, 040000u);          /* NEGMAX */
    ai_ts(m, ROLLFIRE);
    ai_com(m);
    if (!ai_bzmf(m)) {                               /* EXTEND; BZMF +3 */
        ai_cs(m, TEMREG);
        ai_ts(m, TEMREG);
    }
    ai_ca(m, I);                                     /* FIRELOOK */
    if (!ai_bzf(m)) goto L3614;                      /* EXTEND; BZF FIREPLUG; TCF JETROLL */
    ai_ca(m, TIME6);                                 /* FIREPLUG */
    ai_su(m, TEMREG);
    if (ai_bzmf(m)) return leave(m, TASKOVER);       /* EXTENTST */
    goto L3614;
L3611:                                               /* NOROLL */
    ai_csf(m, 0u);                                   /* ZERO */
    ai_ts(m, ROLLFIRE);
    ai_ts(m, TEMREG);
L3614:                                               /* JETROLL */
    ai_dcaf(m, norol1t6);
    ai_dxch(m, T6LOC);
    ai_ca(m, TEMREG);
    ai_ts(m, TIME6);
    ai_ca(m, I);
    if (ai_bzf(m)) return leave(m, TASKOVER);        /* SAMEJETS */
    ccs = ai_ccs(m, ROLLFIRE);
    if (ccs == 1u || ccs == 3u) goto L3672;
    if (ccs == 2u) goto L3652;
    ai_ca(m, ROLLWORD);                              /* +TORQUE */
    ai_maskf(m, 1u);                                 /* BIT1 */
    if (ai_bzf(m)) goto L3644;                       /* EXTEND; BZF NO.9-11 */
    ai_csf(m, 1u);                                   /* BIT1 */
    ai_mask(m, ROLLWORD);
    ai_ts(m, ROLLWORD);
    ai_caf(m, 0120u);                                /* +ROLL2 */
    write(m, &io->channel6);
    goto L3672;
L3644:                                               /* NO.9-11 */
    ai_caf(m, 1u);                                   /* BIT1 */
    ai_ads(m, ROLLWORD);
    ai_caf(m, 05u);                                  /* +ROLL1 */
    write(m, &io->channel6);
    goto L3672;
L3652:                                               /* -TORQUE */
    ai_ca(m, ROLLWORD);
    ai_maskf(m, 2u);                                 /* BIT2 */
    if (ai_bzf(m)) goto L3665;                       /* EXTEND; BZF NO.12-10 */
    ai_csf(m, 2u);                                   /* BIT2 */
    ai_mask(m, ROLLWORD);
    ai_ts(m, ROLLWORD);
    ai_caf(m, 0240u);                                /* -ROLL2 */
    write(m, &io->channel6);
    goto L3672;
L3665:                                               /* NO.12-10 */
    ai_caf(m, 2u);                                   /* BIT2 */
    ai_ads(m, ROLLWORD);
    ai_caf(m, 012u);                                 /* -ROLL1 */
    write(m, &io->channel6);
L3672:                                               /* T6ENABL */
    ai_caf(m, 040000u);                              /* BIT15 */
    wor(m, &io->channel13);
    return leave(m, TASKOVER);                       /* RDAPEND */
L3676:                                               /* NOROLL1 */
    ai_lxch(m, BANKRUPT);
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, ROLLFIRE);
    write(m, &io->channel6);                         /* KILLJETS */
    return leave(m, NOQRSM);
}
