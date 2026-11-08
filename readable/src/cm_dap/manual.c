/* SPDX-License-Identifier: MIT */
/* RCS-CSM_DIGITAL_AUTOPILOT.agc (Comanche055 pp. 1016-1022, bank 21): the
 * start of T5PHASE2, the second phase of the T5 program, and its "MANUAL
 * ROTATION COMMANDS": the RHC's rotation switches in channel 31 read for
 * changes (each change sets the axis' forced firing bits), then, with the
 * RHC "OUT OF DETENT" and the manual rate modes, each axis' commanded rate
 * in WBODY from MANTABLE and the attitude error it builds up in MERRORX,
 * ERRORX for the jet selection logic (JETS); in free mode, FREEFUNC's jet
 * firing times instead. STICKCHK decodes the RHC's bits, and KALUPDT
 * counts the rate filter's first passes. The faithful C is
 * c64-port/src/cm_rcs_dap.c. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* ---- T5PHASE2 -- p. 1016 ---- */

/* T5PHASE2: "IF (+) INITIALIZE RATE ESTIMATE" (KALUPDT). If not, TIME5
 * gets DELTATT2, "RESET FOR PHASE3 IN 20 MS (JET SELECTION LOGIC)", the
 * old TIME5 added into T5TIME "TO COMPENSATE FOR DELAYS IN T5RUPT", and
 * ATTKALMN -1 for "A HIGH RATE AUTO MANEUVER" (bit 15 of RCSFLAGS), +0
 * otherwise. */
void t5phase2(void)
{
    if (ccs_at(ADDRESS_OF(ATTKALMN)) == POSITIVE) {
        continue_at(KALUPDT);
        return;
    }
    set_A(DELTATT2);
    xch(ADDRESS_OF(TIME5));
    ads(ADDRESS_OF(T5TIME));
    ca(ADDRESS_OF(RCSFLAGS));
    mask(BIT15);
    if (A_is_zero()) /* EXTEND; BZF NOHIAUTO */
        nohiauto();
    else
        t5phase2_hiauto();
}

/* NOHIAUTO -1: CS ONE. */
void t5phase2_hiauto(void)
{
    set_A(negate(ONE));
    nohiauto();
}

/* NOHIAUTO: TS ATTKALMN, then "RESET FORCED FIRING BITS (BITS 10 TO 5 OF
 * RCSFLAGS) TO ZERO". Channel 31's rotation bits (MANROT) are compared
 * with CH31TEMP, the last ones seen: on a change, "SAVE CONTENTS OF
 * CHANNEL 31 IN CH31TEMP", "SET FORCED FIRING BITS FOR AXES WITH CHANGES
 * IN COMMAND" (the changed bits, times BIT5, in L) and "SET RATE DAMPING
 * FLAGS". (TS ATTKALMN overflows only for an A that holds an overflow,
 * and skips the CS OCT01760: A is then +1 or -1. The channel's word,
 * sign-extended, leaves TS CH31TEMP no overflow.) */
void nohiauto(void)
{
    if (!ts(ADDRESS_OF(ATTKALMN))) set_A(negate(OCT01760)); /* CS */
    mask_at(ADDRESS_OF(RCSFLAGS));
    ts(ADDRESS_OF(RCSFLAGS));
    read_channel(031);
    ts(L_ADDRESS);
    ca(ADDRESS_OF(CH31TEMP));
    rxor_lchan();
    mask(MANROT);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF NOCHANGE */
        nochange();
        return;
    }
    lxch(A_ADDRESS);
    ts(ADDRESS_OF(CH31TEMP));
    ca(L_ADDRESS);
    mp(BIT5); /* "PUT BITS 6-1 OF A IN BITS 10-5 OF L" */
    ca(L_ADDRESS);
    ads(ADDRESS_OF(RCSFLAGS));
    cs(ADDRESS_OF(RCSFLAGS));
    nohiauto_damping();
}

/* NOHIAUTO +025: MASK OCT16000; ADS RCSFLAGS: the rate damping flags,
 * "BITS 13,12,AND 11 OF RCSFLAGS", set. */
void nohiauto_damping(void)
{
    mask(OCT16000);
    ads(ADDRESS_OF(RCSFLAGS));
    nochange();
}

