/* SPDX-License-Identifier: MIT */
/* RCS-CSM_DIGITAL_AUTOPILOT.agc (Comanche055 pp. 1002-1008, bank 21):
 * the start of the "T5 INTERRUPT PROGRAM FOR THE RCS-CSM AUTOPILOT", its
 * phase 1. T5PHASE says what each T5 interrupt does ("+ = INITIALIZE T5
 * RCS-CSM AUTOPILOT", "+0 = PHASE2 OF THE T5 PROGRAM", "- = RESTART DAP",
 * "-0 = PHASE1 OF THE T5 PROGRAM"). Phase 1 checks the IMU, filters the
 * body rates from the CDU angles, calls the one-second update of the
 * gimbal-to-body matrix and steps the desired angles of an automatic
 * maneuver; the attitude error display ends it (needles.c). The faithful
 * C is c64-port/src/cm_rcs_dap.c. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

static void sett5(void);

/* ---- REDORCS, RCSATT -- pp. 1002-1003 ---- */

/* RCSATT +1: "SAVE Q"; then the autopilot's control, from channel 31:
 * "BIT15 CHAN31 = 0 IF IMU POWER IS ON AND S/C CONT SW IS IN CMC (I.E. IF
 * G/C AUTO PILOT IS FULLY ENABLED)": on to SETT5. If not, "SET NORATE
 * FLAG" (bit 14 of RCSFLAGS), "SET HOLDFLAG +", "ZERO ERRORX, ERRORY, AND
 * ERRORZ", "AND CHECK FREE FUNCTION (BIT14 CHAN31)": "IF IN FREE MODE, GO
 * TO SETT5". Otherwise "SCHEDULE REINITIALIZATION (FRESHDAP) IN 100 MS
 * VIA T5RUPT" (T5PHASE +) and "ZERO JET CHANNELS IN 14 MS VIA ZEROJET". */
static void rcsatt_after_lxch(void)
{
    qxch(ADDRESS_OF(QRUPT));
    set_A(BIT15);
    rand_channel(031);
    if (A_is_zero()) { /* EXTEND; BZF SETT5 */
        sett5();
        return;
    }
    cs(ADDRESS_OF(RCSFLAGS));
    mask(BIT14);
    ads(ADDRESS_OF(RCSFLAGS));
    set_A(POSMAX);
    ts(ADDRESS_OF(HOLDFLAG));
    set_A(ZERO);
    ts(ADDRESS_OF(ERRORX));
    ts(ADDRESS_OF(ERRORY));
    ts(ADDRESS_OF(ERRORZ));
    set_A(BIT14);
    rand_channel(031);
    if (A_is_zero()) { /* EXTEND; BZF SETT5 */
        sett5();
        return;
    }
    ts(ADDRESS_OF(T5PHASE));
    set_A(OCT37766);
    ts(ADDRESS_OF(TIME5));
    tc_to(RCSATT + 031u, ZEROJET); /* TCR */
}

/* REDORCS: "RESTART OF AUTOPILOT COMES HERE ON A T5 RUPT": "IF T5PHASE
 * +0, -0, OR -, RESET TO -" (a restart of the DAP), "IF T5PHASE +, LEAVE
 * IT +. DO A FRESHDAP"; then "HOOK UP T5RUPT TO AUTOPILOT" (T5LOC the
 * 2CADR RCSLOC, of RCSATT) and on as RCSATT. */
void redorcs(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    ca(ADDRESS_OF(T5PHASE));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF +2 */
        set_A(negate(ONE));     /* CS */
        ts(ADDRESS_OF(T5PHASE));
    }
    dca(ADDRESS_OF(RCSLOC));
    dxch(ADDRESS_OF(T5LOC));
    rcsatt_after_lxch(); /* TCF RCSATT +1 */
}

/* RCSATT, the T5 interrupt's entry: "SAVE BB". */
void rcsatt(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    rcsatt_after_lxch();
}

