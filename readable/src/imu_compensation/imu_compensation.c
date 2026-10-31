/* SPDX-License-Identifier: MIT */
/* IMU_COMPENSATION_PACKAGE.agc, both missions (Luminary099 pages 326-337,
 * Comanche055 pages 297-306): see imu_compensation.h. The faithful C is
 * c64-port/src/imu_compensation.c.
 *
 * The listing's scaling comments are kept: "(GYRO PULSES) X 2(+14)" is a
 * count of gyro pulses with its binary point 14 bits up, so a word of
 * GCOMP holds whole pulses and the word after it the fraction. The two
 * missions scale some coefficients differently, so their shifts (MP BITn)
 * differ; the LM's lines come first in each #if.
 *
 * Each call out of the file (NOVAC, BANKCALL, IBNKCALL) ends a function
 * with Q and Z set, and the word the call returns to is an entry of its
 * own. A subroutine of the file returns as the listing's does, through
 * the address its caller's TC left: TC Q (Q becomes 3), TC MPAC +2 or TC
 * BUF +1 (Q becomes the word after it). */
#include "imu_compensation.h"
#include "../agc/names.h"

static void irigcomp(void);
static void irig1(void);
static void nbd2(void);

/* ---- Helpers ---- */

/* INDEX BUF before an instruction on GCOMP or GCOMP +1: the gyro's double
 * word. BUF holds 0, 2 or 4 (X, Y, Z), or the -0 of 1 + NEG1, which
 * indexes as 0. */
static unsigned gcomp_of_buf(unsigned offset)
{
    return indexed(ADDRESS_OF(GCOMP) + offset, BUF(0));
}

/* The end of each of the file's loops over the axes (1/PIPA1, 1/GYRO,
 * GCOMP1): CCS <counter>; AD NEG1; TCF <loop> +1, the TS of the next
 * axis's index. The counter goes 4, 2, then 1 + NEG1, which is -0, and
 * CCS of -0 skips to the code after the loop. True for another pass.
 *
 * Never, so not modelled, as the loop sets the counter itself: +0 (TCF
 * <loop> +1 with +0, for ever) and negative ("LESS THAN ZERO IMPOSSIBLE":
 * the word after the TCF). */
static bool another_axis(unsigned counter)
{
    switch (ccs_at(counter)) {
    case POSITIVE: ad(NEG1); return true;
    case IS_PLUS_ZERO: break;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: return false;
    }
    not_modelled();
    return false;
}

/* ---- 1/PIPA: the accelerometers' errors, then the gyros' ---- */

/* 1/PIPA1 -- LM p. 326, CM p. 297.
 * Each PIPA count, Z, Y then X: "(PIPAI) + (PIPAI)(SFE) - (BIAS)(DELTAT)".
 * The count times the scale factor error is added to it, its minor part
 * kept as the fraction in DELVX +1 (the count's word after it); then the
 * bias times DELTAT is taken off. DELTAT is 1/PIPADT: while SERVICER runs,
 * its 2 seconds between readings (LASTBIAS sets it).
 * TS Q and CA Q: Q holds the major part a moment. */
static void one_pipa1(void)
{
    word axis;

    set_A(FOUR); /* PIPAZ, PIPAY, PIPAX */
    do {
        ts(ADDRESS_OF(BUF) + 2u);
        axis = BUF(2);
        ca(indexed(ADDRESS_OF(PIPASCF), axis));    /* (P.P.M.) X 2(-9) */
        mp_at(indexed(ADDRESS_OF(DELVX), axis));   /* (PP) X 2(+14) NOW (PIPA PULSES) X 2(+5) */
        ts(Q_ADDRESS);                             /* SAVE MAJOR PART */
        ca(L_ADDRESS);                             /* MINOR PART */
        mp(BIT6);                                  /* SCALE 2(+9)   SHIFT RIGHT 9 */
        ts(indexed(ADDRESS_OF(DELVX) + 1u, axis)); /* FRACTIONAL PIPA PULSES SCALED 2(+14) */
        ca(Q_ADDRESS);                             /* MAJOR PART */
        mp(BIT6);                                  /* SCALE 2(+9) SHIFT RIGHT 9 */
        das(indexed(ADDRESS_OF(DELVX), axis));     /* (PIPAI) + (PIPAI)(SFE) */
        cs(indexed(ADDRESS_OF(PIPABIAS), axis));   /* (PIPA PULSES)/(CS) */
        mp_at(ADDRESS_OF(ONE_PIPADT));             /* (CS) X 2(+8) */
#if LUMINARY099
        mp(BIT4); /* SCALE 2(+11)  SHIFT RIGHT 11 */
#else
        mp(BIT1); /* SCALE 2(+14) SHIFT RIGHT 14 */
#endif
        das(indexed(ADDRESS_OF(DELVX), axis)); /* - (BIAS)(DELTAT) */
    } while (another_axis(ADDRESS_OF(BUF) + 2u));
}

