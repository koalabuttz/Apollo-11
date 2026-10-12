/* SPDX-License-Identifier: MIT */
#include "cm_entry_dap.h"
/* Labels are the original addresses: Lnnnn in bank 15, L20_nnnn in bank 20.
 * Every CCS takes its four-way skip. TS words that can overflow take their
 * skip: TS COSM/SINO/COSO/SINM/CALFA/SALFA right after the SPCOS/SPSIN
 * returns (raw A; a skipped EXTEND turns the next MP into MASK), TS
 * CM/SAVE (DOUBLE of SR), TS QREL/RREL/PREL/JETEM around RATEAVG, TS RAXERR,
 * TS JETEM +1 (the dead-zone tag), TS A in 4D/SDZ and EXDAP, TS L in
 * ANGOVCOR, CM/RCS and WHICHALF, TS -VT/180, TS XD/360, TS VDT/180 and
 * TS TON2. The other TS words store loads, constants, masks, MP, MSU or DV
 * results, CCS results or small counts, none of which overflows. */
#define LCHAN 1u
#define QREG 2u
#define EBANK 3u
#define QRUPT 012u
#define BANKRUPT 016u
#define SR 021u
#define TIME1 025u
#define TIME5 030u
#define CDUX 032u
#define CDUY 033u
#define CDUZ 034u
#define FLAGWRD2 076u
#define CM_FLAGS 0102u
#define CMTMTIME 0304u
#define SW_NDX 0305u
#define ENDBUF 0324u
#define PIPTIME 01204u
#define T6LOC 01310u
#define T5LOC 01312u
#define IMODES33 01321u
#define DAPDATR1 01466u
#define PAXERR 01476u
#define AK1 01477u
#define AK2 01500u
#define RCSFLAGS 01501u
#define CALFA 01506u
#define SALFA 01507u
#define SINM 01510u
#define COSM 01511u
#define SINO 01512u
#define COSO 01513u
#define SINOCOSM 01514u
#define COSOCOSM 01515u
#define VDT180 01567u
#define MVT180E 01570u
#define ERRORZ 01571u
#define QAXERR 01572u
#define RAXERR 01573u
#define ALFACOM 01603u
#define BETACOM 01604u
#define TOFF 01605u
#define TBITS 01606u
#define TON2 01607u
#define T2BITS 01610u
#define OUTTAG 01611u
#define NUJET 01612u
#define MVT180 01613u
#define LCX360 01614u
#define XD360 01615u
#define VSQ4API 01616u
#define JNDX 01617u
#define JNDX1 01620u
#define TON1 01621u
#define T1BITS 01622u
#define CM_SAVE 01623u
#define JETEM2 01624u
#define AOG 01661u
#define AIG 01662u
#define AMG 01663u
#define ROLL180 01664u
#define ALFA180 01665u
#define BETA180 01666u
#define MDELAOG 01675u
#define MDELAIG 01676u
#define MDELAMG 01677u
#define CMDAPMOD 01700u
#define PREL 01701u
#define QREL 01702u
#define RREL 01703u
#define BETADOT 01704u
#define PHIDOT 01705u
#define OLDELP 01706u
#define OLDELQ 01707u
#define OLDELR 01710u
#define JETAG 01711u
#define TUSED 01712u
#define PAXERR1 01713u
#define ROLLTM 01714u
#define ROLLC 01715u
#define ROLLHOLD 01717u
#define JETEM 01720u
#define GAMDOT 01723u
#define CM_GYMDT 01725u
#define P63FLAG 01727u
/* External routines (fixed-fixed). */
#define BANKCALL 04662u
#define POSTJUMP 04701u
#define IBNKCALL 04740u
#define SPCOS 05074u
#define SPSIN 05075u
#define WAITLIST 05245u
#define TASKOVER 05324u
#define RESUME 05333u
#define TWOPHSCHNG 05372u
#define PHASCHNG 05412u
#define DOWNFLAG 05560u
#define FLUSHJET 06000u
static const agc_word atdotcad[2] = { 002665u, 032066u };  /* ATDOTCAD: 2CADR ATTRATES */
static const agc_word t5idler1[2] = { 003153u, 012062u };  /* T5IDLER1: 2CADR T5IDLOC */
static const agc_word t5idler2[2] = { 003153u, 012062u };  /* T5IDLER2: 2CADR T5IDLOC */
/* The fixed words the INDEX words read: LIMITS-1..LIMITS+1 (4777-5001),
 * HALFPR-1..HALFPR+1 (5000-5002), YJETCODE-1..+1 (3201-3203) and
 * P/RJCODE-1..+1 (3204-3206). */
static const agc_word limits[] = { 037777u, 057777u, 040000u };
static const agc_word halfpr[] = { 057777u, 040000u, 020000u };
static const agc_word yjetcode[] = { 000120u, 000000u, 000240u };
static const agc_word prjcode[] = { 000005u, 000000u, 000012u };
/* An INDEX value (an erasable word, or A's corrected sign) added to a
 * table's address: the word at that offset, or a fault outside the table. */
static int value(agc_word v) { return agc_to_int(v); }
static int areg(const agc_image *m) { return agc_to_int(agc_correct(m->words[0])); }
static int lreg(const agc_image *m) { return agc_to_int(agc_correct(m->words[1])); }
static agc_word at(const agc_word *table, unsigned size, int origin, int i, uint8_t *bad) {
    i += origin;
    if (i < 0 || i >= (int)size) { *bad = 1; return 0; }
    return table[i];
}
#define TABLE(t, origin, i) at((t), (unsigned)(sizeof (t) / sizeof (t)[0]), (origin), (i), &bad)
/* An INDEX'd erasable operand: base + i inside [lo, hi], or a fault. */
static unsigned slot(unsigned base, int i, unsigned lo, unsigned hi, uint8_t *bad) {
    int a = (int)base + i;
    if (a < (int)lo || a > (int)hi) { *bad = 1; return lo; }
    return (unsigned)a;
}
/* EXTEND MSU on an ordinary erasable word: two's-complement difference of
 * the 15-bit operands, sign-extended, minus one when negative. */