/* ---- NOCHANGE, the RHC out of detent -- pp. 1017-1018 ---- */

/* NOCHANGE: "IF NO MANUAL COMMANDS, GO TO AHFNOROT". If any, "SET
 * HOLDFLAG +" and call STICKCHK, which sets PMANNDX, YMANNDX and RMANNDX
 * "TO 0, 1, OR 2 FOR NO, +, OR - ROTATION". */
void nochange(void)
{
    cs(ADDRESS_OF(CH31TEMP));
    mask(MANROT);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF AHFNOROT */
        continue_at(AHFNOROT);
        return;
    }
    ts(ADDRESS_OF(HOLDFLAG));
    tc_to(NOCHANGE + 5u, STICKCHK);
}

/* NOCHANGE +6, after STICKCHK: "SET STIKFLAG TO INFORM STEERING PROGRAMS
 * (P20) THAT ASTRONAUT HAS ASSUMED ROTATIONAL CONTROL OF SPACECRAFT" (bit
 * 14 of FLAGWRD1). With bit 14 of channel 31 clear, FREEFUNC (free
 * mode). With the rate filter not yet initialized (bit 14 of RCSFLAGS
 * set), the "TILT, RECYCLE TO INITIALIZE FILTER": REINIT. Then ATTKALMN -1
 * "IF MANUAL MANEUVER IS AT HIGH RTE" (RATEINDX above 5), and SETWBODY for
 * the three axes, SPNDX 2, 1 and 0. */
void nochange_after_stickchk(void)
{
    cs(ADDRESS_OF(FLAGWRD1));
    mask(BIT14);
    ads(ADDRESS_OF(FLAGWRD1));
    set_A(BIT14);
    rand_channel(031);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF FREEFUNC */
        continue_at(FREEFUNC);
        return;
    }
    ca(ADDRESS_OF(RCSFLAGS));
    mask(BIT14);
    if (ccs_register(A_ADDRESS) == POSITIVE) { /* CCS A; TCF REINIT */
        continue_at(REINIT);
        return;
    }
    set_A(negate(FIVE)); /* CS */
    ad_at(ADDRESS_OF(RATEINDX));
    if (!A_is_zero_or_minus()) { /* EXTEND; BZMF +3 */
        set_A(negate(ONE));      /* CS */
        ts(ADDRESS_OF(ATTKALMN));
    }
    set_A(TWO); /* "AUTO-HOLD MANUAL ROTATION" */
    continue_at(SETWBODY);
}

/* ---- SETWBODY, each axis' commanded rate -- pp. 1018-1019 ---- */

/* SETWBODY: the axis in SPNDX, twice it in DPNDX. (TS SPNDX overflows only
 * for an A that holds an overflow, and skips the DOUBLE: A is then +1 or
 * -1, and TS DPNDX does not overflow.) */
void setwbody(void)
{
    if (!ts(ADDRESS_OF(SPNDX))) double_A();
    setwbody_dpndx();
}

/* SETWBODY +2: TS DPNDX, which skips the INDEX SPNDX when it overflows. */
void setwbody_dpndx(void)
{
    if (ts(ADDRESS_OF(DPNDX)))
        setwbody_rmanndx_unindexed();
    else
        setwbody_rmanndx();
}

/* The axis' rotation command: "IF NO ROTATION COMMAND ON THIS AXIS, GO TO
 * NORATE". If one, the MANTABLE entry for it and the rate (RATEINDX), "BY
 * 2 TO THE -6 TO GET COMMANDED RATE", in WBODY ("SET WBODY TO COMMANDED
 * RATE"), and MERUPDAT if "RATE DAMPING COMPLETED" (bits 13, 12 and 11 of
 * RCSFLAGS all clear), ZEROER if not. The entry's index is kept in Q for
 * MERUPDAT. (TS Q overflows only for a sum that overflows, and skips the
 * INDEX Q.) */
static void commanded_rate(void)
{
    if (A_is_zero()) { /* EXTEND; BZF NORATE */
        norate();
        return;
    }
    ad_at(ADDRESS_OF(RATEINDX));
    if (ts(Q_ADDRESS))
        ca(ADDRESS_OF(MANTABLE) - 1u);
    else
        ca(index_read(ADDRESS_OF(MANTABLE) - 1u, overflow_corrected(Q)));
    mp(BIT9);
    setwbody_wbody();
}

