/* SPDX-License-Identifier: MIT */
/* CM_ENTRY_DIGITAL_AUTOPILOT.agc, part B (Comanche055 pp. 1065 and
 * 1070-1072, bank 15): "THIS SECTION CALCULATES THE ANGULAR BODY RATES EACH
 * .1 SEC.  THE ANGULAR RATES ARE THOSE ALONG THE BODY AXES" "XB, YB, ZB,
 * AND ARE NORMALLY DESIGNATED  P, Q, R."
 *
 * BODYRATE, from READGYMB's DOBRATE1 every 0.1 s, turns the gimbal angle
 * changes READGYM1 formed (-DELAOG, -DELAIG, -DELAMG, the outer, inner and
 * middle gimbals) into the body rates: the sines and cosines of the outer
 * and middle gimbal angles (SPSIN, SPCOS), then
 *   "PITCHDOT:  Q TCDU/180 = IDOT TCDU/180 COSO COSM + MDOT TCDU/180 SINO"
 *   "YAWDOT:  R TCDU/180 = -IDOT TCDU/180 COSM SINO + MDOT TCDU/180 COSO"
 *   "ROLLDOT:  P TCDU/180 = ODOT TCDU/180 + IDOT TCDU/180 SINM"
 * each averaged with the last change by RATEAVG. With the flight path
 * angle changing (GAMDOT), the rates are taken relative to the turning
 * flight path (attrates.c goes on with QREL and RREL).
 *
 * "SINCE RESTARTS ZERO THE JET OUTPUT CHANNELS, NO ATTEMPT IS MADE TO
 * RESTART THE  ENTRY DAPS." */
#include "entry_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* ---- RATEAVG -- p. 1065 ---- */

/* RATEAVG: "SUBROUTINE TO ESTIMATE RATES IN PRESENCE" "OF CONSTANT
 * ACCELERATION.": with the new change in JETEM and the last one in A,
 * "DELV (EST) = DELV +(DELV-OLDELV)/2"; back by TC Q. */
static void rateavg(void)
{
    com();
    ad_at(ADDRESS_OF(JETEM));
    mp(HALF); /* EXTEND; MP */
    ad_at(ADDRESS_OF(JETEM));
    tc_q();
}

/* ---- BODYRATE -- pp. 1070-1072 ---- */

/* BODYRATE: the middle gimbal angle's cosine, "THESE ARE 2S COMPL NOS, BUT
 * USE ANYWAY." */
void bodyrate(void)
{
    ca(ADDRESS_OF(AMG));
    tc_to(BODYRATE + 1u, SPCOS);
}

/* BODYRATE +2: into COSM; and the outer gimbal angle's sine, "C(AOG) =
 * AOG/180". (A TS that overflows, when SPCOS's caller left one in A, skips
 * the CA AOG: SPSIN runs on +1 or -1.) */
void bodyrate_cosm(void)
{
    if (!ts(ADDRESS_OF(COSM))) ca(ADDRESS_OF(AOG));
    tc_to(BODYRATE + 4u, SPSIN);
}

/* BODYRATE +5: "SINO = SIN(AOG)", and "SO CM" (SINOCOSM); then the outer
 * angle's cosine. (A TS that overflows skips the EXTEND: MP COSM runs as
 * MASK COSM.) */
void bodyrate_sino(void)
{
    if (ts(ADDRESS_OF(SINO)))
        mask_at(ADDRESS_OF(COSM));
    else
        mp_at(ADDRESS_OF(COSM));
    ts(ADDRESS_OF(SINOCOSM));
    ca(ADDRESS_OF(AOG));
    tc_to(BODYRATE + 012u, SPCOS);
}

/* BODYRATE +013: COSO, and "CO CM" (COSOCOSM). Then the pitch rate: the
 * middle gimbal's change times SINO into JETEM, "2 LOCS", the inner
 * gimbal's times COSOCOSM added in (DAS); its upper word the new change,
 * with the last one from OLDELQ, averaged (RATEAVG, back at BODYRATE
 * +032). (MP COSM runs as MASK COSM after an overflow, as above.) */
void bodyrate_coso(void)
{
    if (ts(ADDRESS_OF(COSO)))
        mask_at(ADDRESS_OF(COSM));
    else
        mp_at(ADDRESS_OF(COSM));
    ts(ADDRESS_OF(COSOCOSM));
    cs(ADDRESS_OF(MDELAMG));
    mp_at(ADDRESS_OF(SINO));
    dxch(ADDRESS_OF(JETEM));
    cs(ADDRESS_OF(MDELAIG));
    mp_at(ADDRESS_OF(COSOCOSM));
    das(ADDRESS_OF(JETEM));
    ca(ADDRESS_OF(JETEM));
    xch(ADDRESS_OF(OLDELQ));
    tc_at(BODYRATE + 031u);
    rateavg();
}

