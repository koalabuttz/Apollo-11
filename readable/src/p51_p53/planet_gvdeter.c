/* SPDX-License-Identifier: MIT */
/* P51-P53.agc, part 7 (Luminary099 pp. 961-966, bank 15): PLANET, the
 * direction of the body the crew sights; and GVDETER, the gravity vector
 * determination on the lunar surface, with LUNG, which reads the PIPAs
 * for it.
 *
 * PLANET is "TO PROVIDE THE REFERENCE VECTOR FOR THE SIGHTED CELESTIAL" body:
 * a star from the catalog, the sun, the earth or the moon from LOCSAM, or
 * a planet's vector the crew keys in. GVDETER "FINDS THE DIRECTION OF THE
 * MOON'S GRAVITY" for P57: LUNG reads the PIPAs at one orientation of the
 * IMU, then at the one turned 180 degrees about gravity, and the two
 * readings, averaged, are gravity in navigation base axes (STARSAV1).
 *
 * Each call out of the file (BANKCALL, DOWNFLAG, UPFLAG, PHASCHNG,
 * CURTAINS, GOTOPOOH, TWIDDLE, ENDOFJOB, IBNKCALL, FINDVAC, TASKOVER)
 * leaves Z at the routine and Q at the word after the TC, where the
 * routine comes back to; GOTOPOOH, ENDOFJOB and TASKOVER don't come back.
 * The faithful port keys a routine at each such return, at each TC's
 * return in the file, and at each TC INTPRET's NEWOPS.
 *
 * PROGRAV's end, TCF ATTCHK, goes on into P57's code: that key is written
 * with P57 (part 9). */
#include "p51_p53.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void planet_flash(void);
static void notplan(void);
static void calsam1(void);

/* ---- PLANET -- pp. 961-962 ---- */

/* PLANET: the time (in MPAC) to TSIGHT, STARAD cleared, and the caller's
 * return to GCTR. */
static const word planet_words[] = {
    /* PLANET */ STOVL(TSIGHT),
    ADDRESS_PLUS_ONE(ZEROVEC),
    STORE(STARAD),
    ORDERS(STQ, EXIT),
    ADDRESS(GCTR),
};

const struct interpretive_code PLANET_CODE = {015u, PLANET, COUNT(planet_words), planet_words};

/* PLANET +5, after the EXIT: the star code (AOTCODE's bits 1-6) times 6
 * (REVCNT) to BESTI (or BESTJ, by STARIND). Not 0: a star, or the sun,
 * the earth or the moon (NOTPLAN). Code 0, a planet: the crew keys in
 * its vector (VNPLANV, to STARAD). (A is L, a word: TS does not skip;
 * nor is it negative: CCS takes its first two ways only.) */
void planet_after_exit(void)
{
    set_A(negate(HIGH9)); /* CS HIGH9 */
    mask_at(ADDRESS_OF(AOTCODE));
    mp(REVCNT); /* EXTEND; MP REVCNT */
    xch(L_ADDRESS);
    ts(indexed_ts(ADDRESS_OF(BESTI), fetch(ADDRESS_OF(STARIND)))); /* INDEX STARIND */
    switch (ccs_register(A_ADDRESS)) {                             /* CCS A */
    case POSITIVE: notplan(); break;                               /* TCF NOTPLAN */
    case IS_PLUS_ZERO: planet_flash(); break;
    default: not_modelled();
    }
}

/* The planet's vector flashed (V06N88): terminate or enter, flash again;
 * proceed, on to its unit vector. */
static void planet_flash(void)
{
    set_A(VNPLANV);
    tc_to(PLANET + 017u, BANKCALL); /* CADR GOFLASH */
}

void planet_terminate(void)
{
    tc_at(PLANET + 021u); /* TC -3 */
    planet_flash();
}

void planet_proceed(void)
{
    tc_at(PLANET + 022u); /* TC +2 */
    planet_vector();
}

void planet_enter(void)
{
    tc_at(PLANET + 023u); /* TC -5 */
    planet_flash();
}

/* The planet's vector (STARAD) made a unit vector, back to the caller
 * (GCTR). */
void planet_vector(void)
{
    tc_at(PLANET + 024u);
    intpret();
}

static const word planet_vector_words[] = {
    /* PLANET +025 */ ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(STARAD),
    ORDER(GOTO),
    ADDRESS(GCTR),
};

const struct interpretive_code PLANET_VECTOR_CODE = {
    015u, PLANET + 025u, COUNT(planet_vector_words), planet_vector_words};