/* SETWBODY +3: INDEX SPNDX; CA RMANNDX ("RMANNDX = 0 NO ROTATION", "= 1 +
 * ROTATION", "= 2 - ROTATION", and PMANNDX and YMANNDX the same, in the
 * words after it). */
void setwbody_rmanndx(void)
{
    ca(index_read(ADDRESS_OF(RMANNDX), spndx()));
    commanded_rate();
}

/* SETWBODY +4: CA RMANNDX, not indexed. */
void setwbody_rmanndx_unindexed(void)
{
    ca(ADDRESS_OF(RMANNDX));
    commanded_rate();
}

/* SETWBODY +015: INDEX DPNDX; DXCH WBODY, then the test of the rate
 * damping flags. */
void setwbody_wbody(void)
{
    dxch_indexed(ADDRESS_OF(WBODY), dpndx());
    ca(ADDRESS_OF(RCSFLAGS));
    mask(OCT16000);
    if (A_is_zero()) /* EXTEND; BZF MERUPDAT */
        merupdat();
    else
        zeroer();
}

/* ZEROER: "ZEROER ZEROS MERRORS", the axis' MERRORX. */
void zeroer(void)
{
    set_A(ZERO);
    zl();
    dxch_indexed(ADDRESS_OF(MERRORX), dpndx());
    continue_at(SPNDXCHK);
}

/* NORATE: "ZERO WBODY FOR THIS AXIS". "IS RATE DAMPING COMPLETED": if so,
 * "KEEP CURRENT MERRORX", and if not, ZEROER. */
void norate(void)
{
    zl();
    dxch_indexed(ADDRESS_OF(WBODY), dpndx());
    ca(ADDRESS_OF(RCSFLAGS));
    mask(OCT16000);
    if (A_is_zero()) /* EXTEND; BZF SPNDXCHK */
        continue_at(SPNDXCHK);
    else
        zeroer();
}

/* MERUPDAT: "MERRORX=MERRORX+MEASURED CHANGE IN ANGLE -COMMANDED CHANGE IN
 * ANGLE"; the measured change is already in ("HAS ALREADY BEEN DONE IN THE
 * RATE FILTER"), and the commanded change, "WBODY TIMES .1SEC = MANTABLE
 * ENTRY TIMES 2 TO THE -8", is taken off. */
void merupdat(void)
{
    cs(index_read(ADDRESS_OF(MANTABLE) - 1u, overflow_corrected(Q)));
    mp(BIT7);
    das_indexed(ADDRESS_OF(MERRORX), dpndx());
    continue_at(SPNDXCHK);
}

/* ---- SPNDXCHK, the next axis -- p. 1019 ---- */

/* CCS SPNDX: the next axis, or, after the last, JETS, the jet selection
 * logic. A negative SPNDX or -0 would run the constants after TCF JETS as
 * orders: the faithful port refuses that, and it is not modelled here. */
static void next_axis(void)
{
    switch (ccs_at(ADDRESS_OF(SPNDX))) {
    case POSITIVE: continue_at(SETWBODY); break;
    case IS_PLUS_ZERO: continue_at(JETS); break;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled(); break;
    }
}

/* SPNDXCHK: "ERRORX = HIGH ORDER WORD OF MERRORX". (The CA leaves A no
 * overflow, so TS ERRORX does not skip. An index that has the CA read A
 * or Q itself, which can hold one, does not come up in the program; the
 * faithful port goes on to CCS SPNDX then too, and so does this.) */
void spndxchk(void)
{
    ca(index_read(ADDRESS_OF(MERRORX), dpndx()));
    ts(index_ts(ADDRESS_OF(ERRORX), spndx()));
    next_axis();
}

/* SPNDXCHK +2: INDEX SPNDX; TS ERRORX, which skips to TCF SETWBODY when
 * it overflows. */
void spndxchk_errorx(void)
{
    if (ts(index_ts(ADDRESS_OF(ERRORX), spndx()))) {
        continue_at(SETWBODY);
        return;
    }
    next_axis();
}

/* ---- FREEFUNC and STICKCHK -- pp. 1019-1021 ---- */