/* BODYRATE +032: "PITCHDOT = Q TCDU/180" (QREL). The yaw rate the same
 * way: the middle gimbal's change times COSO, the inner gimbal's times
 * SINOCOSM, averaged with OLDELR (back at BODYRATE +046). (A TS that
 * overflows skips the CS -DELAMG, landing on the EXTEND: MP COSO takes
 * +1 or -1.) */
void bodyrate_qrel(void)
{
    if (!ts(ADDRESS_OF(QREL))) cs(ADDRESS_OF(MDELAMG));
    mp_at(ADDRESS_OF(COSO));
    dxch(ADDRESS_OF(JETEM));
    ca(ADDRESS_OF(MDELAIG));
    mp_at(ADDRESS_OF(SINOCOSM));
    das(ADDRESS_OF(JETEM));
    ca(ADDRESS_OF(JETEM));
    xch(ADDRESS_OF(OLDELR));
    tc_at(BODYRATE + 045u);
    rateavg();
}

/* BODYRATE +046: "YAWDOT = R TCDU/180" (RREL); and the middle gimbal
 * angle's sine (an overflow skips the CA AMG). */
void bodyrate_rrel(void)
{
    if (!ts(ADDRESS_OF(RREL))) ca(ADDRESS_OF(AMG));
    tc_to(BODYRATE + 050u, SPSIN);
}

/* BODYRATE +051: SINM. The roll rate: the inner gimbal's change times SINM
 * into JETEM, its lower word rounded into the upper ("ROUND L INTO A":
 * CA ZERO; DDOUBL), the outer gimbal's change and JETEM added, negated
 * (CS A) into JETEM, the new change, and averaged with OLDELP (back at
 * BODYRATE +065). (MP -DELAIG runs as MASK after an overflow; an overflow
 * at TS JETEM skips the XCH OLDELP.) */
void bodyrate_sinm(void)
{
    if (ts(ADDRESS_OF(SINM)))
        mask_at(ADDRESS_OF(MDELAIG));
    else
        mp_at(ADDRESS_OF(MDELAIG));
    ts(ADDRESS_OF(JETEM));
    set_A(ZERO); /* CA ZERO */
    ddoubl();
    ad_at(ADDRESS_OF(MDELAOG));
    ad_at(ADDRESS_OF(JETEM));
    com(); /* CS A */
    if (!ts(ADDRESS_OF(JETEM))) xch(ADDRESS_OF(OLDELP));
    tc_at(BODYRATE + 064u);
    rateavg();
}

/* BODYRATE +065: "ROLLDOT = P TCDU/180" (PREL). Then, "IF GAMDOT < 0.5
 * DEG/SEC, THEN GAMDOT =0": CCS GAMDOT; positive, TC +2; +0, NOGAMDUT;
 * negative, on at BODYRATE +071; -0, on at its TC SPSIN with +0. An
 * overflow at TS PREL skips the CCS: on at the TC +2 with +1 or -1. */
void bodyrate_prel(void)
{
    if (ts(ADDRESS_OF(PREL))) {
        tc_at(BODYRATE + 067u); /* TC +2 */
        bodyrate_gamdot();
        return;
    }
    switch (ccs_at(ADDRESS_OF(GAMDOT))) {
    case POSITIVE:
        tc_at(BODYRATE + 067u); /* TC +2 */
        bodyrate_gamdot();
        break;
    case IS_PLUS_ZERO: bodyrate_nogamdut(); break;
    case NEGATIVE: bodyrate_gamdot(); break;
    case IS_MINUS_ZERO: tc_to(BODYRATE + 072u, SPSIN); break;
    }
}

/* BODYRATE +070: no flight path rate, on at NOGAMDUT. */
void bodyrate_nogamdut(void)
{
    tc_to(BODYRATE + 070u, NOGAMDUT);
}

/* BODYRATE +071: the sine of -ROLL/180, */
void bodyrate_gamdot(void)
{
    cs(ADDRESS_OF(ROLL_180));
    tc_to(BODYRATE + 072u, SPSIN);
}

/* BODYRATE +073: times GAMDOT, "-SR GAMDOT" (JETEM +1); and times
 * SINTRIM, "SIN(-20)", "(FOR NOMINAL L/D = .3)"; on at BODYRATE +0100
 * (attrates.c). */
void bodyrate_sr_gamdot(void)
{
    mp_at(ADDRESS_OF(GAMDOT)); /* EXTEND; MP */
    ts(ADDRESS_OF(JETEM) + 1u);
    mp(SINTRIM); /* EXTEND; MP */
    continue_at(BODYRATE + 0100u);
}

#endif