/* 1/PIPA -- LM p. 326, CM p. 297.
 * Compensate the PIPA counts SERVICER has just read, then add the gyros'
 * drift since the last time to GCOMP (IRIGCOMP). Called through BANKCALL,
 * it returns through SWRETURN. Its erasable is in E-bank 3, so it saves
 * the caller's EBANK in MODE ("SAVE EBANK OF CALLING PROGRAM"; IRIG1 puts
 * it back). "BYPASS IF GCOMPSW NEGATIVE": GCOMPSW negative turns the
 * compensation off.
 *
 * The CM compensates the counts with interrupts inhibited, to "ASSURE
 * COMPLETE COMPENSATION OF DELV'S FOR DOWNLINK". */
void one_pipa(void)
{
    set_A(LGCOMP_WORD);
    xch(EBANK_ADDRESS);
    ts(ADDRESS_OF(MODE));
    switch (ccs_at(ADDRESS_OF(GCOMPSW))) {
    case POSITIVE:            /* TCF +3 */
    case IS_PLUS_ZERO: break; /* TCF +2 */
    case NEGATIVE: irig1(); return;
    case IS_MINUS_ZERO: break;
    }
#if COMANCHE055
    inhint();
#endif
    one_pipa1();
#if COMANCHE055
    relint();
#endif
    irigcomp();
}

/* GCOMPSUB -- LM p. 331, CM p. 301.
 * One acceleration term of a gyro's drift: the coefficient (in A) times
 * the PIPA count (in MPAC, MPAC +1), added to the gyro's GCOMP (BUF's).
 * "ADIA OR ADSRA COEFFICIENT ARRIVES IN A". TC Q returns. */
static void gcompsub(void)
{
    xch(ADDRESS_OF(MPAC));
    mp_at(ADDRESS_OF(MPAC));
    dxch(ADDRESS_OF(VBUF)); /* NOW = (GYRO PULSES) X 2(+8) (CM 2(+11)) */

    ca(ADDRESS_OF(MPAC) + 1u); /* MINOR PART OF PIPA PULSES */
    mp_at(ADDRESS_OF(MPAC));   /* ADIA OR ADSRA */
    ts(L_ADDRESS);
    set_A(ZERO);
    das(ADDRESS_OF(VBUF));

    ca(ADDRESS_OF(VBUF)); /* PARTIAL RESULT - MAJOR */
#if LUMINARY099
    mp(BIT9); /* SCALE 2(+6)	   SHIFT RIGHT */
#else
    mp(BIT12); /* SCALE 2(+3)	SHIFT RIGHT 3 */
#endif
    das(gcomp_of_buf(0)); /* HI(ADIA)(PIPAI)  OR  HI(ADSRA)(PIPAI) */

    ca(ADDRESS_OF(VBUF) + 1u); /* PARTIAL RESULT - MINOR */
#if LUMINARY099
    mp(BIT9);
#else
    mp(BIT12);
#endif
    ts(L_ADDRESS);
    set_A(ZERO);
    das(gcomp_of_buf(0)); /* (ADIA)(PIPAI) OR (ADSRA)(PIPAI) */

    tc_through(Q_ADDRESS); /* TC Q */
}

/* One acceleration term, as IRIGX, IRIGY and IRIGZ write each: EXTEND;
 * DCS <PIPA count>; DXCH MPAC; CA (or CS) <coefficient>; TC GCOMPSUB,
 * the TC at `at`. DCS: the count's negative. */
static void acceleration_term(unsigned pipa, void (*load)(unsigned), unsigned coefficient, word at)
{
    dcs(pipa);
    dxch(ADDRESS_OF(MPAC));
    load(coefficient);
    tc_at(at);
    gcompsub();
}

static void driftsub(void);

/* IRIGX, IRIGY, IRIGZ -- LM pp. 329-330, CM pp. 299-300.
 * Each gyro's two acceleration terms: its input axis's (ADIA) and its
 * spin axis's (ADSRA). The listing keeps a third, the output axis's
 * (ADOA), commented out ("***").
 *
 * The LM's are subroutines ("SAVE Q" in MPAC +2; TC MPAC +2 returns), as
 * NBD3 calls them too. The CM's fall through each other, each with its
 * gyro's bias drift after it. */
