/* SPDX-License-Identifier: MIT */
/* FINDCDUW--GUIDAP_INTERFACE.agc, pp. 913-918 (Luminary099, bank 30):
 * from AFTRFLTR to TCQCDUW. The window, the required direction cosine
 * matrix and its gimbal angles, the middle gimbal angle limited, the
 * attitude angle changes limited, the commanded attitude rates, and "FINAL
 * TRANSFER" of the commands to the autopilot. The faithful C is
 * c64-port/src/lm_findcduw.c. */
#include <stdbool.h>
#include "findcduw.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099

static void cduwxfr(void);

/* ---- The window and the direction cosine matrix -- p. 913 ---- */

/* "FIND A SUITABLE WINDOW POINTING VECTOR": "IF XOV NOT INHIBITED, GO
 * FETCH ZNB" (BHIZ, FLAGOODW +0); else the window command, tested by
 * UNWCTEST, which comes back here only if it is too near the thrust
 * command (and zeroes FLAGOODW). Then the vehicle's Z axis, tested too;
 * and failing both, -X: "Z AND -X CAN'T BOTH PARALLEL UNFC/2".
 *
 * DCMCL, "COMPUTE THE REQUIRED DIRECTION COSINE MATRIX", in the push-down
 * list's words 0 to 17 (UNX/2, UNY/2 and UNZ/2):
 *     UNY/2 = UNIT(UNZ/2 * UNX/2)          (pushed: "FIRST ITERATION")
 *     -UNZ/2 = 2 UNY/2 * UNX/2
 *     UNX/2 = UNIT(2 (-UNFVY/2 UNY/2 - UNFVZ/2 UNZ/2) + UNX/2)
 * which "TOTALLY ELIMINATES THRUST POINTING ERROR" (the filtered offsets
 * "MUST BE SMALL"), then
 *     UNY/2 = 2 UNX/2 * -UNZ/2,    UNZ/2 = 2 (-UNY/2 * UNX/2).
 * "COMPUTES THE REQUIRED GIMBAL ANGLES" (NB2CDUSP), and the basic code
 * (EXIT). */
static const word aftrfltr_words[] = {
    /* AFTRFLTR */ ORDERS(SLOAD, BHIZ),
    ADDRESS_PLUS_ONE(FLAGOODW),
    ADDRESS(FETCHZNB),
    ORDERS(VLOAD, CALL),
    ADDRESS_PLUS_ONE(UNZ_2),
    ADDRESS(UNWCTEST),
    /* FETCHZNB */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(ZNBPIP),
    STCALL(UNZ_2),
    ADDRESS(UNWCTEST),
    ORDERS(VLOAD, VCOMP),
    ADDRESS_PLUS_ONE(XNBPIP),
    STORE(UNZ_2),
    /* DCMCL */ ORDERS(VLOAD, VXV),
    ADDRESS_PLUS_ONE(UNZ_2),
    ADDRESS_PLUS_ONE(UNX_2),
    ORDERS(UNIT, PUSH), /* "UNY/2 FIRST ITERATION" */
    ORDERS(VXV, VSL1),
    ADDRESS_PLUS_ONE(UNX_2),
    STORE(UNZ_2),       /* "-UNZ/2 FIRST ITERATION" */
    ORDERS(VXSC, PDVL), /* "EXCHANGE -UNFVZ/2 UNZ/2 FOR UNY/2" */
    ADDRESS_PLUS_ONE(UNFVZ_2),
    ORDERS(VXSC, BVSU), /* "YIELDS -UNFVY/2 UNY/2-UNFVZ/2 UNZ/2" */
    ADDRESS_PLUS_ONE(UNFVY_2),
    ORDERS(VSL1, VAD),
    ADDRESS_PLUS_ONE(UNX_2),
    ORDER(UNIT),
    STORE(UNX_2),
    ORDERS(VXV, VSL1),
    ADDRESS_PLUS_ONE(UNZ_2), /* "-UNZ/2 WAS STORED HERE REMEMBER" */
    STORE(UNY_2),
    ORDERS(VCOMP, VXV),
    ADDRESS_PLUS_ONE(UNX_2),
    ORDER(VSL1),
    STORE(UNZ_2),
    ORDER(CALL),
    ADDRESS(NB2CDUSP), /* "YIELDS THE RQD GIMBAL ANGLES, 2'S, PI" */
    ORDER(EXIT),
};