/* After TCR ZEROJET: TCF KMATRIX. */
void rcsatt_after_zerojet(void)
{
    continue_at(KMATRIX);
}

/* ---- SETT5, the IMU status check -- pp. 1003-1004 ---- */

/* FREECHK, with no IMU ("NO ATTITUDE REFERENCE"): bit 14 of RCSFLAGS set
 * ("RATES HAVE NOT BEEN INITIALIZED"), HOLDFLAG + ("STOP ANY AUTOMATIC
 * STEERING AND PREPARE TO PICK UP CDU ANGLES UPON RESUMPTION OF ATTITUDE
 * HOLD"); "IN FREE MODE PROVIDE FREE CONTROL ONLY" (KRESUME1), otherwise
 * REINIT. */
static void freechk(void)
{
    cs(ADDRESS_OF(RCSFLAGS));
    mask(BIT14);
    ads(ADDRESS_OF(RCSFLAGS));
    set_A(BIT14);
    ts(ADDRESS_OF(HOLDFLAG));
    rand_channel(031); /* "CHECK FOR FREE MODE" */
    if (A_is_zero())
        continue_at(KRESUME1); /* EXTEND; BZF */
    else
        continue_at(REINIT); /* TCF */
}

/* RATEFILT: "SEE IF RATEFILTER HAS BEEN INITIALIZED": "IF SO, PROCEED
 * WITH RATE DERIVATION", "IF NOT, SKIP RATE DERIVATION". */
static void ratefilt(void)
{
    ca(ADDRESS_OF(RCSFLAGS));
    mask(BIT14);
    if (!A_is_zero()) { /* EXTEND; BZF +2 */
        continue_at(KMATRIX);
        return;
    }
    set_A(TWO);
    drholoop();
}

/* SETT5: "CHECK PHASE OF T5 PROGRAM" (CCS T5PHASE): FRESHDAP, "TURN ON
 * AUTOPILOT"; T5PHASE2, "BRANCH TO PHASE2 OF PROGRAM"; REDAP, "RESTART
 * AUTOPILOT"; or phase 1: "PHASE 1 RESET FOR PHASE 2" (T5PHASE +0), the
 * time saved in T5TIME ("USED IN COMPENSATING FOR DELAYS IN T5"), "RESET
 * FOR T5RUPT IN 20MS FOR PHASE2", and the "IMU STATUS CHECK" ("BIT6 = 0
 * IMU OK" in IMODES33): RATEFILT, or FREECHK. */
static void sett5(void)
{
    switch (ccs_at(ADDRESS_OF(T5PHASE))) {
    case POSITIVE: continue_at(FRESHDAP); return;
    case IS_PLUS_ZERO: continue_at(T5PHASE2); return;
    case NEGATIVE: continue_at(REDAP); return;
    case IS_MINUS_ZERO: break;
    }
    ts(ADDRESS_OF(T5PHASE));
    ca(ADDRESS_OF(TIME5));
    ts(ADDRESS_OF(T5TIME));
    set_A(DELTATT2);
    ts(ADDRESS_OF(TIME5));
    cs(ADDRESS_OF(IMODES33));
    mask(BIT6);
    if (A_is_zero()) /* CCS A */
        freechk();
    else
        ratefilt();
}

/* ---- The rate filter -- pp. 1004-1006 ---- */

/* The "RATE FILTER EQUATIONS" (the listing marks the last pass's values
 * with a -1 below them, here (-1)):
 *     DRHO = DELRHO - (.1)ADOT + (1 - GAIN1)DRHO(-1)
 *     ADOT = ADOT(-1) + GAIN2 DRHO + KMJ DFT
 * where DELRHO = AMGB (CDU - CDU(-1)), the CDU angles' change since the
 * last pass in body axes. DRHOLOOP and ADOTLOOP run for each axis, SPNDX
 * 2, 1 and 0 (DPNDX twice it, for the double words); ATTKALMN picks the
 * filter's gains. */