#if LUMINARY099
static void irigx(void)
{
    qxch(ADDRESS_OF(MPAC) + 2u);                                             /* SAVE Q */
    acceleration_term(ADDRESS_OF(DELVX), ca, ADDRESS_OF(ADIAX), IRIGX + 6u); /* -(ADIAX)(PIPAX) */
    acceleration_term(ADDRESS_OF(DELVY), cs, ADDRESS_OF(ADSRAX),
                      IRIGX + 013u);   /* +(ADSRAX)(PIPAY) */
    tc_through(ADDRESS_OF(MPAC) + 2u); /* TC MPAC +2 */
}

static void irigy(void)
{
    qxch(ADDRESS_OF(MPAC) + 2u);                                             /* SAVE Q */
    acceleration_term(ADDRESS_OF(DELVY), ca, ADDRESS_OF(ADIAY), IRIGY + 6u); /* -(ADIAY)(PIPAY) */
    acceleration_term(ADDRESS_OF(DELVZ), cs, ADDRESS_OF(ADSRAY),
                      IRIGY + 013u);   /* +(ADSRAY)(PIPAZ) */
    tc_through(ADDRESS_OF(MPAC) + 2u); /* TC MPAC +2 */
}

static void irigz(void)
{
    qxch(ADDRESS_OF(MPAC) + 2u);                                              /* SAVE Q */
    acceleration_term(ADDRESS_OF(DELVY), ca, ADDRESS_OF(ADSRAZ), IRIGZ + 6u); /* -(ADSRAZ)(PIPAY) */
    acceleration_term(ADDRESS_OF(DELVZ), ca, ADDRESS_OF(ADIAZ), IRIGZ + 013u); /* -(ADIAZ)(PIPAZ) */
    tc_through(ADDRESS_OF(MPAC) + 2u);                                         /* TC MPAC +2 */
}
#else
static void irigx(void)
{
    acceleration_term(ADDRESS_OF(DELVX), ca, ADDRESS_OF(ADIAX), IRIGX + 4u); /* -(ADIAX)(PIPAX) */
    acceleration_term(ADDRESS_OF(DELVY), cs, ADDRESS_OF(ADSRAX),
                      IRIGX + 011u); /* +(ADSRAX)(PIPAY) */
    cs(ADDRESS_OF(NBDX));            /* (GYRO PULSES)/(CS) X 2(-5) */
    tc_at(IRIGX + 013u);
    driftsub(); /* -(NBDX)(DELTAT) */
}

static void irigy(void)
{
    acceleration_term(ADDRESS_OF(DELVY), ca, ADDRESS_OF(ADIAY), IRIGY + 4u); /* -(ADIAY)(PIPAY) */
    acceleration_term(ADDRESS_OF(DELVZ), cs, ADDRESS_OF(ADSRAY),
                      IRIGY + 011u); /* +(ADSRAY)(PIPAZ) */
    cs(ADDRESS_OF(NBDY));
    tc_at(IRIGY + 013u);
    driftsub(); /* -(NBDY)(DELTAT) */
}

static void irigz(void)
{
    acceleration_term(ADDRESS_OF(DELVY), ca, ADDRESS_OF(ADSRAZ), IRIGZ + 4u); /* -(ADSRAZ)(PIPAY) */
    acceleration_term(ADDRESS_OF(DELVZ), ca, ADDRESS_OF(ADIAZ), IRIGZ + 011u); /* -(ADIAZ)(PIPAZ) */
    ca(ADDRESS_OF(NBDZ));
    tc_at(IRIGZ + 013u);
    driftsub(); /* +(NBDZ)(DELTAT) */
}
#endif

/* DRFTSUB2's MASK of DEC -1: the LM's COMPCHK, the CM's NEGONE. */
static void mask_minus_one(void)
{
#if LUMINARY099
    mask(COMPCHK);
#else
    mask(NEGONE);
#endif
}

/* DRFTSUB2 -- LM p. 332, CM p. 302.
 * On to the next gyro (BUF += 2), and look at the one just done: "ARE
 * GYRO COMMANDS GREATER THAN 2 PULSES" -- then "SET GCOMPSW POSITIVE".
 * CCS of the gyro's whole pulses leaves one less (+0 for one pulse);
 * MASK -1 clears its lowest bit, so one or two pulses leave +0. TC BUF +1
 * returns.
 *
 * CCS GCOMP's four ways: + and - (TCF +2, the MASK), +0 (TC BUF +1),
 * -0 (the CCS A, with +0). A is never negative at the CCS A: CCS leaves
 * +0 or more and the MASK keeps it so. */