const struct interpretive_code AFTRFLTR_CODE = {030u, AFTRFLTR, COUNT(aftrfltr_words),
                                                aftrfltr_words};

/* ---- The middle gimbal angle -- p. 914 ---- */

/* "LIMIT THE MIDDLE GIMBAL ANGLE": NB2CDUSP's angles are in MPAC, MPAC +1
 * and MPAC +2 (X, Y, Z); the middle one, Z, to within CDUZDLIM, "70 DEG
 * LIMIT FOR MGA" (LIMITSUB; "CAN'T LXCH: NEED UNLIMITED MGA FOR ALARM").
 * (A is a word: TS L does not skip.) */
void limit_mga(void)
{
    ca(MPAC_AT(2u));
    ts(L_ADDRESS);
    set_A(CDUZDLIM);
    tc_at(MGARET - 7u);
    limitsub();
    limit_mga_after_limitsub();
}

/* LIMITSUB's return: the limited angle to MPAC +2, and the difference
 * from the unlimited one: "THIS BETTER YIELD ZERO", else alarm 00401
 * (ALARMMGA), which comes back to MGARET. ("1 BIT ERROR POSSIBLE BECAUSE
 * USING 2'S COMP. WHO CARES?") */
void limit_mga_after_limitsub(void)
{
    xch(MPAC_AT(2u));
    msu_at(MPAC_AT(2u));
    if (!A_is_zero()) {
        tc_to(ALARMMGA, ALARM); /* OCT 00401 */
        return;
    }
    mgaret();
}

/* MGARET: "COMPUTE THE UNLIMITED GIMBAL ANGLE CHGS", Z, Y, X (TEM2 2, 1,
 * 0), interrupts inhibited ("RELINT AT TC INTPRET AFTER TCQCDUW"). Each
 * angle goes to CPHI ("OUTPUTS TO NOUN22"), and its change from the
 * commanded one (MSU CDUXD: "NO MATTER THAT THESE SLIGHTLY DIFFERENT FROM
 * WHEN WE INITIALLY FETCHED THEM"), negated, to -DELGMB: "-UNLIMITED
 * GIMBAL ANGLE CHGS, 1'S, PI". "TO PREVENT FALSE STARTS ABOUT X, ZERO
 * FLAGOODW IF DELGMBZ OR Y TOO BIG": the last change (in L) squared plus
 * HI5, "WITHIN 1 BIT OF -(45 DEG SQUARED)", above zero. (TEM2 is 2, 1 or
 * 0 here, and A a word at each TS: none skips.)
 *
 * Then the "BRANCHES TO NOATTCNT": "NO PNGCS AUTO" (FLPAUTNO), or "ENGINE
 * NOT ON" (ENGONFLG): NOATTCNT +2, a STOPRATE and no commands. CCS
 * FLPAUTNO takes four ways: above zero NOATTCNT +2; +0 the engine's bit;
 * below zero the bit of its magnitude less one; -0 straight to the BZF,
 * with A +0. */
