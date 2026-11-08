/* SPDX-License-Identifier: MIT */
/* RCS-CSM_DIGITAL_AUTOPILOT.agc (Comanche055 pp. 1012-1016, bank 21):
 * REINIT, the "TILT LOGIC" that restarts the autopilot, and the
 * "INITIALIZATION PROGRAM FOR RCS-CSM AUTOPILOT": FRESHDAP and REDAP,
 * which decode the crew's DAP data, zero the autopilot's erasable words and
 * the jets (ZEROJET), and start the T5 program again. The faithful C is
 * c64-port/src/cm_rcs_dap.c. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* REINIT: "REINITIALIZE DAP IN 200MS": TIME5 to DELAY200 and T5PHASE +
 * (the next T5 interrupt runs FRESHDAP), and RESUME. */
void reinit(void)
{
    set_A(DELAY200);
    ts(ADDRESS_OF(TIME5));
    ts(ADDRESS_OF(T5PHASE));
    continue_at(RESUME);
}

/* FRESHDAP: "RESET HOLDFLAG TO STOP AUTOMATIC STEERING AND PREPARE TO PICK
 * UP AN ATTITUDE HOLD REFERENCE", then REDAP. */
void freshdap(void)
{
    set_A(ONE);
    ts(ADDRESS_OF(HOLDFLAG));
    continue_at(REDAP);
}

/* REDAP: S41.2 by IBNKCALL, to "DECODE DAPDATR1, DAPDATR2 FOR DEADBANDS"
 * and "RATES, QUADFAILS, QUAD MANAGEMENT". */
void redap(void)
{
    tc_to(REDAP, IBNKCALL); /* CADR S41.2 */
}

/* REDAP +2: S40.14 by IBNKCALL, to "DECODE IXX, IAVG, AND CONVERT TO
 * AUTOPILOT GAINS". */
void redap_s40_14(void)
{
    tc_to(REDAP + 2u, IBNKCALL); /* CADR S40.14 */
}

/* REDAP +4: "ZERO ALL NECESSARY ERASABLE REGISTERS", the NO.T5VAR + 1
 * words from WBODY on (the rates, the filter's words, the attitude errors),
 * then the jets (TCR ZEROJET). The loop counts SPNDX down from 36, so CCS
 * SPNDX ends it at +0. */
void redap_zerot5(void)
{
    set_A(NO_T5VAR);
    do {
        ts(ADDRESS_OF(SPNDX)); /* ZEROT5 */
        set_A(ZERO);
        ts(index_ts(ADDRESS_OF(WBODY), spndx()));
    } while (ccs_at(ADDRESS_OF(SPNDX)) == POSITIVE); /* TCF ZEROT5 */
    tc_at(ZEROT5 + 6u); /* TCR ZEROJET */
    zerojet();
}

/* REDAP +014, after ZEROJET: CS ZERO for CHANTEMP. */
void redap_after_zerojet(void)
{
    set_A(negate(ZERO));
    redap_chantemp();
}

/* REDAP +015: "INITIALIZE MINIMUM IMPULSE CONTROL" (CHANTEMP) and the "RHC
 * POSITION MEMORY FOR MANUAL RATE MODES" (CH31TEMP), each -0; the
 * "SWITCHING LOGIC SLOPE"; T5TIME 4 ("PHASE 1 (RATEFILTER) BEGINS CYCLING
 * 100 MS FROM NOW AND EVERY 100MS THEREAFTER"); ATTKALMN 11, "TO PICK UP
 * KALMAN FILTER GAINS TO INITIALIZE THE S/C ANGULAR RATES"; the CDU angles
 * as the rate filter's past values; and T5PHASE +0, "TO BEGIN EXECUTING
 * PHASE2 OF PROGRAM". (TS CHANTEMP overflows only for an A that holds an
 * overflow, and skips TS CH31TEMP.) */
