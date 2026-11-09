/* SPDX-License-Identifier: MIT */
/* CM_ENTRY_DIGITAL_AUTOPILOT.agc, part C (Comanche055 pp. 1074-1075, bank
 * 15): the 0.1 s yaw and pitch DAPs inside the atmosphere, from ATTRATES
 * once .05GSW is set, and EXDAPIN, where the extra atmospheric DAP joins
 * them.
 *
 * "START YAW AUTOPILOT HERE.  RATE DAMPING WITH ENFORCED COORDINATED ROLL
 * MANEUVER." The yaw rate taken against the roll rate, "YAW ERROR = RREL
 * - PREL TAN(ALFA)", picks the yaw jets (YJETCODE) by its dead zone tag;
 * "START PITCH AUTOPILOT HERE.  RATE DAMPING ONLY.": the pitch rate QREL
 * picks the pitch jets (P/RJCODE). Both go out on channel 5, and JETAG
 * picks what follows: CM/RCS, the 2 s roll system, when its update is due;
 * else the needles (CM/FDAI, or CM/FDAIR when the roll jets run at 0.1 s,
 * "JETAG=-1 EQUIVALENT TO CMDAPMOD=+1"). */
#include "entry_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* The yaw error, "LET SIN(-20) BE APPROX FOR TAN(-20)", and "GO TEST DZ.
 * GET TAG: +0 IF IN DZ" (2D/SDZ, back at ATTRATES +076). */
static void yaw_error(void)
{
    cs(ADDRESS_OF(PREL));
    mp(SINTRIM); /* EXTEND; MP */
    ad_at(ADDRESS_OF(RREL));
    tc_at(ATTRATES + 075u);
    two_d_sdz();
}

/* ATTRATES +066: "SAVE -1 FOR USE IN CM/RCS" (CMDAPMOD), and -1 to AK1
 * and AK2, "TO ZERO PITCH AND YAW FDAI NEEDLES" "IN ATM.  (MODE =-1)".
 * (An overflow at TS CMDAPMOD, from a caller's A, skips the TS AK1.) */
void atm_cmdapmod(void)
{
    if (ts(ADDRESS_OF(CMDAPMOD))) {
        ts(ADDRESS_OF(AK2));
        yaw_error();
        return;
    }
    atm_ak1();
}

/* ATTRATES +067: AK1 and AK2 (an overflow skips the TS AK2); then the yaw
 * error. */
void atm_ak1(void)
{
    if (!ts(ADDRESS_OF(AK1))) ts(ADDRESS_OF(AK2));
    yaw_error();
}

/* ATTRATES +076: the tag's yaw jet code, "RCS JET BITS", into JETEM; then
 * the pitch rate's tag (2D/SDZ, back at EXDAPIN). */
void atm_yjetcode(void)
{
    jet_code(YJETCODE); /* INDEX A; CAF YJETCODE */
    ts(ADDRESS_OF(JETEM));
    ca(ADDRESS_OF(QREL));
    tc_at(ATTRATES + 0102u);
    two_d_sdz();
}

/* EXDAPIN: "COME HERE FROM EX ATM DAP": the tag's pitch jet code added,
 * "COMBINE ALL NEW BITS.", and out on channel 5: "DOES NOT REQUIRE SAVING
 * OLD CODES.", "SET PYCHAN TO DESIRED BIT CONFIG." Then CCS JETAG:
 * positive, CM/RCS; +0, CM/FDAI; negative, CM/FDAIR -1; -0, on into 3DDZ
 * with +0. */
void exdapin(void)
{
    jet_code(P_RJCODE); /* INDEX A; CAF P/RJCODE */
    ads(ADDRESS_OF(JETEM));
    write_channel(5); /* EXTEND; WRITE PYJETS */
    switch (ccs_at(ADDRESS_OF(JETAG))) {
    case POSITIVE: tc_to(EXDAPIN + 6u, CM_RCS); break;
    case IS_PLUS_ZERO: exdapin_fdai(); break;
    case NEGATIVE: exdapin_fdair(); break;
    case IS_MINUS_ZERO: three_d_dz(); break;
    }
}

/* EXDAPIN +7: the needles (CM/FDAI). */
void exdapin_fdai(void)
{
    tc_to(EXDAPIN + 7u, CM_FDAI);
}

/* EXDAPIN +010: the needles, from the word before CM/FDAIR. */
void exdapin_fdair(void)
{
    tc_to(EXDAPIN + 010u, CM_FDAIR - 1u);
}

#endif