static void drftsub2(void)
{
    set_A(TWO); /* PIPAX, PIPAY, PIPAZ */
    ad_at(ADDRESS_OF(BUF));
    xch(ADDRESS_OF(BUF));
    switch (ccs_at(indexed(ADDRESS_OF(GCOMP), a_word()))) {      /* INDEX A: the old BUF */
    case POSITIVE: mask_minus_one(); break;                      /* YES: TCF +2 */
    case IS_PLUS_ZERO: tc_through(ADDRESS_OF(BUF) + 1u); return; /* NO */
    case NEGATIVE: mask_minus_one(); break;
    case IS_MINUS_ZERO: break;
    }
    if (ccs_at(A_ADDRESS) == POSITIVE) ts(ADDRESS_OF(GCOMPSW)); /* YES - SET GCOMPSW POSITIVE */
    tc_through(ADDRESS_OF(BUF) + 1u);                           /* TC BUF +1 */
}

/* DRIFTSUB -- LM p. 332, CM p. 302.
 * A gyro's bias drift for 1/PIPA: the coefficient NBD (in A) times
 * DELTAT (1/PIPADT, SERVICER's 2 seconds), added to its GCOMP; then
 * DRFTSUB2. Its return in BUF +1. */
static void driftsub(void)
{
    qxch(ADDRESS_OF(BUF) + 1u);
    mp_at(ADDRESS_OF(ONE_PIPADT)); /* (CS) X 2(+8)	 NOW (GYRO PULSES) X 2(+3) */
    lxch(ADDRESS_OF(MPAC) + 1u);   /* SAVE FOR FRACTIONAL COMPENSATION */
    mp(BIT4);                      /* SCALE 2(+11)	   SHIFT RIGHT 11 */
    das(gcomp_of_buf(0));          /* HI(NBD)(DELTAT)   (GYRO PULSES) X 2(+14) */
    ca(ADDRESS_OF(MPAC) + 1u);     /* NOW MINOR PART */
    mp(BIT4);
    ts(L_ADDRESS);
    set_A(ZERO);
    das(gcomp_of_buf(0)); /* (NBD)(DELTAT)     (GYRO PULSES) X 2(+14) */
    drftsub2();
}

/* IRIGCOMP -- LM p. 328, CM pp. 299-301.
 * The gyros' drift, X, Y then Z: each one's acceleration terms (IRIGX,
 * IRIGY, IRIGZ) and its bias drift (DRIFTSUB). Then "ARE GYRO COMMANDS
 * GREATER THAN 2 PULSES": if so, start the job 1/GYRO to "SEND OUT GYRO
 * TORQUING COMMANDS", at PRIO21, "PRIO GREATER THAN SERVICER" (TC NOVAC,
 * 2CADR 1/GYRO). Either way, back to the caller (IRIG1).
 *
 * A is +0 here (1/PIPA1's CCS of -0): GCOMPSW and BUF start at +0. CCS
 * GCOMPSW: + (TCF +2) and - start the job, +0 returns, -0 lands on the
 * TC NOVAC with A +0; GCOMPSW is never negative here. */
static void irigcomp(void)
{
    ts(ADDRESS_OF(GCOMPSW)); /* INDICATE COMMANDS 2 PULSES OR LESS. */
    ts(ADDRESS_OF(BUF));     /* INDEX COUNTER .  IRIGX, IRIGY, IRIGZ. */
#if LUMINARY099
    tc_at(IRIGCOMP + 2u);
    irigx();              /* COMPENSATE ACCELERATION TERMS */
    cs(ADDRESS_OF(NBDX)); /* (GYRO PULSES)/(CS) X 2(-5) */
    tc_at(IRIGCOMP + 4u);
    driftsub(); /* -(NBOX)(DELTAT)   (GYRO PULSES) X 2(+14) */
    tc_at(IRIGCOMP + 5u);
    irigy();
    cs(ADDRESS_OF(NBDY));
    tc_at(IRIGCOMP + 7u);
    driftsub(); /* -(NBDY)(DELTAT) */
    tc_at(IRIGCOMP + 010u);
    irigz();
    ca(ADDRESS_OF(NBDZ));
    tc_at(IRIGCOMP + 012u);
    driftsub(); /* +(NBDZ)(DELTAT) */
#else
    irigx();
    irigy();
    irigz();
#endif
    switch (ccs_at(ADDRESS_OF(GCOMPSW))) {
    case POSITIVE: set_A(PRIO21); break; /* YES: TCF +2; PRIO GREATER THAN SERVICER */
    case IS_PLUS_ZERO: irig1(); return;  /* NO  RETURN */
    case NEGATIVE: set_A(PRIO21); break;
    case IS_MINUS_ZERO: break;
    }
    tc_at(IRIG1 - 4u); /* TC NOVAC; 2CADR 1/GYRO */
    continue_at(NOVAC);
}