static void msu(agc_image *m, unsigned k) {
    unsigned a = agc_correct(m->words[0]) & 077777u, b = ~(unsigned)ai_word(m, k) & 077777u, d = a + b + 1u;
    if (d & 040000u) d = (d | 0100000u) - 1u;
    m->words[0] = agc_extend((agc_word)(d & 077777u));
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
/* EXTEND WRITE on an I/O channel, and ROR LCHAN (channel 1 is L: the raw
 * 16-bit registers), as yaAGC defines them. */
static void write(agc_image *m, agc_word *channel) {
    *channel = (agc_word)(agc_correct(m->words[0]) & 077777u);
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static void rorl(agc_image *m) {
    m->words[0] = (agc_word)(m->words[0] | m->words[1]);
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static enum cm_entry_dap_status leave(agc_image *m, agc_word z, agc_word q) {
    if (q) ai_tc(m, q);
    m->words[2] &= 077777u;                          /* canonical frontier Q */
    m->words[5] = z;
    return CM_ENTRY_DAP_OK;
}
/* Every transfer out of the file; an INDEX fault on the way is reported
 * instead. */
#define LEAVE(z, q) return bad ? CM_ENTRY_DAP_FAULT : leave(m, (z), (q))
static uint8_t in(const agc_image *m, unsigned k, int lo, int hi) {
    agc_word v = ai_word(m, k);
    return v != 077777u && agc_to_int(v) >= lo && agc_to_int(v) <= hi;
}
enum cm_entry_dap_status cm_entry_dap(agc_image *m, cm_entry_dap_io *io) {
    unsigned a;
    uint8_t ccs, bad = 0;
    if (m->words[3] != 03000u) return CM_ENTRY_DAP_OUT_OF_SCOPE;
    switch (m->words[5]) {
    case 02434u: case 02446u: case 02476u: case 02500u: case 02507u: case 02540u: case 02543u:
    case 02551u: case 02607u: case 02631u: case 02641u: case 02665u: case 02700u: case 02712u:
    case 02724u: case 03056u: case 03064u: case 03207u: case 03215u: case 03642u: case 03650u:
    case 03717u:
        if (m->words[4] != 032000u || m->words[6] != 032006u) return CM_ENTRY_DAP_OUT_OF_SCOPE;
        break;
    case 03612u: case 03616u: case 03620u: case 03627u: case 03636u: case 03672u: case 03716u:
    case 03732u:
        if (m->words[4] != 040000u || m->words[6] != 040006u) return CM_ENTRY_DAP_OUT_OF_SCOPE;
        break;
    default: return CM_ENTRY_DAP_OUT_OF_SCOPE;
    }
    if (!in(m, SW_NDX, -13, 13) && ai_word(m, SW_NDX) != 077777u) return CM_ENTRY_DAP_OUT_OF_SCOPE;
    if (!in(m, JETAG, -1, 1) || !(in(m, CMDAPMOD, -1, 1) || ai_word(m, CMDAPMOD) == 077777u))
        return CM_ENTRY_DAP_OUT_OF_SCOPE;
    switch (m->words[5]) {
    case 02434u: goto L2434;
    case 02446u: goto L2446;
    case 02476u: goto L2476;
    case 02500u: goto L2500;
    case 02507u: goto L2507;
    case 02540u: goto L2540;
    case 02543u: goto L2543;
    case 02551u: goto L2551;
    case 02607u: goto L2607;
    case 02631u: goto L2631;
    case 02641u: goto L2641;
    case 02665u: goto L2665;
    case 02700u: goto L2700;
    case 02712u: goto L2712;
    case 02724u: goto L2724;
    case 03056u: goto L3056;
    case 03064u: goto L3064;
    case 03207u: goto L3207;
    case 03215u: goto L3215;
    case 03642u: goto L3642;
    case 03650u: goto L3650;
    case 03717u: goto L3717;
    case 03612u: goto L20_3612;
    case 03616u: goto L20_3616;
    case 03620u: goto L20_3620;
    case 03627u: goto L20_3627;
    case 03636u: goto L20_3636;
    case 03672u: goto L20_3672;
    case 03716u: goto L20_3716;
    case 03732u: goto L20_3732;
    default: break;
    }
L2434:                                               /* READGYMB */
    ai_caf(m, 012u);                                 /* TEN */
    ai_ads(m, CM_GYMDT);
    ai_caf(m, 040u);                                 /* BIT6 */
    ai_mask(m, IMODES33);
    if (ai_bzf(m)) goto L2447;                       /* EXTEND; BZF READGYM1 */
    ai_csf(m, 01u);                                  /* BIT1 */
    ai_mask(m, CM_FLAGS);
    ai_ts(m, CM_FLAGS);
    LEAVE(FLUSHJET, 02446u);                         /* TC FLUSHJET */
L2446:
    ai_tc(m, 02447u);                                /* TC CM/GYMIC +2 */
    goto L2514;
L2447:                                               /* READGYM1 */
    ai_ca(m, CDUX);
    ai_xch(m, AOG);
    msu(m, AOG);                                     /* EXTEND; MSU AOG */
    ai_ts(m, MDELAOG);
    ai_ca(m, CDUY);
    ai_xch(m, AIG);
    msu(m, AIG);                                     /* EXTEND; MSU AIG */
    ai_ts(m, MDELAIG);
    ai_ca(m, CDUZ);
    ai_xch(m, AMG);
    msu(m, AMG);                                     /* EXTEND; MSU AMG */
    ai_ts(m, MDELAMG);
    ai_cs(m, CM_FLAGS);                              /* DOBRATE? */
    ai_maskf(m, 03u);                                /* THREE */
    a = (unsigned)areg(m);                           /* INDEX A; TC +1 */
    ai_tc(m, 02472u);
    switch (a) {
    case 0u: goto L2472;
    case 1u: goto L2473;
    case 2u: goto L2474;
    case 3u: goto L2475;
    default: return CM_ENTRY_DAP_FAULT;
    }
L2472:
    ai_tc(m, 02473u);                                /* TC DOBRATE */
    goto L2501;
L2473:
    ai_tc(m, 02474u);                                /* TC CM/GYMIC */
    goto L2512;
L2474:                                               /* NOOP */
L2475:
    LEAVE(FLUSHJET, 02476u);                         /* TC FLUSHJET */
L2476:
    LEAVE(PHASCHNG, 02477u);                         /* TC PHASCHNG; OCT 00006 */
L2500:
    LEAVE(TASKOVER, 02501u);                         /* TC TASKOVER */
L2501:                                               /* DOBRATE */
    ai_caf(m, 01u);                                  /* ONE */
L2502:                                               /* DOBRATE1 */
    ai_ts(m, JETEM);
    ai_caf(m, 012u);                                 /* TEN */
    LEAVE(WAITLIST, 02505u);                         /* TC WAITLIST; 2CADR READGYMB */
L2507:
    ccs = ai_ccs(m, JETEM);
    if (ccs == 0u) {
        ai_tc(m, 02511u);                            /* TC BODYRATE */
        goto L2536;
    }
    if (ccs == 1u) LEAVE(TASKOVER, 02512u);          /* TC TASKOVER */
    if (ccs == 3u) goto L2513;
L2512:                                               /* CM/GYMIC */
    ai_ads(m, CM_FLAGS);
L2513:
    ai_caf(m, 0u);                                   /* ZERO */
L2514:
    if (!ai_ts(m, JETAG)) ai_ts(m, OLDELP);          /* raw A from the FLUSHJET return can overflow */
    ai_ts(m, OLDELQ);
    ai_ts(m, OLDELR);
    ai_ts(m, GAMDOT);
    ai_tc(m, 02522u);                                /* TC DOBRATE1 */
    goto L2502;
L2522:                                               /* ANGOVCOR */
    if (!ai_ts(m, LCHAN)) goto TCQ;                  /* TS L; TC Q (no overflow) */
    ai_caf(m, TABLE(limits, 1, areg(m)));            /* INDEX A; CAF LIMITS */
    ai_ads(m, LCHAN);
    goto TCQ;                                        /* TC Q */
L2530:                                               /* RATEAVG */
    ai_com(m);
    ai_ad(m, JETEM);
    ai_mpf(m, 020000u);                              /* EXTEND; MP HALF */
    ai_ad(m, JETEM);
    goto TCQ;                                        /* TC Q */
L2536:                                               /* BODYRATE */
    ai_ca(m, AMG);
    LEAVE(SPCOS, 02540u);                            /* TC SPCOS */
L2540:
    if (!ai_ts(m, COSM)) ai_ca(m, AOG);
    LEAVE(SPSIN, 02543u);                            /* TC SPSIN */
L2543:
    if (ai_ts(m, SINO)) ai_mask(m, COSM);            /* overflow skips EXTEND: MASK COSM */
    else ai_mp(m, COSM);                             /* EXTEND; MP COSM */
    ai_ts(m, SINOCOSM);
    ai_ca(m, AOG);
    LEAVE(SPCOS, 02551u);                            /* TC SPCOS */
L2551:
    if (ai_ts(m, COSO)) ai_mask(m, COSM);            /* overflow skips EXTEND: MASK COSM */
    else ai_mp(m, COSM);                             /* EXTEND; MP COSM */
    ai_ts(m, COSOCOSM);
    ai_cs(m, MDELAMG);
    ai_mp(m, SINO);                                  /* EXTEND */
    ai_dxch(m, JETEM);
    ai_cs(m, MDELAIG);
    ai_mp(m, COSOCOSM);                              /* EXTEND */
    ai_das(m, JETEM);
    ai_ca(m, JETEM);
    ai_xch(m, OLDELQ);
    ai_tc(m, 02570u);                                /* TC RATEAVG */
    goto L2530;
L2570:
    if (!ai_ts(m, QREL)) ai_cs(m, MDELAMG);
    ai_mp(m, COSO);                                  /* EXTEND */
    ai_dxch(m, JETEM);
    ai_ca(m, MDELAIG);
    ai_mp(m, SINOCOSM);                              /* EXTEND */
    ai_das(m, JETEM);
    ai_ca(m, JETEM);
    ai_xch(m, OLDELR);
    ai_tc(m, 02604u);                                /* TC RATEAVG */
    goto L2530;
L2604:
    if (!ai_ts(m, RREL)) ai_ca(m, AMG);
    LEAVE(SPSIN, 02607u);                            /* TC SPSIN */
L2607:
    if (ai_ts(m, SINM)) ai_mask(m, MDELAIG);         /* overflow skips EXTEND: MASK -DELAIG */
    else ai_mp(m, MDELAIG);                          /* EXTEND; MP -DELAIG */
    ai_ts(m, JETEM);
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ddoubl(m);
    ai_ad(m, MDELAOG);
    ai_ad(m, JETEM);
    ai_cs(m, 0u);                                    /* CS A */
    if (!ai_ts(m, JETEM)) ai_xch(m, OLDELP);
    ai_tc(m, 02623u);                                /* TC RATEAVG */
    goto L2530;
L2623:
    if (ai_ts(m, PREL)) {                            /* overflow skips CCS GAMDOT */
        ai_tc(m, 02626u);                            /* TC +2 */
        goto L2627;
    }
    ccs = ai_ccs(m, GAMDOT);
    if (ccs == 0u) {
        ai_tc(m, 02626u);                            /* TC +2 */
        goto L2627;
    }
    if (ccs == 1u) {
        ai_tc(m, 02627u);                            /* TC NOGAMDUT */
        goto L2651;
    }
    if (ccs == 3u) goto L2630;
L2627:
    ai_cs(m, ROLL180);
L2630:
    LEAVE(SPSIN, 02631u);                            /* TC SPSIN */
L2631:
    ai_mp(m, GAMDOT);                                /* EXTEND */
    ai_ts(m, JETEM + 1u);
    ai_mpf(m, 065033u);                              /* EXTEND; MP SINTRIM */
    ai_ads(m, PREL);
    ai_ca(m, ROLL180);
    LEAVE(SPCOS, 02641u);                            /* TC SPCOS */
L2641:
    ai_com(m);
    ai_mp(m, GAMDOT);                                /* EXTEND */
    ai_ads(m, QREL);
    ai_cs(m, JETEM + 1u);
    ai_mpf(m, 036044u);                              /* EXTEND; MP COSTRIM */
    ai_ads(m, RREL);
L2651:                                               /* NOGAMDUT */
    ai_caf(m, 04000u);                               /* BIT12 */
    ai_mask(m, CM_FLAGS);
    if (ai_bzf(m)) LEAVE(TASKOVER, 0u);              /* EXTEND; STBYDUMP BZF TASKOVER */
    ai_caf(m, 037777u);                              /* POSMAX */
    ai_ts(m, TIME5);
    ai_dcaf(m, atdotcad);                            /* EXTEND; DCA ATDOTCAD */
    ai_dxch(m, T5LOC);
    LEAVE(TASKOVER, 02663u);                         /* TC TASKOVER */
L2665:                                               /* ATTRATES */
    ai_lxch(m, BANKRUPT);
    ai_qxch(m, QRUPT);                               /* EXTEND; QXCH QRUPT */
    ai_ca(m, SR);
    ai_double(m);
    if (!ai_ts(m, CM_SAVE)) ai_ca(m, QREL);
    ai_ad(m, ALFA180);
    ai_tc(m, 02676u);                                /* TC ANGOVCOR */
    goto L2522;
L2676:
    ai_ts(m, ALFA180);
    LEAVE(SPCOS, 02700u);                            /* TC SPCOS */
L2700:
    if (!ai_ts(m, CALFA)) ai_ts(m, PHIDOT);
    ai_mp(m, PREL);                                  /* EXTEND */
    ai_xch(m, PHIDOT);
    ai_mp(m, RREL);                                  /* EXTEND */
    ai_ts(m, BETADOT);
    ai_ca(m, ALFA180);
    LEAVE(SPSIN, 02712u);                            /* TC SPSIN */
L2712:
    if (ai_ts(m, SALFA)) ai_mask(m, RREL);           /* overflow skips EXTEND: MASK RREL */
    else ai_mp(m, RREL);                             /* EXTEND; MP RREL */
    ai_ads(m, PHIDOT);
    ai_cs(m, SALFA);
    ai_mp(m, PREL);                                  /* EXTEND */
    ai_ads(m, BETADOT);
    ai_ads(m, BETA180);
    LEAVE(SPSIN, 02724u);                            /* TC SPSIN */
L2724:
    ai_mp(m, PHIDOT);                                /* EXTEND */
    ai_ad(m, ALFA180);
    ai_tc(m, 02730u);                                /* TC ANGOVCOR */
    goto L2522;
L2730:
    ai_ts(m, ALFA180);
    ai_com(m);
    ai_ad(m, ALFACOM);
    ai_tc(m, 02734u);                                /* TC ANGOVCOR */
    goto L2522;
L2734:
    ai_ts(m, AK1);
    ai_ts(m, QAXERR);
    ai_ca(m, PHIDOT);
    ai_ad(m, ROLL180);
    ai_tc(m, 02741u);                                /* TC ANGOVCOR */
    goto L2522;
L2741:
    ai_ts(m, ROLLTM);
    ai_ts(m, ROLL180);
    ai_cs(m, BETA180);
    ai_ad(m, BETACOM);
    if (!ai_ts(m, RAXERR)) ai_caf(m, 04u);           /* BIT3 */
    ai_mask(m, CM_FLAGS);
    if (ai_bzf(m)) goto L3037;                       /* EXTEND; BZF EXDAP */
    ai_csf(m, 01u);                                  /* ONE */
    ai_ts(m, CMDAPMOD);
    ai_ts(m, AK1);
    ai_ts(m, AK2);
    ai_cs(m, PREL);
    ai_mpf(m, 065033u);                              /* EXTEND; MP SINTRIM */
    ai_ad(m, RREL);
    ai_tc(m, 02763u);                                /* TC 2D/SDZ */
    goto L3027;
L2763:
    ai_caf(m, TABLE(yjetcode, 1, areg(m)));          /* INDEX A; CAF YJETCODE */
    ai_ts(m, JETEM);
    ai_ca(m, QREL);
    ai_tc(m, 02770u);                                /* TC 2D/SDZ */
    goto L3027;
L2770:                                               /* EXDAPIN */
    ai_caf(m, TABLE(prjcode, 1, areg(m)));           /* INDEX A; CAF P/RJCODE */
    ai_ads(m, JETEM);
    write(m, &io->channel5);                         /* EXTEND; WRITE PYJETS */
    ccs = ai_ccs(m, JETAG);
    if (ccs == 0u) {
        ai_tc(m, 02777u);                            /* TC CM/RCS */
        goto L3216;
    }
    if (ccs == 1u) {
        ai_tc(m, 03000u);                            /* TC CM/FDAI */
        goto L3677;
    }
    if (ccs == 2u) {
        ai_tc(m, 03001u);                            /* TC CM/FDAIR -1 */
        goto L3704;
    }
L3001:                                               /* 3DDZ */
    ccs = ai_ccs(m, 0u);                             /* CCS A */
    if (ccs == 0u) { ai_adf(m, 037267u); goto L3033; }   /* YAWLIM; TCF DZCOM */
    if (ccs == 1u) goto L3033;                       /* TCF DZCOM */
    if (ccs == 2u) ai_adf(m, 037267u);               /* YAWLIM */
    goto L3034;                                      /* TCF DZNOCOM */
L3006:                                               /* BIASEDZ */
    ai_ts(m, JETEM2);
    ccs = ai_ccs(m, 0u);                             /* CCS A */
    if (ccs == 0u) ai_csf(m, 067u);                  /* CM/BIAS; TCF +2 */
    else if (ccs == 2u) ai_caf(m, 067u);             /* CM/BIAS */
    ai_ad(m, LCHAN);
    ai_lxch(m, QREG);
    ai_tc(m, 03016u);                                /* TC 3DDZ */
    goto L3001;
L3016:
    ai_dxch(m, LCHAN);                               /* DXCH L */
    ccs = ai_ccs(m, JETEM2);                         /* 4D/SDZ */
    if (ccs == 0u || ccs == 2u) ai_adf(m, 037734u);  /* 4D/SLIM */
    if (ai_ts(m, 0u)) ai_zl(m);                      /* TS A; overflow skips TCF +2: ZL */
    ai_xch(m, JETEM2);
L3027:                                               /* 2D/SDZ */
    ccs = ai_ccs(m, 0u);                             /* CCS A */
    if (ccs == 1u) goto L3034;                       /* TCF +3 */
    if (ccs == 0u) { ai_adf(m, 037756u); goto L3034; }   /* YDOTLIM; TCF +3 */
    if (ccs == 2u) ai_adf(m, 037756u);               /* YDOTLIM */
L3033:                                               /* DZCOM */
    ai_com(m);
L3034:                                               /* DZNOCOM */
    if (!ai_ts(m, JETEM + 1u)) ai_caf(m, 0u);        /* ZERO */
    goto TCQ;                                        /* TC Q */
L3037:                                               /* EXDAP */
    ai_ts(m, CMDAPMOD);
    ccs = ai_ccs(m, CALFA);
    if (ccs == 0u || ccs == 2u) ai_adf(m, 011277u);  /* C45LIM */
    if (!ai_ts(m, 0u)) goto L3126;                   /* TS A; TCF EXDAP2 */
    ccs = ai_ccs(m, CALFA);
    if (ccs == 2u) goto L3051;
    if (ccs == 3u) goto L3052;
    ccs = ai_ccs(m, P63FLAG);
    if (ccs == 1u) goto L3052;
    if (ccs == 2u) goto L3053;
    if (ccs == 3u) goto L3054;
L3051:
    ai_tc(m, 03052u);                                /* TC EXDAP4 */
    goto L3064;
L3052:
    ai_tc(m, 03053u);                                /* TC +2 */
    goto L3054;
L3053:
    ai_tc(m, 03054u);                                /* TC EXDAP4 */
    goto L3064;
L3054:
    LEAVE(PHASCHNG, 03055u);                         /* TC PHASCHNG; OCT 40334 */
L3056:
    ai_csf(m, 01u);                                  /* ONE */
    ai_ts(m, P63FLAG);
    ai_caf(m, 04064u);                               /* NSEC */
    LEAVE(WAITLIST, 03062u);                         /* TC WAITLIST; 2CADR WAKEP62 */
L3064:                                               /* EXDAP4 */
    ccs = ai_ccs(m, JETAG);
    if (ccs <= 1u) goto L3073;                       /* TCF EXDAP3 */
    if (ccs == 2u) ai_caf(m, 0u);                    /* ZERO */
    write(m, &io->channel6);                         /* EXTEND; WRITE ROLLJETS */
    ai_ts(m, JETAG);
L3073:                                               /* EXDAP3 */
    ccs = ai_ccs(m, CALFA);
    if (ccs == 0u) { ai_ca(m, RAXERR); goto L3101; }   /* TCF EXDAP1 */
    if (ccs == 1u) goto L3101;                       /* TCF EXDAP1 */
    if (ccs == 2u) ai_csf(m, 0u);                    /* ZERO */
    ai_ts(m, CMDAPMOD);
    ai_cs(m, RAXERR);
L3101:                                               /* EXDAP1 */
    ai_ts(m, RAXERR);
    ai_ts(m, AK2);
    ai_ts(m, LCHAN);
    ccs = ai_ccs(m, CMDAPMOD);
    if (ccs == 0u) {
        ai_tc(m, 03106u);                            /* TC +3 */
        goto L3110;
    }
    if (ccs == 3u) goto L3110;
    if (ccs == 1u) ai_caf(m, 01u);                   /* ONE */
    ai_ca(m, slot(RREL, areg(m), RREL, BETADOT, &bad));   /* INDEX A; CA RREL */
    goto L3111;
L3110:
    ai_ca(m, RREL);
L3111:
    ai_tc(m, 03112u);                                /* TC BIASEDZ */
    goto L3006;
L3112:
    rorl(m);                                         /* EXTEND; ROR LCHAN */
    ai_caf(m, TABLE(yjetcode, 1, areg(m)));          /* INDEX A; CAF YJETCODE */
    ai_ts(m, JETEM);
    ai_ca(m, QAXERR);
    ai_ts(m, LCHAN);
    ai_ca(m, QREL);
    ai_tc(m, 03123u);                                /* TC BIASEDZ */
    goto L3006;
L3123:
    rorl(m);                                         /* EXTEND; ROR LCHAN */
    goto L2770;                                      /* TCF EXDAPIN */
L3126:                                               /* EXDAP2 */
    ai_incr(m, CMDAPMOD);
    ai_csf(m, 01u);                                  /* ONE */
    ai_ts(m, JETAG);
    ccs = ai_ccs(m, P63FLAG);
    if (ccs == 3u) goto L3135;
    if (ccs == 0u) ai_ts(m, P63FLAG);
    ccs = ai_ccs(m, SALFA);                          /* NOOP; CCS SALFA */
    if (ccs == 1u || ccs == 3u) goto L3140;          /* TCF +2 */
    if (ccs == 2u) {
        ai_ca(m, RAXERR);
        goto L3140;
    }
L3135:
    ai_cs(m, RAXERR);                                /* TCF +2 */
L3140:
    ai_ts(m, PAXERR1);
    ai_mpf(m, 020000u);                              /* EXTEND; MP HALF */
    ai_xch(m, PAXERR1);
    ai_ts(m, LCHAN);
    ccs = ai_ccs(m, SALFA);
    if (ccs == 0u) ai_cs(m, BETADOT);
    if (ccs <= 1u) {
        ai_tc(m, 03150u);                            /* TC +2 */
        goto L3151;
    }
    if (ccs == 2u) ai_ca(m, BETADOT);
L3151:
    ai_tc(m, 03152u);                                /* TC BIASEDZ */
    goto L3006;
L3152:
    rorl(m);                                         /* EXTEND; ROR LCHAN */
    ai_caf(m, TABLE(prjcode, 1, areg(m)));           /* INDEX A; CAF P/RJCODE */
    write(m, &io->channel6);                         /* EXTEND; WRITE ROLLJETS */
    ai_ca(m, ROLLHOLD);
    msu(m, ROLL180);                                 /* EXTEND; MSU ROLL/180 */
    ai_ts(m, LCHAN);
    ccs = ai_ccs(m, SALFA);
    if (ccs == 0u) ai_ca(m, LCHAN);
    if (ccs <= 1u) {
        ai_tc(m, 03167u);                            /* TC EXDAP1 */
        goto L3101;
    }
    if (ccs == 2u) ai_cs(m, LCHAN);
    ai_tc(m, 03171u);                                /* TC EXDAP1 */
    goto L3101;
L3207:                                               /* SETJTAG */
    ai_cs(m, TIME1);
    ai_ts(m, TUSED);
    ai_caf(m, 01u);                                  /* ONE */
    ai_ts(m, JETAG);
    LEAVE(PHASCHNG, 03214u);                         /* TC PHASCHNG; OCT 00001 */
L3215:
    LEAVE(TASKOVER, 03216u);                         /* TC TASKOVER */
L3216:                                               /* CM/RCS */
    ai_csf(m, 01u);                                  /* ONE */
    ai_ts(m, JNDX);
    ai_csf(m, 050u);                                 /* 2T/TCDU */
    ai_mp(m, PREL);                                  /* EXTEND */
    ai_ad(m, LCHAN);
    if (!ai_ts(m, MVT180)) ai_cs(m, ROLL180);
    ai_ts(m, SR);
    ai_cs(m, CM_FLAGS);
    ai_maskf(m, 010u);                               /* BIT4 */
    if (ai_bzf(m)) goto L3240;                       /* EXTEND; BZF GETLCX */
    ai_ads(m, CM_FLAGS);
    ai_ca(m, ROLLC);
    ai_ad(m, SR);
    ai_xch(m, LCX360);
    goto L3300;                                      /* TCF COMPAT */
L3240:                                               /* GETLCX */
    ai_caf(m, 020000u);                              /* POS1/2 */
    ai_double(m);
    ai_ad(m, ROLLC);
    ai_xch(m, LCX360);
    ai_ca(m, SR);
    ai_adf(m, 057777u);                              /* NEG1/2 */
    ai_adf(m, 057777u);                              /* NEG1/2 */
    ai_xch(m, LCX360);
    ai_ads(m, LCX360);
    ccs = ai_ccs(m, MVT180);
    if (ccs == 0u || ccs == 2u) ai_adf(m, 01u);      /* ONE */
    ai_mp(m, MVT180);                                /* EXTEND */
    ai_mpf(m, 023617u);                              /* EXTEND; MP 1/16A1 */
    ai_dv(m, CALFA);                                 /* EXTEND */
    ai_ts(m, LCHAN);
    ccs = ai_ccs(m, LCX360);
    if (ccs == 0u) ai_caf(m, 020000u);               /* POS1/2; TCF +2 */
    else if (ccs == 2u) ai_csf(m, 020000u);          /* POS1/2 */
    ai_ad(m, LCX360);
    ai_ad(m, LCHAN);
    if (!ai_ts(m, LCHAN)) goto L3300;                /* TS L; TCF COMPAT */
    ai_csf(m, TABLE(halfpr, 1, areg(m)));            /* TRTAGXPI: INDEX A; CS HALFPR */
    ai_double(m);
    ai_ads(m, LCX360);
L3300:                                               /* COMPAT */
    ai_ca(m, LCX360);
    ai_mp(m, CALFA);                                 /* EXTEND */
    ai_ts(m, LCX360);
    ccs = ai_ccs(m, CMDAPMOD);
    if (ccs == 0u) {
        ai_tc(m, 03306u);                            /* TC DZCALL1 */
        goto L3675;
    }
    if (ccs == 1u) ai_tc(m, 03307u);                 /* TC +1 */
    if (ccs <= 2u) ai_ca(m, LCX360);
    ai_ts(m, LCX360);
    ai_ts(m, ERRORZ);
    ai_ts(m, PAXERR1);
    ai_ca(m, MVT180);
    ai_ts(m, SR);
    ai_ca(m, SR);
    ai_ts(m, MVT180E);
    ai_xch(m, MVT180);
    ai_mp(m, MVT180);                                /* EXTEND */
    ai_mpf(m, 023617u);                              /* EXTEND; MP 180/8ATT */
    ai_ts(m, VSQ4API);
    ai_double(m);                                    /* WHICHALF */
    ai_com(m);
    ai_adf(m, 0133u);                                /* BUFLIM */
    if (ai_ts(m, LCHAN)) goto L3332;                 /* overflow skips CCS -VT/180 */
    ccs = ai_ccs(m, MVT180);
    if (ccs == 2u) ai_ca(m, LCHAN);
    if (ccs != 0u) goto L3335;
L3332:
    ai_cs(m, LCHAN);                                 /* TCF +2 */
L3335:
    ai_ad(m, LCX360);
    ai_adf(m, 0133u);                                /* BUFLIM */
    if (ai_bzmf(m)) goto L3355;                      /* EXTEND; BZMF REFLECT */
    ai_com(m);
    ai_adf(m, 0133u);                                /* BUFLIM */
    ai_adf(m, 0133u);                                /* BUFLIM */
    if (ai_bzmf(m)) goto L3364;                      /* EXTEND; BZMF DZ1 */
    ai_csf(m, 012u);                                 /* VSQMIN */
    ai_ad(m, VSQ4API);
    if (ai_bzmf(m)) goto L3657;                      /* EXTEND; BZMF DZCALL */
    ai_cs(m, JNDX);
    ai_ts(m, JNDX1);
    ai_tc(m, 03355u);                                /* TC OVRLINE1 */
    goto L3437;
L3355:                                               /* REFLECT */
    ai_cs(m, MVT180);
    ai_ts(m, MVT180);
    ai_ts(m, SR);
    ai_cs(m, LCX360);
    ai_ts(m, LCX360);
    ai_cs(m, JNDX);
    ai_ts(m, JNDX);
L3364:                                               /* DZ1 */
    ai_csf(m, 012u);                                 /* VSQMIN */
    ai_ad(m, VSQ4API);
    if (!ai_bzmf(m)) goto L3376;                     /* EXTEND; BZMF DZ2; TCF MAXVTEST */
    ai_csf(m, 0266u);                                /* DZ2: XMIN/360 */
    ai_ad(m, LCX360);
    ai_ad(m, SR);
    if (ai_bzmf(m)) goto L3657;                      /* EXTEND; BZMF DZCALL */
L3376:                                               /* MAXVTEST */
    ai_cs(m, JNDX);
    ai_ts(m, JNDX1);
    ai_csf(m, 0133u);                                /* XS/360 */
    ai_ad(m, VSQ4API);
    ai_ad(m, LCX360);
    if (!ai_ts(m, XD360)) ai_adf(m, 070706u);        /* -VM/360K */
    ai_com(m);
    if (ai_bzmf(m)) goto L3415;                      /* EXTEND; BZMF MAXVTIM1 */
    ai_ca(m, XD360);
    ai_mpf(m, 020000u);                              /* EXTEND; MP KTRCS */
    ai_ddoubl(m);
    ai_tc(m, 03415u);                                /* TC GETON1 */
    goto L3420;
L3415:                                               /* MAXVTIM1 */
    ai_zq(m);                                        /* EXTEND; ZQ */
    ai_csf(m, 070706u);                              /* -VMT/180 */
L3420:                                               /* GETON1 */
    if (!ai_ts(m, VDT180)) ai_ad(m, MVT180);
    ai_double(m);
    ai_mpf(m, 023617u);                              /* EXTEND; MP 180/8ATT */
    ai_ts(m, TON1);
    if (ai_bzmf(m)) goto L3431;                      /* EXTEND; BZMF OVRLINE */
    ai_tc(m, 03431u);                                /* TC GETON2 */
    goto L3443;
L3431:                                               /* OVRLINE */
    ccs = ai_ccs(m, QREG);                           /* CCS Q */
    if (ccs == 0u) goto L3437;                       /* TCF OVRLINE1 */
    if (ccs == 1u) ai_ca(m, JNDX1);                  /* MAXVTIM2 */
    if (ccs <= 2u) ai_ts(m, JNDX);
    ai_cs(m, TON1);
    goto L3442;                                      /* TCF OVRLINE2 +1 */
L3437:                                               /* OVRLINE1 */
    ai_cs(m, MVT180);
    ai_ts(m, VDT180);
    ai_caf(m, 0u);                                   /* OVRLINE2: ZERO */
L3442:
    ai_ts(m, TON1);
L3443:                                               /* GETON2 */
    ai_ca(m, VDT180);
    ai_double(m);
    ai_mpf(m, 023617u);                              /* EXTEND; MP 180/8ATT */
    ai_double(m);
    if (!ai_ts(m, TON2)) ai_com(m);
    if (!ai_bzmf(m)) {                               /* EXTEND; BZMF GETOFF */
        ai_ts(m, TON2);
        ai_ca(m, JNDX);
        ai_ts(m, JNDX1);
    }
    ai_cs(m, TON2);                                  /* GETOFF */
    ai_mp(m, VDT180);                                /* EXTEND */
    ai_ts(m, XD360);
    ai_cs(m, VDT180);
    if (ai_bzf(m)) goto L3501;                       /* EXTEND; BZF TOFFOVFL */
    ai_ad(m, MVT180);
    ai_mp(m, TON1);                                  /* EXTEND */
    ai_ad(m, XD360);
    ai_ad(m, LCX360);
    ai_zl(m);
    ai_xch(m, LCHAN);
    ai_dv(m, VDT180);                                /* EXTEND */
    if (ai_bzf(m)) {                                 /* EXTEND; BZF GETOFF2 */
        ai_xch(m, LCHAN);                            /* GETOFF2 */
        ai_dv(m, VDT180);                            /* EXTEND */
        ai_mpf(m, 0620u);                            /* EXTEND; MP 2JETT */
        goto L3510;
    }
L3501:                                               /* TOFFOVFL */
    ai_caf(m, 0620u);                                /* 2JETT; TCF TIMSCAL */
L3510:                                               /* TIMSCAL */
    ai_ts(m, TOFF);
    ai_caf(m, 01440u);                               /* 4JETT */
    ai_mp(m, TON1);                                  /* EXTEND */
    ai_ts(m, TON1);
    ai_caf(m, 01440u);                               /* 4JETT */
    ai_mp(m, TON2);                                  /* EXTEND */
    ai_ts(m, TON2);
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, JETAG);
    ai_caf(m, 01u);                                  /* ONE */
    ai_ts(m, SW_NDX);
    ai_ca(m, TIME1);                                 /* TIMETST */
    ai_adf(m, 020000u);                              /* POS1/2 */
    ai_adf(m, 020000u);                              /* POS1/2 */
    ai_ads(m, TUSED);
    ai_caf(m, 077464u);                              /* -T-3 */
    ai_ads(m, TUSED);
    ai_csf(m, 02u);                                  /* TWO */
    ai_ad(m, TON1);
    if (ai_bzmf(m)) goto L3550;                      /* EXTEND; BZMF TIMETST1 */
    ai_caf(m, TABLE(prjcode, 1, value(ai_word(m, JNDX))));   /* INDEX JNDX; CAF P/RJCODE */
    ai_ts(m, T1BITS);
    ai_ca(m, TON1);
    ai_ads(m, TUSED);
    if (ai_bzmf(m)) goto L3552;                      /* EXTEND; BZMF TOFFTEST */
    ai_caf(m, 0u);                                   /* ZERO; TCF TIMETST3 */
    ai_ts(m, TON1);                                  /* TIMETST3 */
    ai_csf(m, 01u);                                  /* ONE */
    goto L3605;
L3550:                                               /* TIMETST1 */
    ai_csf(m, 01u);                                  /* ONE */
    ai_ts(m, TON1);
L3552:                                               /* TOFFTEST */
    ai_csf(m, 02u);                                  /* TWO */
    ai_ad(m, TOFF);
    if (ai_bzmf(m)) goto L3564;                      /* EXTEND; BZMF TIMETST2 */
    ai_ca(m, TOFF);
    ai_ads(m, TUSED);
    if (ai_bzmf(m)) goto L3566;                      /* EXTEND; BZMF TON2TEST */
    ai_caf(m, 0u);                                   /* ZERO; TCF TIMETST4 */
    goto L3605;
L3564:                                               /* TIMETST2 */
    ai_csf(m, 01u);                                  /* ONE */
    ai_ts(m, TOFF);
L3566:                                               /* TON2TEST */
    ai_csf(m, 02u);                                  /* TWO */
    ai_ad(m, TON2);
    if (ai_bzmf(m)) goto L3606;                      /* EXTEND; BZMF TIMETST5 */
    ai_caf(m, TABLE(prjcode, 1, value(ai_word(m, JNDX1))));  /* INDEX JNDX1; CAF P/RJCODE */
    ai_ts(m, T2BITS);
    ai_ca(m, TON2);
    ai_ads(m, TUSED);
    if (ai_bzmf(m)) goto L3610;                      /* EXTEND; BZMF JETCALL1 */
    ai_caf(m, 0u);                                   /* ZERO; TCF TIMETST5 +1 */
    goto L3607;
L3605:                                               /* TIMETST4 */
    ai_ts(m, TOFF);
L3606:                                               /* TIMETST5 */
    ai_csf(m, 01u);                                  /* ONE */
L3607:
    ai_ts(m, TON2);
L3610:                                               /* JETCALL1 */
    ai_caf(m, 0u);                                   /* ZERO */
    ai_ts(m, OUTTAG);
    ai_ts(m, NUJET);
    ai_ts(m, TBITS);
    ai_dxch(m, TON1);
    ccs = ai_ccs(m, 0u);                             /* CCS A */
    if (ccs == 0u) goto L3633;                       /* TCF JETCALL2 */
    if (ccs == 2u) goto L3620;
    if (ccs == 3u) goto L3621;
L3617:                                               /* JETCALL3 */
    ai_lxch(m, NUJET);
L3620:
    ai_csf(m, 01u);                                  /* ONE */
L3621:
    ai_dxch(m, TOFF);
    ccs = ai_ccs(m, 0u);                             /* CCS A */
    if (ccs == 0u) goto L3633;                       /* TCF JETCALL2 */
    if (ccs == 1u) ai_lxch(m, NUJET);
    if (ccs <= 2u) ai_csf(m, 01u);                   /* ONE */
    ai_dxch(m, TON2);
    ccs = ai_ccs(m, 0u);                             /* CCS A */
    if (ccs == 0u) goto L3633;                       /* TCF JETCALL2 */
    if (ccs == 1u) ai_lxch(m, NUJET);
    if (ccs <= 2u) {
        ai_tc(m, 03633u);                            /* TC JETACTN */
        goto L3642;
    }
L3633:                                               /* JETCALL2 */
    ai_xch(m, LCHAN);
    ai_ads(m, NUJET);
    ai_xch(m, LCHAN);
    ai_adf(m, 01u);                                  /* ONE */
    LEAVE(WAITLIST, 03640u);                         /* TC WAITLIST; 2CADR JETCALL */
L3642:                                               /* JETACTN */
    ai_ca(m, NUJET);
L3643:
    write(m, &io->channel6);                         /* EXTEND; WRITE ROLLJETS */
    ccs = ai_ccs(m, OUTTAG);
    if (ccs == 0u) LEAVE(TASKOVER, 03647u);          /* TC TASKOVER */
    if (ccs == 2u) goto L3650;
    if (ccs == 3u) goto L3651;
L3647:                                               /* ROLLDUMP */
    ai_tc(m, 03650u);                                /* TC CM/FDAIR */
    goto L3705;
L3650:                                               /* JETCALL */
    ai_caf(m, 02u);                                  /* BIT2 */
L3651:
    ai_ts(m, OUTTAG);
    ai_mask(m, CM_FLAGS);
    if (ai_bzf(m)) goto L3643;                       /* EXTEND; BZF JETACTN +1 */
    ai_zl(m);
    goto L3617;                                      /* TCF JETCALL3 */
L3657:                                               /* DZCALL */
    ai_cs(m, CMDAPMOD);
    ai_maskf(m, 01u);                                /* BIT1 */
    ai_ts(m, LCHAN);
    ai_ca(m, slot(ROLLTM, areg(m), ROLLTM, ROLLC, &bad));   /* INDEX A; CA ROLLTM */
    ai_ts(m, slot(0u, lreg(m), 0u, LCHAN, &bad));    /* INDEX L; TS A */
    ai_ad(m, LCHAN);
    ai_tc(m, 03670u);                                /* TC ANGOVCOR */
    goto L2522;
L3670:
    ai_ts(m, ROLLHOLD);
    ai_caf(m, 0u);                                   /* ZERO */
    write(m, &io->channel6);                         /* EXTEND; WRITE ROLLJETS */
    ai_ts(m, VDT180);
L3675:                                               /* DZCALL1 */
    ai_ts(m, JETAG);
    ai_tc(m, 03677u);                                /* TC ROLLDUMP */
    goto L3647;
L3677:                                               /* CM/FDAI */
    ai_cs(m, PHIDOT);
    ai_mp(m, CALFA);                                 /* EXTEND */
    ai_mpf(m, 020000u);                              /* EXTEND; MP HALF */
L3704:
    ai_ads(m, PAXERR1);
L3705:                                               /* CM/FDAIR */
    ai_caf(m, 020000u);                              /* HALF */
    ai_mp(m, PAXERR1);                               /* EXTEND */
    ai_ts(m, PAXERR);
    ai_cs(m, SW_NDX);                                /* CM/DUMPR */
    ai_ts(m, SW_NDX);
    if (!ai_bzmf(m)) LEAVE(IBNKCALL, 03716u);        /* EXTEND; BZMF CMTMFILE; TC IBNKCALL; CADR NEEDLER */
    ai_adf(m, 03u);                                  /* CMTMFILE: THREE */
    if (!ai_bzmf(m)) {                               /* EXTEND; BZMF SAVENDX */
        ai_ca(m, TIME1);
        ai_ts(m, CMTMTIME);
        ai_csf(m, 015u);                             /* THIRTEEN */
    }
    ai_ts(m, SW_NDX);                                /* SAVENDX */
    ai_dca(m, PREL);                                 /* EXTEND */
    ai_dxch(m, slot(ENDBUF - 1u, value(ai_word(m, SW_NDX)), ENDBUF - 016u, ENDBUF - 1u, &bad));   /* INDEX SW/NDX; DXCH ENDBUF -1 */
    ai_ca(m, RREL);
    ai_ts(m, slot(ENDBUF + 1u, value(ai_word(m, SW_NDX)), ENDBUF - 014u, ENDBUF + 1u, &bad));     /* INDEX SW/NDX; TS ENDBUF +1 */
    goto L3736;
L3717:
    ai_tc(m, 03720u);                                /* TC CM/END */
L3736:                                               /* CM/END */
    if (bad) return CM_ENTRY_DAP_FAULT;
    ai_ca(m, CM_SAVE);
    ai_ts(m, SR);
    ai_dcaf(m, t5idler2);                            /* EXTEND; DCA T5IDLER2 */
    ai_dxch(m, T5LOC);
    LEAVE(RESUME, 03744u);                           /* TC RESUME */
TCQ:                                                 /* TC Q runs the TC held in Q */
    if (bad) return CM_ENTRY_DAP_FAULT;
    a = m->words[2] & 077777u;
    ai_tc(m, 3u);
    switch (a) {
    case 02570u: goto L2570;
    case 02604u: goto L2604;
    case 02623u: goto L2623;
    case 02676u: goto L2676;
    case 02730u: goto L2730;
    case 02734u: goto L2734;
    case 02741u: goto L2741;
    case 02763u: goto L2763;
    case 02770u: goto L2770;
    case 03016u: goto L3016;
    case 03112u: goto L3112;
    case 03123u: goto L3123;
    case 03152u: goto L3152;
    case 03670u: goto L3670;
    default: return CM_ENTRY_DAP_FAULT;
    }
/* Bank 20. */
L20_3612:                                            /* CM/DAPON */
    ai_caf(m, 03000u);                               /* EBAOG */
    ai_ts(m, EBANK);
    LEAVE(DOWNFLAG, 03615u);                         /* TC DOWNFLAG; ADRES DAPBIT1 */
L20_3616:
    LEAVE(DOWNFLAG, 03617u);                         /* TC DOWNFLAG; ADRES DAPBIT2 */
L20_3620:
    ai_dcaf(m, t5idler1);                            /* EXTEND; DCA T5IDLER1 */
    ai_dxch(m, T5LOC);
    ai_dcaf(m, t5idler1);                            /* EXTEND; DCA T5IDLER1 */
    ai_dxch(m, T6LOC);
    LEAVE(FLUSHJET, 03627u);                         /* TC FLUSHJET */
L20_3627:
    ai_csf(m, 070000u);                              /* 13,14,15 */
    ai_mask(m, DAPDATR1);
    ai_ts(m, DAPDATR1);
    ai_tc(m, 03633u);                                /* TC +4 */
    goto L20_3636;
L20_3633:                                            /* NOTYET */
    ai_caf(m, 062u);                                 /* .5SEC */
    LEAVE(BANKCALL, 03635u);                         /* TC BANKCALL; CADR DELAYJOB */
L20_3636:
    ai_caf(m, 02000u);                               /* BIT11 */
    ai_mask(m, CM_FLAGS);
    if (ai_bzf(m)) goto L20_3633;                    /* EXTEND; BZF NOTYET */
    ai_csf(m, 01u);                                  /* ONE */
    ai_ts(m, RCSFLAGS);
    ai_ts(m, P63FLAG);
    ai_ca(m, 7u);
    ai_ts(m, JETAG);
    ai_ts(m, PAXERR1);
    ai_dca(m, ALFA180);                              /* INHINT; EXTEND */
    ai_dxch(m, ALFACOM);
    ai_ca(m, ROLL180);
    ai_ts(m, ROLLHOLD);
    ai_mpf(m, 020000u);                              /* EXTEND; MP HALF */
    ai_ts(m, ROLLC);
    ai_cs(m, CM_FLAGS);
    ai_maskf(m, 04000u);                             /* BIT12 */
    ai_ads(m, CM_FLAGS);
    ai_cs(m, FLAGWRD2);
    ai_maskf(m, 01u);                                /* BIT1 */
    ai_ads(m, FLAGWRD2);
    LEAVE(POSTJUMP, 03671u);                         /* RELINT; TC POSTJUMP; CADR P62.1 */
L20_3672:                                            /* CM/DAPIC */
    ai_caf(m, 03000u);                               /* EBAOG */
    ai_ts(m, EBANK);
    ai_cs(m, PIPTIME + 1u);                          /* INHINT; CM/DAP2C */
    ai_ts(m, JETEM);
    ai_caf(m, 020000u);                              /* POS1/2 */
    ai_adf(m, 020000u);                              /* POS1/2 */
    ai_ad(m, TIME1);
    ai_ads(m, JETEM);
    ai_csf(m, 05u);                                  /* FIVE */
    ai_ad(m, JETEM);
L20_3705:
    ccs = ai_ccs(m, 0u);                             /* CCS A */
    if (ccs == 0u) {
        ai_adf(m, 077766u);                          /* -CDUT+1 */
        goto L20_3705;                               /* TCF -2 */
    }
    /* +0 would spin on TCF -2: A = -5 + JETEM and A - 10 are never +0. */
    if (ccs == 1u) return CM_ENTRY_DAP_FAULT;
    ai_adf(m, 01u);                                  /* NOOP; ONE */
    if (ai_ts(m, CM_GYMDT))                          /* overflow skips TC WAITLIST: the 2CADR's */
        LEAVE(02434u, 03715u);                       /* first word runs as TC 2434 in bank 20 */
    LEAVE(WAITLIST, 03714u);                         /* TC WAITLIST; 2CADR READGYMB */
L20_3716:
    ai_csf(m, 016017u);                              /* CM/SWIC1 */
    ai_mask(m, CM_FLAGS);
    ai_adf(m, 012u);                                 /* CM/SWIC2 */
    ai_ts(m, CM_FLAGS);
    ai_ca(m, 7u);
    ai_ts(m, BETA180);
    ai_caf(m, 01u);                                  /* ONE */
    ai_ts(m, SW_NDX);
    LEAVE(TWOPHSCHNG, 03727u);                       /* TC 2PHSCHNG; OCT 40116, 05024, 13000 */
L20_3732:
    LEAVE(POSTJUMP, 03733u);                         /* TC POSTJUMP; CADR P62.2 */
}