/* NOTPLAN: the code times 6, less one (as CCS leaves it), 227 or more
 * (code 46, 47 or 50: the sun, the earth or the moon), and LOCSAM
 * computes it (CALSAM1). Else a star: its place in the catalog (BESTI,
 * by STARIND) to X1, and its vector from the catalog (CATLOG,1). (A is a
 * word: TS does not skip.) */
static void notplan(void)
{
    cs(A_ADDRESS); /* CS A */
    ad(DEC227);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF CALSAM1 */
        calsam1();
        return;
    }
    ca(indexed_ca(ADDRESS_OF(BESTI), fetch(ADDRESS_OF(STARIND)))); /* INDEX STARIND */
    ts(indexed_ts(ADDRESS_OF(X1), FIXLOC));                        /* INDEX FIXLOC */
    tc_at(NOTPLAN + 010u);
    intpret();
}

static const word notplan_words[] = {
    /* NOTPLAN +011 */ ORDERS(VLOAD_STAR, GOTO),
    ADDRESS_PLUS_ONE_AT_X1(CATLOG, 0),
    ADDRESS(GCTR),
};

const struct interpretive_code NOTPLAN_CODE = {015u, NOTPLAN + 011u, COUNT(notplan_words),
                                               notplan_words};

static void calsam1(void)
{
    tc_at(CALSAM1);
    intpret();
}

/* CALSAM: the sun, the earth and the moon at TSIGHT (LOCSAM, to VEARTH,
 * VSUN and VMOON, STARAD's 18 words); VEARTH and VSUN trade places
 * (through 0D), so that the three are in their codes' order; then the
 * one the code picks: STARAD less 228 plus the code times 6 (BESTI, by
 * STARIND, to X1). */
static const word calsam_words[] = {
    /* CALSAM */ ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(TSIGHT),
    ADDRESS(LOCSAM),
    ORDERS(LXC_1, VLOAD),
    ADDRESS(STARIND),
    ADDRESS_PLUS_ONE(VEARTH),
    STOVL_D(0),
    ADDRESS_PLUS_ONE(VSUN),
    STOVL(VEARTH),
    ADDRESS_PLUS_ONE_D(0),
    STORE(VSUN),
    ORDERS(DLOAD_STAR, LXC_1),
    ADDRESS_PLUS_ONE_AT_X1(BESTI, 0),
    ADDRESS(MPAC),
    ORDERS(VLOAD_STAR, GOTO),
    ADDRESS_PLUS_ONE_AT_X1(STARAD, -228),
    ADDRESS(GCTR),
};

const struct interpretive_code CALSAM_CODE = {015u, CALSAM, COUNT(calsam_words), calsam_words};

/* ---- GVDETER -- pp. 963-965 ---- */

/* GVDETER: the first orientation's gimbal angles (42DEG, its complement
 * and 35DEG) to THETAD. (Each A is a constant: no TS skips.) */
void gvdeter(void)
{
    set_A(FORTY_TWO_DEG);
    ts(ADDRESS_OF(THETAD));
    com();
    ts(ADDRESS_OF(THETAD) + 1u);
    set_A(THIRTY_FIVE_DEG);
    ts(ADDRESS_OF(THETAD) + 2u);
    tc_at(GVDETER + 6u);
    intpret();
}

/* The IMU to THETAD, and gravity read there (LUNG, to GOUT). Then, for
 * each axis (X1 from 18D, by S1, 6), the navigation base's axis in the
 * present stable member (*NBSM*, to XNB) and the row of the stable
 * member turned 180 degrees about gravity: gravity (STAR) times its
 * component, less the axis ("OUTER PRODUCT", to XSM). Its gimbal angles
 * (CALCGA); half the first gravity to STARAD +12D, and gravity read
 * there (LUNG); the two halves' sum, a unit vector, to STARSAV1, and the
 * angle between it and the last (GSAV) to DSPTEM1. */