/* DRHOLOOP: the axis' indexes. (TS SPNDX overflows only for an A that
 * holds an overflow, and skips the DOUBLE; TS DPNDX overflows only when
 * the DOUBLE does, and skips the INDEX DPNDX: CS DRHO runs unindexed.) */
void drholoop(void)
{
    if (!ts(ADDRESS_OF(SPNDX))) double_A();
    continue_at(ts(ADDRESS_OF(DPNDX)) ? DRHOLOOP + 4u : DRHOLOOP + 3u);
}

/* DRHOLOOP +5: "PICK UP DESIRED FILTER GAIN": DRHO times GAIN1, "(1
 * -.064)DRHO". */
static void drholoop_gain(void)
{
    mp_at(index_read(ADDRESS_OF(GAIN1), fetch(ADDRESS_OF(ATTKALMN))));
    drholoop_das();
}

/* DRHOLOOP +3: INDEX DPNDX; CS DRHO, "DRHO SCALED 180 DEGS". */
void drholoop_drho(void)
{
    cs(index_read(ADDRESS_OF(DRHO), dpndx()));
    drholoop_gain();
}

/* DRHOLOOP +4: CS DRHO, the INDEX skipped. */
void drholoop_drho_unindexed(void)
{
    cs(ADDRESS_OF(DRHO));
    drholoop_gain();
}

/* DRHOLOOP +010: -(.1)ADOT, by SMALLMP: QUARTER times -ADOT in KMPAC. */
void drholoop_das(void)
{
    das_indexed(ADDRESS_OF(DRHO), dpndx());
    dcs_indexed(ADDRESS_OF(ADOT), dpndx());
    dxch(ADDRESS_OF(KMPAC)); /* "-(.1)ADOT" */
    set_A(QUARTER);          /* CA */
    tc_to(DRHOLOOP + 017u, SMALLMP);
}

/* The measured body rates, DELRHO = AMGB (CDU - CDU(-1)): each CDU's
 * change from the last pass (RHO, RHO1, RHO2 keep them) into DELTEMPX,
 * DELTEMPY and DELTEMPZ, double length, "MUST BE DOUBLE PRECISION OR WILL
 * LOSE PULSES": DELTEMPX = (CDUX-RHO) + AMGB1(CDUY-RHO1), DELTEMPY =
 * AMGB4(CDUY-RHO1) + AMGB5(CDUZ-RHO2), DELTEMPZ = AMGB7(CDUY-RHO1) +
 * AMGB8(CDUZ-RHO2). Then ADOTLOOP. (CCS SPNDX comes in at CA CDUX on +0,
 * at XCH RHO on a negative SPNDX and at EXTEND; MSU RHO on -0. MSU leaves
 * no overflow in A, so TS T5TEMP never skips; DAS has left L +0 before
 * ZL.) */
static void body_rates(enum sign entry)
{
    if (entry == IS_PLUS_ZERO) ca(ADDRESS_OF(CDUX));
    if (entry != IS_MINUS_ZERO) xch(ADDRESS_OF(RHO));
    msu_at(ADDRESS_OF(RHO));
    com();
    zl();
    dxch(ADDRESS_OF(DELTEMPX));
    ca(ADDRESS_OF(CDUY));
    xch(ADDRESS_OF(RHO1));
    msu_at(ADDRESS_OF(RHO1));
    com();
    ts(ADDRESS_OF(T5TEMP)); /* "(CDUY - RHO1) SCALED 90 DEGS" */
    mp_at(ADDRESS_OF(AMGB1));
    das(ADDRESS_OF(DELTEMPX));
    ca(ADDRESS_OF(AMGB4));
    mp_at(ADDRESS_OF(T5TEMP));
    dxch(ADDRESS_OF(DELTEMPY));
    ca(ADDRESS_OF(AMGB7));
    mp_at(ADDRESS_OF(T5TEMP));
    dxch(ADDRESS_OF(DELTEMPZ));
    ca(ADDRESS_OF(CDUZ));
    xch(ADDRESS_OF(RHO2));
    msu_at(ADDRESS_OF(RHO2));
    com();
    ts(ADDRESS_OF(T5TEMP)); /* "(CDUZ - RHO2) SCALED 90 DEGS" */
    mp_at(ADDRESS_OF(AMGB5));
    das(ADDRESS_OF(DELTEMPY));
    ca(ADDRESS_OF(AMGB8));
    mp_at(ADDRESS_OF(T5TEMP));
    das(ADDRESS_OF(DELTEMPZ));
    set_A(TWO);
    adotloop();
}