void mgaret(void)
{
    inhint();
    zl();
    set_A(TWO);
    do {
        ts(ADDRESS_OF(TEM2)); /* DELGMBLP */
        ca(L_ADDRESS);
        square();
        ad(HI5);
        if (!A_is_zero_or_minus()) {
            set_A(ZERO);
            ts(ADDRESS_OF(FLAGOODW));
        }
        ca(indexed_operand(CA_CODE | ADDRESS_OF(MPAC), TEM2));
        ts(indexed_store(TS_CODE | ADDRESS_OF(CPHI), TEM2));
        msu_at(indexed_operand(MSU_CODE | ADDRESS_OF(CDUXD), TEM2));
        com();
        ts(indexed_store(TS_CODE | ADDRESS_OF(MDELGMB), TEM2));
        ts(L_ADDRESS); /* "FOR PRECEDING TEST ON NEXT LOOP PASS" */
    } while (ccs_at(ADDRESS_OF(TEM2)) == POSITIVE);

    switch (ccs_at(ADDRESS_OF(FLPAUTNO))) {
    case POSITIVE: noattcnt_plus2(); return;
    case IS_PLUS_ZERO: ca(ADDRESS_OF(FLAGWRD5)); /* fall through */
    case NEGATIVE: mask(ENGONBIT); break;
    case IS_MINUS_ZERO: break;
    }
    if (A_is_zero()) {
        noattcnt_plus2();
        return;
    }

    /* "LIMIT THE ATTITUDE ANGLE CHANGES" about X, YPRIME and ZPRIME, "THE
     * COMMANDED VEHICLE SYSTEM ROTATED ABOUT THE X AXIS TO BRING THE Z AXIS
     * INTO ALIGNMENT WITH THE MIDDLE GIMBAL AXIS": first Z, -DELGMB +2,
     * "SAME AS -DELATTZPRIME UNLIMITED", to DAZMAX (20 degrees in 2
     * seconds, 2 with the CSM docked). */
    lxch(MINUS_DELGMB_AT(2u));
    set_A(docked_constant(CA_CODE | DAXMAX)); /* INDEX NDXCDUW; CA DAZMAX */
    tc_at(MGARET + 042u);
    limitsub();
    limit_attitude_after_z();
}

/* ---- The attitude angle changes -- p. 916 ---- */

/* LIMITSUB's return: "-DELGMBZ"; then Y: -DELGMB +1 times COS(CDUZD),
 * "-DELATTYPRIME/2 UNLIMITED", to DAY/2MAX. (An A that overflows skips
 * the CA: MP then takes -DELGMBZ's +1 or -1. A product never overflows:
 * TS L does not skip.) */
void limit_attitude_after_z(void)
{
    if (!ts(MINUS_DELGMB_AT(2u))) ca(MINUS_DELGMB_AT(1u));
    mp_at(ADDRESS_OF(COSCDUZ));
    ts(L_ADDRESS);
    set_A(docked_constant(CA_CODE | DAY_2MAX));
    tc_at(MGARET + 052u);
    limitsub();
    limit_attitude_after_y();
}

/* LIMITSUB's return: divided by COS(CDUZD) back, "-DELGMBY, FETCHING
 * UNLIMITED VALUE"; then X: "+DELATTX UNLIMITD, MAG < 180 DEG. BASED ON
 * UNLIMITED DELGMBV" (twice the unlimited -DELGMBY times SIN(CDUZD),
 * negated, less -DELGMB; "ONE BIT ERROR IF OPERANDS IN MSU OF MIXED
 * SIGNS. WHO CARES?"), to DAXMAX. (MSU gives a word: TS L does not
 * skip.) */
void limit_attitude_after_y(void)
{
    dv_at(ADDRESS_OF(COSCDUZ));
    xch(MINUS_DELGMB_AT(1u));
    mp_at(ADDRESS_OF(SINCDUZ));
    ddoubl();
    com();
    msu_at(MINUS_DELGMB_AT(0u));
    ts(L_ADDRESS);
    set_A(docked_constant(CA_CODE | DAXMAX));
    tc_at(MGARET + 067u);
    limitsub();
    limit_attitude_after_x();
}

/* LIMITSUB's return: "SAVE LIMITED +DELATTX"; "FETCH IT BACK CHGING SIGN
 * IF WINDOW GOOD" (FLAGOODW), "OTHERWISE USE ZERO FOR -DELATTX"; and the
 * "-CNTRIB TO -DELATTX FROM -DELGMBY" (twice -DELGMBY times SIN(CDUZD))
 * added in: "-DELGMBX. NO OVERFLOW SINCE LIMITED TO
 * 20DEG(1+SIN(70DEG)/COS(70DEG)) < 180DEG". CCS FLAGOODW takes four ways, each starting
 * further down the same steps: above zero (as when an A that overflows
 * skips the CCS) from the CS; +0 from the TS, with A +0; below zero from
 * CS -DELGMB +1; -0 from the MP, with A +0.
 *
 * Then "COMPUTE COMMANDED ATTITUDE RATES", in units of PI/4 per second
 * ("2 SECONDS BEING THE COMPUTATION PERIOD"):
 *     OMEGAPD = -(-DELGMBX) - 4 SINCDUZ (-DELGMBY)
 *     OMEGAQD = -8 COSCDUZ COSCDUX (-DELGMBY) - 4 SINCDUX (-DELGMBZ)
 *     OMEGARD = +8 COSCDUZ SINCDUX (-DELGMBY) - 4 COSCDUX (-DELGMBZ)
 * and the "FINAL TRANSFER" (CDUWXFR). (A is a word at each TS: none
 * skips.) */