/* The RELINT before IRIG1 -- LM p. 328, CM p. 301: where NOVAC returns
 * (the 1/GYRO job started), with interrupts inhibited. */
void irigcomp_after_novac(void)
{
    relint();
    irig1();
}

/* IRIG1 -- LM p. 328, CM p. 301. "RESTORE CALLERS EBANK" and return to
 * 1/PIPA's caller (or GCOMPZER's). */
static void irig1(void)
{
    ca(ADDRESS_OF(MODE));
    ts(EBANK_ADDRESS);
    continue_at(SWRETURN);
}

/* ---- 1/GYRO: the pulses to the gyros ---- */

/* 1/GYRO -- LM p. 333, CM p. 303.
 * The job that sends GCOMP's whole pulses to the gyros. "SCALE GYRO
 * COMMANDS FOR IMUPULSE": each gyro's double word shifts right 7
 * (MP BIT8), so its whole pulses (bits 8-14 of the first word, and the
 * second word's) move into the second word, which IMUPULSE sends, and the
 * fraction left behind stays in the first word for next time. Then
 * IMUPULSE takes GCOMP (A, LGCOMP: its E-bank and address) and IMUSTALL
 * waits "FOR PULSES TO GET OUT" (TC BANKCALL with each CADR after it).
 *
 * LGCOMP, the ECADR, sits after the loop's TCF, the word a negative count
 * would run. */
void one_gyro(void)
{
    set_A(FOUR); /* PIPAZ, PIPAY, PIPAX */
    do {
        ts(ADDRESS_OF(BUF));
        ca(gcomp_of_buf(1u)); /* FRACTIONAL PULSES */
        mp(BIT8);             /* SHIFT RIGHT 7 */
        ts(gcomp_of_buf(1u)); /* FRACTIONAL PULSES SCALED */
        set_A(ZERO);          /* SET GCOMP = 0 FOR DAS INSTRUCTION */
        xch(gcomp_of_buf(0)); /* GYRO PULSES */
        mp(BIT8);             /* SHIFT RIGHT 7 */
        das(gcomp_of_buf(0)); /* ADD THESE TO FRACTIONAL PULSES ABOVE */
    } while (another_axis(ADDRESS_OF(BUF)));
    set_A(LGCOMP_WORD);
    tc_at(LGCOMP + 2u); /* TC BANKCALL; CADR IMUPULSE */
    continue_at(BANKCALL);
}

/* LGCOMP +4, IMUPULSE's return. "WAIT FOR PULSES TO GET OUT". */
void one_gyro_after_imupulse(void)
{
    tc_at(LGCOMP + 4u); /* TC BANKCALL; CADR IMUSTALL */
    continue_at(BANKCALL);
}

/* LGCOMP +6, IMUSTALL's return: TCF ENDOFJOB ("TEMPORARY"). */
void one_gyro_after_imustall(void)
{
    continue_at(ENDOFJOB);
}

/* GCOMP1 -- LM p. 333, CM p. 303.
 * "RESCALE": each gyro's fraction (GCOMP +1) shifted left 7 into L ("BITS
 * 8-14 OF MINOR PART WERE = 0") and stored back, then TCF ENDOFJOB. No
 * routine of either program calls it. The word after the loop's TCF is
 * the LM's COMPCHK (DEC -1, DRFTSUB2's MASK) and the CM's V06N30S. */
void gcomp1(void)
{
    set_A(FOUR); /* PIPAZ, PIPAY, PIPAX */
    do {
        ts(ADDRESS_OF(BUF));
        ca(gcomp_of_buf(1u));
        mp(BIT8); /* SHIFT MINOR PART LEFT 7 - MAJOR PART = 0 */
        lxch(gcomp_of_buf(1u));
    } while (another_axis(ADDRESS_OF(BUF)));
    continue_at(ENDOFJOB);
}