static const word gvdeter_words[] = {
    /* GVDETER +7 */ ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, REFSMFLG),
    ADDRESS(LUNG),
    ORDERS(AXT_1, SSP), /* "X1=18" */
    NUMBER(18),
    ADDRESS_PLUS_ONE(S1), /* "S1=6" */
    NUMBER(6),
    ORDER(LXC_2),
    ADDRESS(S1),
    /* GRAVEL */ ORDERS(VLOAD_STAR, CALL),
    ADDRESS_PLUS_ONE_AT_X2(XUNIT, -6),
    ADDRESS(STAR_NBSM_STAR), /* "SIN AND COS COMPUTED IN LUNG" */
    STORE_AT_X1(XNB, 18),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(STAR),
    ORDERS(LXC_2, VXSC_STAR), /* "COMPLEMENT -- UNITX ARE BACKWARD --" */
    ADDRESS(X2),
    ADDRESS_PLUS_ONE_AT_X2(STAR, 6), /* "OUTER PRODUCT" */
    ORDERS(VSL2, LXC_2),
    ADDRESS(X2),
    ORDERS(VSU_STAR, INCR_2),
    ADDRESS_PLUS_ONE_AT_X2(XUNIT, -6),
    NUMBER(2),
    STORE_AT_X1(XSM, 18),
    ORDERS(TIX_1, CALL),
    ADDRESS(GRAVEL),
    ADDRESS(CALCGA),
    ORDERS(VLOAD, VSR1),
    ADDRESS_PLUS_ONE(GOUT),
    STCALL_AT(STARAD, 12),
    ADDRESS(LUNG),
    ORDERS(VLOAD, VSR1),
    ADDRESS_PLUS_ONE(GOUT),
    ORDERS(VAD, UNIT),
    ADDRESS_PLUS_ONE_AT(STARAD, 12),
    STORE(STARSAV1),
    ORDER(DOT),
    ADDRESS_PLUS_ONE(GSAV),
    ORDERS(SL1, ACOS),
    STORE(DSPTEM1),
    ORDER(EXIT),
};

const struct interpretive_code GVDETER_CODE = {015u, GVDETER + 7u, COUNT(gvdeter_words),
                                               gvdeter_words};

/* After the EXIT: FREEFLAG cleared "IN CASE OF RECYCLE"; the angle
 * flashed (DISGRVER, V06N04): terminate, proceed (PROGRAV) or recycle
 * ("STORE GRAV AND DO IT AGAIN": FREEFLAG set, then PROGRAV). */
void gravel_after_exit(void)
{
    tc_to(GVDETER + 060u, DOWNFLAG); /* ADRES FREEFLAG */
}

void gravel_display(void)
{
    set_A(DISGRVER);
    tc_to(GVDETER + 063u, BANKCALL); /* CADR GOFLASH */
}

void gravel_terminate(void)
{
    tc_to(GVDETER + 065u, GOTOPOOH);
}

void gravel_proceed(void)
{
    prograv(); /* TCF PROGRAV */
}

void gravel_recycle(void)
{
    tc_to(GVDETER + 067u, UPFLAG); /* ADRES FREEFLAG */
}

/* PROGRAV: a restart point (PHASCHNG, 04024); the new gravity (STARSAV1)
 * to GSAV. */
void prograv(void)
{
    tc_to(PROGRAV, PHASCHNG); /* OCT 04024 */
}

void prograv_after_phaschng(void)
{
    tc_at(PROGRAV + 2u);
    intpret();
}

static const word prograv_words[] = {
    /* PROGRAV +3 */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(STARSAV1),
    STORE(GSAV),
    ORDER(EXIT),
};

const struct interpretive_code PROGRAV_CODE = {015u, PROGRAV + 3u, COUNT(prograv_words),
                                               prograv_words};

/* ---- LUNG and ADDGRAV -- pp. 965-966 ---- */

/* LUNG: the caller's return to QMIN; GACC, where the readings add up,
 * cleared. */
static const word lung_words[] = {
    /* LUNG */ ORDERS(STQ, VLOAD),
    ADDRESS(QMIN),
    ADDRESS_PLUS_ONE(ZEROVEC),
    STORE(GACC),
    ORDER(EXIT),
};

const struct interpretive_code LUNG_CODE = {015u, LUNG, COUNT(lung_words), lung_words};

/* After the EXIT: a restart point (PHASCHNG, 04024); the IMU coarse
 * aligned to THETAD (IMUCOARS), then fine aligned (IMUFINE), IMUSTALL
 * waiting for each (CURTAINS when it fails). */
void lung_after_exit(void)
{
    tc_to(LUNG + 5u, PHASCHNG); /* OCT 04024 */
}

void lung_coarse(void)
{
    tc_to(LUNG + 7u, BANKCALL); /* CADR IMUCOARS */
}

void lung_after_imucoars(void)
{
    tc_to(LUNG + 011u, BANKCALL); /* CADR IMUSTALL */
}

void lung_coarse_failed(void)
{
    tc_to(LUNG + 013u, CURTAINS);
}

void lung_fine(void)
{
    tc_to(LUNG + 014u, BANKCALL); /* CADR IMUFINE */
}

void lung_after_imufine(void)
{
    tc_to(LUNG + 016u, BANKCALL); /* CADR IMUSTALL */
}

void lung_fine_failed(void)
{
    tc_to(LUNG + 020u, CURTAINS);
}