void limit_attitude_after_x(void)
{
    enum sign window = ts(MINUS_DELGMB_AT(0u)) ? POSITIVE : ccs_at(ADDRESS_OF(FLAGOODW));

    switch (window) {
    case POSITIVE: cs(MINUS_DELGMB_AT(0u));     /* fall through */
    case IS_PLUS_ZERO: ts(MINUS_DELGMB_AT(0u)); /* fall through */
    case NEGATIVE: cs(MINUS_DELGMB_AT(1u));     /* fall through */
    case IS_MINUS_ZERO: break;
    }
    mp_at(ADDRESS_OF(SINCDUZ));
    ddoubl();
    ads(MINUS_DELGMB_AT(0u));

    cs(MINUS_DELGMB_AT(0u)); /* OMEGAPD */
    ts(ADDRESS_OF(OMEGAPD));
    cs(MINUS_DELGMB_AT(1u));
    mp_at(ADDRESS_OF(SINCDUZ));
    ddoubl();
    ads(ADDRESS_OF(OMEGAPD));
    ads(ADDRESS_OF(OMEGAPD));

    cs(MINUS_DELGMB_AT(1u)); /* OMEGAQD */
    mp_at(ADDRESS_OF(COSCDUX));
    ddoubl();
    mp_at(ADDRESS_OF(COSCDUZ));
    ts(ADDRESS_OF(OMEGAQD));
    cs(MINUS_DELGMB_AT(2u));
    mp_at(ADDRESS_OF(SINCDUX));
    ads(ADDRESS_OF(OMEGAQD));
    ads(ADDRESS_OF(OMEGAQD));
    ads(ADDRESS_OF(OMEGAQD));

    ca(MINUS_DELGMB_AT(1u)); /* OMEGARD */
    mp_at(ADDRESS_OF(SINCDUX));
    ddoubl();
    mp_at(ADDRESS_OF(COSCDUZ));
    ts(ADDRESS_OF(OMEGARD));
    cs(MINUS_DELGMB_AT(2u));
    mp_at(ADDRESS_OF(COSCDUX));
    ads(ADDRESS_OF(OMEGARD));
    ads(ADDRESS_OF(OMEGARD));
    ads(ADDRESS_OF(OMEGARD));

    set_A(TWO);
    cduwxfr();
}

/* ---- The final transfer -- p. 918 ---- */

/* CDUWXFR, for Z, Y, X (TEM2 2, 1, 0): each -DELGMB times DT/DELT, the
 * "RATIO OF DAP INTERVAL TO CDUW INTERVAL", in two's complement
 * (ONESTO2S), to DELCDUX, "ANGLE INTERFACE"; then the lag angle. */
static void cduwxfr_step(void)
{
    ts(ADDRESS_OF(TEM2));
    ca(indexed_operand(CA_CODE | ADDRESS_OF(MDELGMB), TEM2));
    mp(DT_DELT);
    tc_at(CDUWXFR + 5u);
    onesto2s();
}

/* ONESTO2S's return: the "LAG ANGLE = OMEGA ABS(OMEGA)/2 ACCEL" (the
 * rate, "WE NOW HAVE ABS(OMEGAPD,QD,RD)", times the rate, times BIT11,
 * "1/16", over 1JACC, the axis's acceleration), to DELERLIM. CCS takes the
 * rate's magnitude: AD ONE puts back the one CCS took off. (An A that
 * overflows skips the INDEX: CCS then takes OMEGAPD. A quotient is a
 * word: TS L does not skip.) */