/* ---- NBDONLY and LASTBIAS: the bias drift in free fall ---- */

/* FBIASSUB -- LM p. 336, CM p. 306.
 * DRIFTSUB for NBD3: a gyro's bias drift, the coefficient NBD (in A)
 * times DELTAT (in MPAC, MPAC +1), added to its GCOMP; then DRFTSUB2.
 * XCH Q; TS BUF +1: the return in BUF +1, the coefficient in Q. */
static void fbiassub(void)
{
    xch(Q_ADDRESS);
    ts(ADDRESS_OF(BUF) + 1u);
    ca(Q_ADDRESS);           /* NBD SCALED (GYRO PULSES)/(CS) X 2(-5) */
    mp_at(ADDRESS_OF(MPAC)); /* DELTAT SCALED (CS) X 2(+19) */
    das(gcomp_of_buf(0));    /* HI(NBD)(DELTAT)   (GYRO PULSES) X 2(+14) */
    ca(Q_ADDRESS);           /* NOW FRACTIONAL PART */
    mp_at(ADDRESS_OF(MPAC) + 1u);
    ts(L_ADDRESS);
    set_A(ZERO);
    das(gcomp_of_buf(0)); /* (NBD)(DELTAT)     (GYRO PULSES) X 2(+14) */
    drftsub2();           /* CHECK MAGNITUDE OF COMPENSATION */
}

/* NBD3 -- LM pp. 334-335, CM pp. 304-305.
 * Each gyro's bias drift over DELTAT (in A, centiseconds): DELTAT shifted
 * right 5 into VBUF +2 (CM VBUF) and MPAC, then FBIASSUB for each gyro,
 * with the signs the listing's comments give: -(NBDX)(DELTAT) by CS NBDX,
 * -(NBDY)(DELTAT) by DCS of DELTAT, +(NBDZ)(DELTAT) by both. Then "ARE
 * GYRO COMMANDS GREATER THAN 2 PULSES": TCF 1/GYRO, else TCF ENDOFJOB.
 *
 * The LM on the lunar surface ("IF SURFACE FLAG IS SET", TEM1 BIT8) does
 * each gyro's acceleration terms first (TC IRIGX). TEM1 is +0 or BIT8,
 * as its callers leave it. GCOMPSW is +0 here, or positive from DRFTSUB2. */
static void nbd3(void)
{
#if LUMINARY099
    mp(BIT10); /* SHIFT RIGHT 5 */
    dxch(ADDRESS_OF(VBUF) + 2u);
    set_A(ZERO);
    ts(ADDRESS_OF(GCOMPSW));                    /* INDICATE COMMANDS 2 PULSES OR LESS. */
    ts(ADDRESS_OF(BUF));                        /* INDEX  X, Y, Z. */
    if (ccs_at(ADDRESS_OF(TEM1)) == POSITIVE) { /* IF SURFACE FLAG IS SET, */
        tc_at(NBD3 + 7u);
        irigx(); /* COMPENSATE ACCELERATION TERMS. */
    }
    dca(ADDRESS_OF(VBUF) + 2u);
    dxch(ADDRESS_OF(MPAC)); /* DELTAT NOW SCALED (CS) X 2(+19) */
    cs(ADDRESS_OF(NBDX));
    tc_at(NBD3 + 014u);
    fbiassub(); /* -(NBDX)(DELTAT)   (GYRO PULSES) X 2(+14) */
    if (ccs_at(ADDRESS_OF(TEM1)) == POSITIVE) {
        tc_at(NBD3 + 016u);
        irigy();
    }
    dcs(ADDRESS_OF(VBUF) + 2u);
    dxch(ADDRESS_OF(MPAC)); /* DELTAT SCALED (CS) X 2(+19) */
    ca(ADDRESS_OF(NBDY));
    tc_at(NBD3 + 023u);
    fbiassub(); /* -(NBDY)(DELTAT) */
    if (ccs_at(ADDRESS_OF(TEM1)) == POSITIVE) {
        tc_at(NBD3 + 025u);
        irigz();
    }
    dcs(ADDRESS_OF(VBUF) + 2u);
    dxch(ADDRESS_OF(MPAC));
    cs(ADDRESS_OF(NBDZ));
    tc_at(NBD3 + 032u);
    fbiassub(); /* +(NBDZ)(DELTAT) */
#else
    mp(BIT10); /* SHIFT RIGHT 5 */
    dxch(ADDRESS_OF(VBUF));
    dca(ADDRESS_OF(VBUF));
    dxch(ADDRESS_OF(MPAC)); /* DELTAT NOW SCALED (CS) X 2(+19) */
    set_A(ZERO);
    ts(ADDRESS_OF(GCOMPSW)); /* INDICATE COMMANDS 2 PULSES OR LESS */
    ts(ADDRESS_OF(BUF));     /* PIPAX, PIPAY, PIPAZ */
    cs(ADDRESS_OF(NBDX));
    tc_at(NBD3 + 012u);
    fbiassub(); /* -(NBOX)(DELTAT) */
    dcs(ADDRESS_OF(VBUF));
    dxch(ADDRESS_OF(MPAC));
    ca(ADDRESS_OF(NBDY));
    tc_at(NBD3 + 017u);
    fbiassub(); /* -(NBDY)(DELTAT) */
    dcs(ADDRESS_OF(VBUF));
    dxch(ADDRESS_OF(MPAC));
    cs(ADDRESS_OF(NBDZ));
    tc_at(NBD3 + 024u);
    fbiassub(); /* +(NBDZ)(DELTAT) */
#endif
    if (ccs_at(ADDRESS_OF(GCOMPSW)) == POSITIVE)
        one_gyro(); /* YES: TCF 1/GYRO */
    else
        continue_at(ENDOFJOB); /* NO */
}