void redap_chantemp(void)
{
    if (!ts(ADDRESS_OF(CHANTEMP))) ts(ADDRESS_OF(CH31TEMP));
    set_A(SLOPE_24);
    ts(ADDRESS_OF(SLOPE));
    set_A(FOUR);
    ts(ADDRESS_OF(T5TIME));
    set_A(ELEVEN);
    ts(ADDRESS_OF(ATTKALMN));
    ca(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(RHO));
    ca(ADDRESS_OF(CDUY));
    ts(ADDRESS_OF(RHO1));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(RHO2));
    set_A(ZERO);
    ts(ADDRESS_OF(T5PHASE));
    redap_imu();
}

/* RCSSWIT: RCSFLAGS set (RCSINIT or RCSINITB: "CLEAR BIT1 -INITIALIZE T6
 * PROGRAM", "SET BIT3 -INITIALIZE NEEDLES", "CLEAR BIT4 -RESET FOR
 * FDAIDSP1", and bit 14 as the IMU allows), and the "NEXT T5RUPT 60 MS
 * FROM NOW TO ALLOW IMU ERROR COUNTER TO ZERO". */
static void rcsswit(void)
{
    ts(ADDRESS_OF(RCSFLAGS));
    set_A(T5WAIT60);
    ts(ADDRESS_OF(TIME5));
    tc_to(RCSSWIT + 3u, RESUME);
}

/* REDAP +031: "CHECK IMU STATUS" ("IF BIT6 = 0 IMU IN FINE ALIGN"). With
 * the IMU ready, IMUAOK: "START MATRIX INITIALIZATION" (AMBGUPDT, by
 * NOVAC). If not ("CANNOT USE IMU"), ATTKALMN +0 and RCSINITB: "PROVIDE
 * FREE CONTROL ONLY", "DONT START UP RATE FILTER". (MASK BIT6 leaves A +
 * or +0.) */
void redap_imu(void)
{
    cs(ADDRESS_OF(IMODES33));
    mask(BIT6);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        set_A(PRIO34); /* IMUAOK */
        tc_to(IMUAOK + 1u, NOVAC);
        return;
    }
    ts(ADDRESS_OF(ATTKALMN));
    set_A(RCSINITB);
    rcsswit();
}

/* IMUAOK +4, after NOVAC: "CLEAR BIT14 -ASSUME WE HAVE A GOOD IMU". */
void imuaok_after_novac(void)
{
    set_A(RCSINIT);
    rcsswit();
}

/* ZEROJET: "ZERO BLAST2, BLAST1, BLAST, YWORD2, YWORD1, PWORD2, PWORD1,
 * RWORD2, AND RWORD1" (the 12 words from RWORD1 on, counted down by
 * SPNDX), then the jet timing words BLAST1 +1 and BLAST2 +1, "RESET BIT1
 * OF RCSFLAGS TO 0", and T6, the 2CADR T6ADDR of T6START in T6LOC: "ENABLE
 * T6RUPT TO SHUT OFF JETS IN 14 MS" (bit 15 of channel 13). */
void zerojet(void)
{
    set_A(ELEVEN);
    do {
        ts(ADDRESS_OF(SPNDX));
        set_A(ZERO);
        ts(index_ts(ADDRESS_OF(RWORD1), spndx()));
    } while (ccs_at(ADDRESS_OF(SPNDX)) == POSITIVE); /* TCF ZEROJET +1 */
    set_A(FOUR);
    ts(ADDRESS_OF(BLAST1) + 1u);
    set_A(ELEVEN);
    ts(ADDRESS_OF(BLAST2) + 1u);
    set_A(negate(BIT1)); /* CS */
    mask_at(ADDRESS_OF(RCSFLAGS));
    ts(ADDRESS_OF(RCSFLAGS));
    dca(ADDRESS_OF(T6ADDR));
    dxch(ADDRESS_OF(T6LOC));
    set_A(PLUS_14MS);
    ts(ADDRESS_OF(TIME6));
    set_A(BIT15);
    wor_channel(013);
    continue_at(ZEROJET + 026u);
}

/* ZEROJET +026: TC Q. */
void zerojet_return(void)
{
    tc_q();
}

#endif