/* The count of readings (T/2SEC, -20) to GCTR; PRIO31's word to
 * 1/PIPADT; the compensation initialized (GCOMPZER), a restart point
 * (PHASCHNG, 04024), and the PIPAs read once (PIPSRINE: "DON'T USE
 * DATA"). (Each A is a constant: no TS skips.) */
void lung_aligned(void)
{
    set_A(T_2SEC); /* CA T/2SEC */
    ts(ADDRESS_OF(GCTR));
    set_A(PRIO31); /* CA PRIO31 */
    ts(ADDRESS_OF(ONE_PIPADT));
    tc_to(LUNG + 025u, BANKCALL); /* CADR GCOMPZER */
}

void lung_after_gcompzer(void)
{
    tc_to(LUNG + 027u, PHASCHNG); /* OCT 04024 */
}

void lung_pipas(void)
{
    tc_to(LUNG + 031u, BANKCALL); /* CADR PIPSRINE */
}

void lung_after_pipsrine(void)
{
    tc_at(LUNG + 033u);
    intpret();
}

/* GREED, an EXIT ("= MASK 7776 IN BASIC SO DON'T CARE"): BMN GREED comes
 * back here for each reading but the last. */
static const word greed_words[] = {
    /* GREED */ ORDER(EXIT),
};

const struct interpretive_code GREED_CODE = {015u, GREED, COUNT(greed_words), greed_words};

/* "SET UP 2 SEC TASK TO READ PIPAS" (GRABGRAV, by TWIDDLE), and the job
 * ends. */
void greed_after_exit(void)
{
    set_A(TWO_SECS);
    tc_to(GREED + 2u, TWIDDLE); /* ADRES GRABGRAV */
}

void greed_after_twiddle(void)
{
    tc_to(GREED + 4u, ENDOFJOB);
}

/* GRABGRAV, the task: the PIPAs read (PIPSRINE); "RE-ESTABLISH MAINLINE
 * JOB" (ADDGRAV, priority 13, by FINDVAC); the task ends. */
void grabgrav(void)
{
    tc_to(GRABGRAV, IBNKCALL); /* CADR PIPSRINE */
}

void grabgrav_after_pipsrine(void)
{
    set_A(PRIO13);
    tc_to(GRABGRAV + 3u, FINDVAC); /* 2CADR ADDGRAV */
}

void grabgrav_after_findvac(void)
{
    tc_to(GRABGRAV + 6u, TASKOVER);
}

/* ADDGRAV, the job: the reading compensated (1/PIPA, to DELV), one more
 * counted (GCTR). */
void addgrav(void)
{
    tc_to(ADDGRAV, BANKCALL); /* CADR 1/PIPA */
}

void addgrav_after_pipa(void)
{
    incr(ADDRESS_OF(GCTR));
    tc_at(ADDGRAV + 3u);
    intpret();
}

/* The reading added to GACC ("ACCUMULATE G VECTOR"); while GCTR is
 * negative, another (GREED); then the sum, a unit vector, to STAR, and
 * in navigation base axes (CDUTRIG, *SMNB*) to GOUT. */
static const word addgrav_words[] = {
    /* ADDGRAV +4 */ ORDERS(VLOAD, VAD),
    ADDRESS_PLUS_ONE(DELV),
    ADDRESS_PLUS_ONE(GACC),
    STORE(GACC),
    ORDERS(SLOAD, BMN),
    ADDRESS_PLUS_ONE(GCTR),
    ADDRESS(GREED),
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(GACC),
    STCALL(STAR),
    ADDRESS(CDUTRIG), /* "TRANSFORM IN NB COOR AND STORE" */
    ORDER(CALL),
    ADDRESS(STAR_SMNB_STAR),
    STORE(GOUT),
    ORDER(EXIT),
};

const struct interpretive_code ADDGRAV_CODE = {015u, ADDGRAV + 4u, COUNT(addgrav_words),
                                               addgrav_words};

/* After the EXIT: a restart point (PHASCHNG, 04024), then QMINEXIT, back
 * to LUNG's caller (QMIN). */
void addgrav_after_exit(void)
{
    tc_to(ADDGRAV + 023u, PHASCHNG); /* OCT 04024 */
}

void qminexit(void)
{
    tc_at(QMINEXIT);
    intpret();
}

static const word qminexit_words[] = {
    /* QMINEXIT +1 */ ORDER(GOTO),
    ADDRESS(QMIN),
};

const struct interpretive_code QMINEXIT_CODE = {015u, QMINEXIT + 1u, COUNT(qminexit_words),
                                                qminexit_words};

#endif