/* NBD2 -- LM p. 334, CM p. 304.
 * DELTAT, the time since the last compensation, from the difference of
 * two times in A. TIME1 counts centiseconds and wraps past 2^14.
 *
 * The LM adds HALF twice: a difference that crossed a wrap comes out
 * right, and one that did not overflows, which XCH L twice removes ("IF
 * TIME1 DID NOT TICK, REMOVE RESULTING OVERFLOW"). The CM takes the
 * difference's magnitude with CCS: + (AD ONE, "NO TIME1 OVERFLOW"), +0,
 * - ("TIME1 OVERFLOW": COM; AD POSMAX), and -0 ("IF ELAPSED TIME = 0",
 * TCF ENDOFJOB). */
static void nbd2(void)
{
#if LUMINARY099
    ad(HALF); /* CORRECT FOR POSSIBLE TIME1 TICK */
    ad(HALF);
    xch(L_ADDRESS); /* IF TIME1 DID NOT TICK, REMOVE RESULTING */
    xch(L_ADDRESS); /* OVERFLOW. */
#else
    switch (ccs_at(A_ADDRESS)) {   /* CALCULATE ELAPSED TIME */
    case POSITIVE: ad(ONE); break; /* NO TIME1 OVERFLOW: TCF NBD3 */
    case IS_PLUS_ZERO: break;      /* TCF NBD3 */
    case NEGATIVE:                 /* TIME1 OVERFLOW: TCF +2 */
        com();                     /* CALCULATE ABSOLUTE DIFFERENCE */
        ad(POSMAX);
        break;
    case IS_MINUS_ZERO: continue_at(ENDOFJOB); return; /* IF ELAPSED TIME = 0 */
    }
#endif
    nbd3();
}

/* CA TIME1; XCH 1/PIPADT; RELINT; COM; AD 1/PIPADT, then NBD2: the time
 * now becomes the last compensation's, and A the "PRESENT TIME - PREVIOUS
 * TIME". */
static void nbdonly_deltat(void)
{
    ca(ADDRESS_OF(TIME1));       /* (CS) X 2(+14) */
    xch(ADDRESS_OF(ONE_PIPADT)); /* PREVIOUS TIME */
    relint();
    com();
    ad_at(ADDRESS_OF(ONE_PIPADT)); /* PRESENT TIME - PREVIOUS TIME */
    nbd2();
}

/* NBDONLY -- LM p. 334, CM p. 304.
 * The free-fall job: "COMPENSATE FOR NBD COEFFICIENTS ONLY". "BYPASS IF
 * GCOMPSW NEGATIVE"; ends too unless the drift flag (FLAGWRD2's bit 15,
 * the word negative) is still set, with interrupts inhibited: "PREREAD
 * T3RUPT MAY COINCIDE" (SERVICER may have just taken over).
 *
 * On the lunar surface the LM reads the PIPAs first, for the acceleration
 * terms: TC IBNKCALL; CADR PIPASR +3 ("READ PIPAS, BUT DO NOT SCALE
 * THEM"), whose return is NBDONLY +020. */