/* DRHOLOOP +020, after SMALLMP: DRHO's last term added in; CCS SPNDX:
 * the next axis, or the measured body rates. */
void drholoop_after_smallmp(void)
{
    enum sign axis;

    dxch(ADDRESS_OF(KMPAC));
    das_indexed(ADDRESS_OF(DRHO), dpndx());
    axis = ccs_at(ADDRESS_OF(SPNDX));
    if (axis == POSITIVE)
        continue_at(DRHOLOOP); /* TCF */
    else
        body_rates(axis);
}

/* ADOTLOOP: the axis' indexes, and DELTEMP's double word (as in DRHOLOOP,
 * TS DPNDX overflows only when the DOUBLE does, and then skips the EXTEND:
 * DCA DELTEMPX runs as CA DELTEMPX +1). */
void adotloop(void)
{
    if (!ts(ADDRESS_OF(SPNDX))) double_A();
    if (ts(ADDRESS_OF(DPNDX)))
        ca(index_read(ADDRESS_OF(DELTEMPX) + 1u, dpndx()));
    else
        dca_indexed(ADDRESS_OF(DELTEMPX), dpndx());
    continue_at(ADOTLOOP + 6u);
}

static void kmatrix_ccs(enum sign entry);

/* ADOTLOOP +013: DELTEMP into MERRORX too; then "ADOT + (.16)(.1)DRHO"
 * (DRHO doubled twice, "N.B.", times GAIN2, "PICK UP DESIRED FILTER
 * GAINS") and "KMJ(DFT)" ("S/C TORQUE TO INERTIA RATIO" times DFT) into
 * ADOT; CCS SPNDX: the next axis ("END CALCULATION OF VEHICLE RATES"), or
 * KMATRIX. */
void adotloop_merrorx(void)
{
    enum sign axis;

    das_indexed(ADDRESS_OF(MERRORX), dpndx());
    ca(index_read(ADDRESS_OF(DRHO), dpndx()));
    double_A(); /* "N.B." */
    double_A(); /* "N.B." */
    mp_at(index_read(ADDRESS_OF(GAIN2), fetch(ADDRESS_OF(ATTKALMN))));
    das_indexed(ADDRESS_OF(ADOT), dpndx());
    ca(index_read(ADDRESS_OF(KMJ), spndx()));
    mp_at(index_read(ADDRESS_OF(DFT), spndx()));
    das_indexed(ADDRESS_OF(ADOT), dpndx()); /* "KMJ(DFT)" */
    axis = ccs_at(ADDRESS_OF(SPNDX));
    if (axis == POSITIVE)
        continue_at(ADOTLOOP); /* TCF */
    else
        kmatrix_ccs(axis);
}

/* ADOTLOOP +6: DELTEMP into DRHO. */
void adotloop_drho(void)
{
    das_indexed(ADDRESS_OF(DRHO), dpndx());
    dca_indexed(ADDRESS_OF(DELTEMPX), dpndx());
    adotloop_merrorx();
}

/* ---- KMATRIX, the desired angles -- pp. 1006-1008 ---- */

static void tenthsek(void);