/* FREEFUNC, with the RHC in free mode (bit 14 of channel 31 clear): the
 * acceleration commands, each axis' jet firing time from FREETAU by its
 * index ("0 SEC", "+0.10 SEC", "-0.10 SEC", "(+3) 0 SEC") in TAU, TAU1 and
 * TAU2, then T6PROGM. (Each CA reads a word with no overflow, but for an
 * index that has it read A or Q itself, which does not come up in the
 * program; the faithful port goes on past the TS then too, and so does
 * this.) */
void freefunc(void)
{
    ca(index_read(ADDRESS_OF(FREETAU), fetch(ADDRESS_OF(RMANNDX))));
    ts(ADDRESS_OF(TAU));
    ca(index_read(ADDRESS_OF(FREETAU), fetch(ADDRESS_OF(PMANNDX))));
    ts(ADDRESS_OF(TAU1));
    ca(index_read(ADDRESS_OF(FREETAU), fetch(ADDRESS_OF(YMANNDX))));
    ts(ADDRESS_OF(TAU2));
    t6progm();
}

/* T6PROGM: the attitude errors zeroed "FOR MANUAL ROTATIONS", and T6PROG
 * (JET_SELECTION_LOGIC.agc) to fire the jets for TAU, TAU1 and TAU2. */
void t6progm(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(ERRORX));
    ts(ADDRESS_OF(ERRORY));
    ts(ADDRESS_OF(ERRORZ));
    continue_at(T6PROG);
}

/* STICKCHK: the "INDICES FOR MANUAL ROTATION" from the RHC's bits in A
 * (CH31TEMP's, complemented), two to an axis: "MAN RATE 0 0 RATE (DP)",
 * "+1 +RATE (DP)", "+2 -RATE (DP)", "(+3) 0 RATE (DP)". Bits 2-1 go to
 * PMANNDX, and MP QUARTER shifts the rest down two bits for YMANNDX, then
 * RMANNDX. (TS T5TEMP overflows only for an A that holds an overflow, and
 * skips the MASK THREE: A is then +1 or -1.) */
void stickchk(void)
{
    if (!ts(ADDRESS_OF(T5TEMP))) mask(THREE);
    ts(ADDRESS_OF(PMANNDX));
    ca(ADDRESS_OF(T5TEMP));
    mp(QUARTER);
    ts(ADDRESS_OF(T5TEMP));
    mask(THREE);
    ts(ADDRESS_OF(YMANNDX));
    ca(ADDRESS_OF(T5TEMP));
    mp(QUARTER);
    ts(ADDRESS_OF(RMANNDX));
    tc_q();
}

/* ---- KALUPDT, the rate filter started -- pp. 1021-1022 ---- */

/* KALUPDT: from T5PHASE2's CCS ATTKALMN, A is ATTKALMN less one, kept as
 * the count of passes: the "INITIALIZATION OF ATTITUDE RATES USING KALMAN
 * FILTER TAKES 1.1 SEC". The next T5 interrupt comes in DELTATT's "80MS
 * (TIME5)" (the comment here reads "=1SEC - 80MS") "+ DELAYS" (T5TIME);
 * if that overflows TIME5, "SAFETY PLAY TO ASSURE A T5RUPT": DELTATT2.
 * Then KRESUME2. (TS ATTKALMN
 * overflows only for an A that holds an overflow, and skips the CA
 * DELTATT: A is then +1 or -1.) */
void kalupdt(void)
{
    if (!ts(ADDRESS_OF(ATTKALMN))) set_A(DELTATT);
    ad_at(ADDRESS_OF(T5TIME));
    if (ts(ADDRESS_OF(TIME5))) { /* skips TCF +3 */
        set_A(DELTATT2);
        ts(ADDRESS_OF(TIME5));
    }
    set_A(negate(ZERO)); /* KRESUME2: CS ZERO */
    continue_at(KRESUME2 + 1u);
}

/* KRESUME2 +1: T5PHASE -0, "RESET FOR PHASE1", and "RESUME INTERRUPTED
 * PROGRAM". (TS T5PHASE overflows only for an A that holds an overflow,
 * and skips the TCF RESUME: on to FDAIDSP2.) */
void kresume2_t5phase(void)
{
    if (ts(ADDRESS_OF(T5PHASE)))
        fdaidsp2();
    else
        continue_at(RESUME);
}

#endif