void nbdonly(void)
{
    switch (ccs_at(ADDRESS_OF(GCOMPSW))) {
    case POSITIVE:            /* TCF +3 */
    case IS_PLUS_ZERO: break; /* TCF +2 */
    case NEGATIVE: continue_at(ENDOFJOB); return;
    case IS_MINUS_ZERO: break;
    }
    inhint();
    switch (ccs_at(ADDRESS_OF(FLAGWRD2))) {
    case POSITIVE:
    case IS_PLUS_ZERO: continue_at(ENDOFJOB); return;
    case NEGATIVE: /* TCF +1 */
    case IS_MINUS_ZERO: break;
    }
#if LUMINARY099
    ca(ADDRESS_OF(FLAGWRD8));  /* IF SURFACE FLAG IS SET, SET TEM1 */
    mask(BIT8);                /* POSITIVE SO THAT THE ACCELERATION TERMS */
    ts(ADDRESS_OF(TEM1));      /* WILL BE COMPENSATED. */
    if (!A_is_zero()) {        /* EXTEND; BZF +3: ARE WE ON THE SURFACE */
        tc_at(NBDONLY + 016u); /* TC IBNKCALL; CADR PIPASR +3 */
        continue_at(IBNKCALL);
        return;
    }
#endif
    nbdonly_deltat();
}

#if LUMINARY099
/* NBDONLY +020, PIPASR +3's return on the surface. */
void nbdonly_after_pipasr(void)
{
    nbdonly_deltat();
}
#endif

/* LASTBIAS -- LM p. 336, CM p. 306.
 * The bias drift up to the last PIPA reading in free fall, as SERVICER
 * starts: PIPUSE1 (the CM's PIPUSE) reads the PIPAs (TC BANKCALL with the
 * CADR after it), and its return is LASTBIAS +2. */
void lastbias(void)
{
    tc_at(LASTBIAS);
    continue_at(BANKCALL);
}

/* LASTBIAS +2. "BYPASS IF GCOMPSW NEGATIVE"; else DELTAT from the last
 * compensation (1/PIPADT) to the PIPA reading (PIPTIME +1, the CM's
 * PIPTIME1 +1), and NBD2. 1/PIPADT becomes PRIO31, "2 SECONDS SCALED (CS)
 * X 2(+8)": DELTAT for each of SERVICER's readings, from now on. */
void lastbias_after_pipuse(void)
{
    switch (ccs_at(ADDRESS_OF(GCOMPSW))) {
    case POSITIVE:            /* TCF +3 */
    case IS_PLUS_ZERO: break; /* TCF +2 */
    case NEGATIVE: continue_at(ENDOFJOB); return;
    case IS_MINUS_ZERO: break;
    }
#if LUMINARY099
    ca(ADDRESS_OF(FLAGWRD8)); /* IF SURFACE FLAG IS SET, SET TEM1 */
    mask(SURFFBIT);           /* POSITIVE SO THAT THE ACCELERATION TERMS */
    ts(ADDRESS_OF(TEM1));     /* WILL BE COMPENSATED. */
#endif
    set_A(PRIO31); /* 2 SECONDS SCALED (CS) X 2(+8) */
    xch(ADDRESS_OF(ONE_PIPADT));
    com();
#if LUMINARY099
    ad_at(ADDRESS_OF(PIPTIME) + 1u);
#else
    ad_at(ADDRESS_OF(PIPTIME1) + 1u); /* TIME AT PIPA1 =0 */
#endif
    nbd2();
}

#if LUMINARY099
/* GCOMPZER -- LM pp. 336-337.
 * "ROUTINE TO ZERO GCOMP BEFORE FIRST CALL TO 1/PIPA" (P51-P53): GCOMPSW
 * and the six words of GCOMP to +0, saving and restoring the caller's
 * EBANK as 1/PIPA does. The CM's IMU alignment zeroes them itself
 * ("GCOMPZER SUBROUTINE NO LONGER NEEDED"). */
void gcompzer(void)
{
    unsigned i;

    set_A(LGCOMP_WORD); /* ROUTINE TO ZERO GCOMP BEFORE FIRST */
    xch(EBANK_ADDRESS); /* CALL TO 1/PIPA */
    ts(ADDRESS_OF(MODE));
    set_A(ZERO);
    ts(ADDRESS_OF(GCOMPSW));
    for (i = 0; i < 6u; ++i) ts(ADDRESS_OF(GCOMP) + i);
    irig1(); /* RESTORE EBANK AND RETURN */
}
#endif