/* KMATRIX: ATTSEC counts the passes, ten to a second: in the tenth, "CALL
 * FOR 1 SEC UPDATE OF TRANSFORMATION MATRIX FROM GIMBAL AXES TO BODY AXES"
 * (AMBGUPDT, by NOVAC). (CCS SPNDX comes in at CA ATTSEC on +0, at MASK
 * LOW4 on a negative SPNDX and at CCS A, with A +0, on -0, where MASK
 * LOW4 changes nothing. The low four bits leave A + or +0.) */
static void kmatrix_ccs(enum sign entry)
{
    if (entry == IS_PLUS_ZERO) ca(ADDRESS_OF(ATTSEC));
    mask(LOW4);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tenthsek(); /* TCF */
        return;
    }
    set_A(PRIO34);
    tc_to(KMATRIX + 5u, NOVAC);
}

void kmatrix(void)
{
    kmatrix_ccs(IS_PLUS_ZERO);
}

/* KMATRIX +010, after NOVAC: ATTSEC starts again at NINE. */
void kmatrix_after_novac(void)
{
    set_A(NINE);
    tenthsek();
}

/* TENTHSEK: "WHEN AUTOMATIC MANEUVERS ARE BEING PERFORMED, THE FOLLOWING
 * ANGLE ADDITION MUST BE MADE TO PROVIDE A SMOOTH SEQUENCE OF ANGULAR
 * COMMANDS TO THE AUTOPILOT": CDUXD = CDUXD + DELCDUX, and so on, double
 * length. HOLDFLAG is - while "PERFORMING AUTOMATIC MANEUVER": "IF
 * HOLDFLAG +0,-0,+, BYPASS AUTOMATIC COMMANDS". */
static void tenthsek(void)
{
    ts(ADDRESS_OF(ATTSEC));
    cs(ADDRESS_OF(HOLDFLAG));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF */
        continue_at(DACNDLS);
        return;
    }
    set_A(TWO); /* DCDUINCR */
    deloop();
}

/* DELOOP: the axis' indexes and the desired angle, CDUXD's double word.
 * (TS SPNDX's and TS DPNDX's overflows as in DRHOLOOP: TS DPNDX's leaves A
 * +1 or -1 to INDEX A, and skips the EXTEND, so DCA CDUXD runs as CA CDUXD
 * +1.) */
void deloop(void)
{
    if (!ts(ADDRESS_OF(SPNDX))) double_A();
    if (ts(ADDRESS_OF(DPNDX)))
        ca(index_read(ADDRESS_OF(CDUXD) + 1u, a_word()));
    else
        dca_indexed(ADDRESS_OF(CDUXD), a_word());
    continue_at(DELOOP + 6u);
}

/* DELOOP +6: the angle into KMPAC and its increment, DELCDUX's double
 * word, added by DPADD (round the circle on an overflow). */
void deloop_delcdux(void)
{
    dxch(ADDRESS_OF(KMPAC));
    dca_indexed(ADDRESS_OF(DELCDUX), dpndx());
    deloop_dpadd();
}

/* DELOOP +012: TC DPADD. */
void deloop_dpadd(void)
{
    tc_to(DELOOP + 012u, DPADD);
}

/* DELOOP +013, after DPADD: the sum's upper word into THETADX, the
 * single-length desired angle, and the double word back into CDUXD. */
void deloop_after_dpadd(void)
{
    dca(ADDRESS_OF(KMPAC));
    ts(index_ts(ADDRESS_OF(THETADX), spndx()));
    dxch_indexed(ADDRESS_OF(CDUXD), dpndx());
    deloop_next();
}

/* DELOOP +021: CCS SPNDX: the next axis, or the attitude error display. */
void deloop_next(void)
{
    enum sign axis = ccs_at(ADDRESS_OF(SPNDX));

    if (axis == POSITIVE)
        continue_at(DELOOP); /* TCF */
    else
        dacndls_ccs(axis);
}

#endif