static void lag_angle(void)
{
    bool skipped = ts(indexed_store(TS_CODE | ADDRESS_OF(DELCDUX), TEM2));
    unsigned rate =
        skipped ? ADDRESS_OF(OMEGAPD) : indexed_operand(CCS_CODE | ADDRESS_OF(OMEGAPD), TEM2);

    switch (ccs_at(rate)) {
    case POSITIVE:
    case NEGATIVE: ad(ONE); break;
    default: break;
    }
    mp_at(indexed_operand(MP_CODE | ADDRESS_OF(OMEGAPD), TEM2));
    mp(BIT11);
    dv_at(indexed_operand(DV_CODE | ADDRESS_OF(1JACC), TEM2));
    if (!ts(L_ADDRESS)) set_A(DELERLIM);
    tc_at(CDUWXFR + 027u);
    limitsub();
}

/* LIMITSUB's return: the lag angle to DELPEROR. Then the next axis (CCS
 * TEM2 above zero: TEM2 less one), or "HAUSKEEPING AND RETURN". True to
 * go round again: also when an A that overflows skips the CCS, with TEM2
 * then +1 or -1. CCS TEM2 below zero starts TCQCDUW at TS EBANK (A the
 * magnitude less one), -0 at its TC INTPRET. */
static bool after_lag_angle(void)
{
    if (ts(indexed_store(TS_CODE | ADDRESS_OF(DELPEROR), TEM2))) return true;
    switch (ccs_at(ADDRESS_OF(TEM2))) {
    case POSITIVE: return true;
    case IS_PLUS_ZERO: tcqcduw(); break;
    case NEGATIVE:
        ts(EBANK_ADDRESS);
        tcqcduw_intpret();
        break;
    case IS_MINUS_ZERO: tcqcduw_intpret(); break;
    }
    return false;
}

/* CDUWXFR, with the index in A. */
static void cduwxfr(void)
{
    do {
        cduwxfr_step();
        lag_angle();
    } while (after_lag_angle());
}

void cduwxfr_after_onesto2s(void)
{
    lag_angle();
    if (after_lag_angle()) cduwxfr();
}

void cduwxfr_after_limitsub(void)
{
    if (after_lag_angle()) cduwxfr();
}

/* TCQCDUW: "RETURN USER'S EBANK", and the interpreter: "NORMAL AND
 * ABNORMAL RETURN TO USER" (QCDUWUSR), the push-down list back to its
 * start (SETPD 0). (A is a word: TS EBANK does not skip.) */
void tcqcduw(void)
{
    ca(ADDRESS_OF(ECDUWUSR));
    ts(EBANK_ADDRESS);
    tcqcduw_intpret();
}

void tcqcduw_intpret(void)
{
    tc_at(TCQCDUW + 2u);
    intpret();
}

static const word tcqcduw_words[] = {
    /* TCQCDUW +3 */ ORDERS(SETPD, GOTO),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(QCDUWUSR),
};

const struct interpretive_code TCQCDUW_CODE = {030u, TCQCDUW + 3u, COUNT(tcqcduw_words),
                                               tcqcduw_words};

/* ---- NOATTCNT and ALARMMGA -- p. 924 ---- */

/* NOATTCNT: alarm 00402, "NO ATTITUDE CONTROL" (the commands would not
 * unitize). */
void noattcnt(void)
{
    tc_to(NOATTCNT, ALARM);
}

/* NOATTCNT +2: "COME HERE FOR NOATTCNT WITHOUT ALARM": STOPRATE (through
 * IBNKCALL; "RELINT AT TC INTPRET AFTER TCQCDUW"), then "RETURN TO USER
 * SKIPPING AUTOPILOT CMDS". */
void noattcnt_plus2(void)
{
    inhint();
    tc_to(NOATTCNT + 3u, IBNKCALL); /* FCADR STOPRATE */
}

void noattcnt_after_stoprate(void)
{
    tcqcduw();
}

/* ALARMMGA: alarm 00401, the middle gimbal angle limited ("FINDCDUW
 * DRIVES CDUXD AND CDUYD TO THE RQD VALUES, BUT DRIVES CDUZD ONLY TO THE
 * GIMBAL LOCK CONE"); its return goes on at MGARET. */
void alarmmga_after_alarm(void)
{
    mgaret();
}

#endif
