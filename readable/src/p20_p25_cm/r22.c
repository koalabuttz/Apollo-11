/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 5 (Comanche055 pp. 579-582): R22, the "RENDEZVOUS
 * TRACKING DATA PROCESSING ROUTINE", up to REND12 (bank 34).
 *
 * P20 starts R22 as a job of its own, which runs as long as P20 tracks.
 * It takes two kinds of data: the crew's optics marks, which R21 leaves in
 * MRKBUF2, and the VHF ranging's range, which R22 reads itself once a
 * minute (RANGERD) while VHFRFLAG is set. Each one updates the state
 * vector of one of the two vehicles (VEHUPFLG set: the CSM's; clear: the
 * LM's) by one measurement, through the W matrix (INCORP1).
 *
 * For each measurement R22 integrates both state vectors to its time
 * (SETINTG and INTEGRV; the vehicle it updates last, with its W matrix
 * when that is valid), takes the line of sight RCLP from the CSM to the
 * LM, and works out what the measurement should have been: the B vector
 * and DELTAQ (BVECTORS for a mark, REND14 for a range) and the variance
 * of its error. An optics mark is taken in two passes (INCORFLG set on
 * the first), two measurements at right angles to each other. If the
 * first pass's update is larger than RMAX or VMAX, the crew is asked
 * (RENDISP, verb 06 noun 49); else INCORP2 (REND12, the next part) puts
 * it into the state vector.
 *
 * Each call out of the file (PRIOCHNG, GENTRAN) leaves Z at the routine
 * and Q at the word after the TC, where the routine comes back to (GENTRAN
 * past its two ADRES words). */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void rend2(void);
static void rend3a(void);

/* R22 -- p. 579. The job's priority 26, kept in PHSPRDT2 for a restart
 * (PRIOCHNG sets it). (A CAF leaves no overflow for the TS to skip on.) */
void r22(void)
{
    set_A(PRIO26);
    if (ts(ADDRESS_OF(PHSPRDT2))) not_modelled();
    tc_to(R22 + 2u, PRIOCHNG);
}

/* R22 +3: MRKBUF2 -3, no mark in it, and on in interpretive code. (A CA
 * of a constant leaves no overflow for the TS to skip on.) */
void r22_after_priochng(void)
{
    set_A(NEG3);
    if (ts(ADDRESS_OF(MRKBUF2))) not_modelled();
    tc_at(R22 + 5u);
    intpret();
}

/* R22 +6: VHFTIME, the "PRESENT TIME", so that the first range is read a
 * minute on. REND1: a wait (WAITONE: four seconds, and again until P20
 * allows an update, CONTCHK), and out of the interpreter. */
static const word r22_words[] = {
    ORDER(RTB),
    ADDRESS(LOADTIME),
    STORE(VHFTIME), /* "PRESENT TIME" */
    /* REND1 */
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDER(CALL),
    ADDRESS(WAITONE),
    /* REND1A */
    ORDER(EXIT),
};

/* REND1A +1 -- after the EXIT: MRKBUF2's first word, the mark's time
 * (TIME2), +0 or positive, a new mark, to REND2; negative (-1 none, -3
 * used), no mark, to REND3A. */
void rend1a_after_exit(void)
{
    ca(ADDRESS_OF(MRKBUF2));
    if (A_is_zero()) { /* EXTEND; BZF REND2 */
        rend2();
        return;
    }
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF REND3A */
        rend3a();
        return;
    }
    rend2();
}

/* REND2: the mark, six words, from MRKBUF2 to MARKTIME and on ("MARKTIME
 * MUST BE CONTIGUOUS WITH VTEMP") by GENTRAN, */
static void rend2(void)
{
    set_A(SIX);
    tc_to(REND2 + 1u, GENTRAN); /* ADRES MRKBUF2; ADRES MARKTIME */
}

/* REND2 +4: and MRKBUF2 -3 again, "NEG VALUE TO INDICATE VALUES USED".
 * GENTRAN holds interrupts; RELINT releases them. (A CAF leaves no
 * overflow for the TS to skip on.) */
void rend2_after_gentran(void)
{
    set_A(NEG3);
    if (ts(ADDRESS_OF(MRKBUF2))) not_modelled();
    relint();
    tc_at(REND2 + 7u);
    intpret();
}

/* REND2 +010: SOURCFLG clear, "0 = OPTICS DATA", and MARKDATA the mark's
 * address (VTEMP -2, MARKTIME), for GETUM. Then on at REND4. */
static const word rend2_words[] = {
    ORDERS(CLEAR, CALL),
    /* "0 = OPTICS DATA" */
    SWITCH(CLEAR, SOURCFLG),
    ADDRESS(GRP2PC),
    ORDERS(SSP, GOTO),
    ADDRESS_PLUS_ONE(MARKDATA),
    /* ECADR VTEMP -2 */
    ADDRESS_AT(VTEMP, -2),
    ADDRESS(REND4),
};

/* REND3A: no mark; on in interpretive code, at REND3. */
static void rend3a(void)
{
    tc_at(REND3A);
    intpret();
}

/* REND3 -- pp. 579-580: a wait, and then a range if VHFRFLAG is set and a
 * minute has gone by since VHFTIME, the "LAST READING OF RADAR" (else back
 * to REND1A, to look for a mark again). RANGERD reads it ("READ RADAR
 * RANGE"), with its time in MARKTIME, kept in VHFTIME "FOR DOWNLINK". */
static const word rend3_words[] = {
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDER(CALL),
    ADDRESS(WAITONE),
    ORDER(BOFF),
    SWITCH(BOF, VHFRFLAG),
    ADDRESS(REND1A),
    ORDER(RTB),
    ADDRESS(LOADTIME), /* "PRESENT TIME" */
    ORDERS(DSU, DSU),
    ADDRESS_PLUS_ONE(60SECDP), /* "1 MIN" */
    ADDRESS_PLUS_ONE(VHFTIME), /* "LAST READING OF RADAR" */
    ORDERS(BMN, CALL),
    ADDRESS(REND1A),
    ADDRESS(RANGERD), /* "READ RADAR RANGE" */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(MARKTIME),
    STORE(VHFTIME), /* "FOR DOWNLINK" */
};

/* REND4 -- p. 580: both state vectors to the measurement's time. SETINTG
 * ("SET INTEGRV FLAGS") sets VINTFLAG, the CSM. For an LM update, the CSM
 * first (INTEGRV), then the LM ("SET INTEGRATION VEHICLE TO LM"); for a
 * CSM update ("BRANCH IF CSM UPDATE"), CSMUPP the other way round. REND5:
 * the vehicle updated last, with its W matrix too (DIM0FLAG) if RENDWFLG
 * says it is valid ("DO NOT INTEGRATE W IF FLAG = 0"). */
static const word rend4_words[] = {
    ORDER(CALL),
    ADDRESS(SETINTG), /* "SET INTEGRV FLAGS" */
    ORDERS(BON, CALL),
    SWITCH(BON, VEHUPFLG),
    ADDRESS(CSMUPP), /* "BRANCH IF CSM UPDATE" */
    ADDRESS(INTEGRV),
    ORDER(CALL),
    ADDRESS(GRP2PC), /* "PHASE CHANGE" */
    ORDER(CALL),
    ADDRESS(SETINTG), /* "SET INTEGRV FLAGS" */
    ORDER(CLEAR),
    SWITCH(CLEAR, VINTFLAG), /* "SET INTEGRATION VEHICLE TO LM" */
    /* REND5 */
    ORDERS(BOFF, SET),
    SWITCH(BOF, RENDWFLG),
    ADDRESS(REND5A), /* "DO NOT INTEGRATE W IF FLAG = 0" */
    SWITCH(SET, DIM0FLAG),
    /* REND5A */
    ORDER(CALL),
    ADDRESS(INTEGRV),
    ORDER(CALL),
    ADDRESS(SHIFTNDX), /* "SET EARTH MOON SCALING INDEX" */
    ORDER(CALL),
    ADDRESS(CMPOS), /* "SET CSM POSITION" */
    ORDERS(SET, CALL),
    SWITCH(SET, INCORFLG), /* "SET FOR 1ST PASS" */
    ADDRESS(LMPOS),        /* "SET LM POSITION" */
    ORDERS(CLEAR, BON),
    SWITCH(CLEAR, ORBWFLAG), /* "CLEAR FOR ORBITAL AND CISLUNAR" */
    SWITCH(BON, RENDWFLG),
    ADDRESS(REND6),
    /* The W matrix not valid: a new one, from WRENDPOS and WRENDVEL (in
     * push-down words 0 and 1), and the counts of updates +0 ("ZERO OUT
     * VHFCNT AND TRKMKCNT"). */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(WRENDPOS),
    STCALL_D(0),
    ADDRESS(INITIALW),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(ZEROVECS),
    STORE(VHFCNT), /* "ZERO OUT VHFCNT AND TRKMKCNT" */
};

/* REND6 -- p. 581: RENDWFLG set, and RCLP, "LM - CSM". REND7: its unit
 * vector; a range goes to REND14 ("BRANCH IF DATA IS RADAR"). A mark's
 * line of sight from the state vectors is UCL; on the first pass, UM the
 * one measured ("CALCULATE UM LINE OF SIGHT", GETUM), and USTAR the unit
 * vector at right angles to both ("UCL X UM"), which BVECTORS works from.
 * If they are too near parallel for a unit vector (BOV, after VXV and
 * after UNIT), "BRANCH IF OVERFLOW IGNORE MARK". */
static const word rend6_words[] = {
    ORDER(SET),
    SWITCH(SET, RENDWFLG),
    ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(LEMPOS),
    ADDRESS_PLUS_ONE(CSMPOS),
    STORE(RCLP), /* "LM - CSM" */
    /* REND7 */
    ORDERS(UNIT, BON),
    SWITCH(BON, SOURCFLG),
    ADDRESS(REND14), /* "BRANCH IF DATA IS RADAR" */
    STORE(UCL),
    ORDERS(BOFF, CALL),
    SWITCH(BOF, INCORFLG),
    ADDRESS(REND9),
    ADDRESS(GETUM), /* "CALCULATE UM LINE OF SIGHT" */
    STOVL(UM),
    ADDRESS_PLUS_ONE(UCL),
    ORDERS(VXV, BOV),
    ADDRESS_PLUS_ONE(UM), /* "UCL X UM" */
    ADDRESS(REND8),
    /* REND8 */
    ORDERS(UNIT, BOV),
    ADDRESS(REND3), /* "BRANCH IF OVERFLOW IGNORE MARK" */
    STORE(USTAR),
};

/* REND9: the B vector and DELTAQ (BVECTORS). For a CSM update the B
 * vector turns round (VCOMP): RCLP is the LM's position less the CSM's,
 * so moving the CSM moves it the other way. REND9A: the error's
 * variance, the sextant's and the IMU's (SXTVAR and IMUVAR), or for R23's
 * marks ("BRANCH IF BACKUP OPTICS (R23 WORKING)") REND15's. */
static const word rend9_words[] = {
    ORDER(CALL),
    ADDRESS(BVECTORS),
    ORDERS(BON, VLOAD),
    SWITCH(BON, VEHUPFLG),
    ADDRESS(REND9A),
    ADDRESS_PLUS_ONE(BVECTOR),
    ORDER(VCOMP),
    STORE(BVECTOR),
    /* REND9A */
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDER(BON),
    SWITCH(BON, R23FLG),
    ADDRESS(REND15), /* "BRANCH IF BACKUP OPTICS (R23 WORKING)" */
    ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(SXTVAR),
    ADDRESS_PLUS_ONE(IMUVAR),
};

/* REND10 -- pp. 581-582: the variance of an angle (in VARIANCE, "TEMP
 * STORAGE FOR VARIANCE CALC.") times RCLP's length squared, a distance's,
 * shifted to B-40 by NORM's count (twice, for the square) and the "EARTH
 * MOON SCALING INDEX" (X2, SHIFTNDX); plus INTVAR, the "INTEGRATION VARIANCE SCALED B-15" (shifted
 * to B-40). A range's is at least RVARMIN, the "VHF RADAR MIN. VARIANCE",
 * which is kept negative ("MIN. VALUE WAS STORED AS NEG."). */
static const word rend10_words[] = {
    STOVL(VARIANCE), /* "TEMP STORAGE FOR VARIANCE CALC." */
    ADDRESS_PLUS_ONE(RCLP),
    ORDERS(ABVAL, NORM),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DSQ, DMP),
    ADDRESS_PLUS_ONE(VARIANCE),
    ORDERS(XAD_1, CALL),
    ADDRESS(X1),
    ADDRESS(SHIFTNDX), /* "GET EARTH MOON SCALING INDEX" */
    ORDERS(XAD_1, XAD_1),
    ADDRESS(X2),
    ADDRESS(X2),
    ORDERS(SR_STAR, TLOAD),
    SR_X1(-2), /* "ADJUST SCALING TO B-40" */
    ADDRESS_PLUS_ONE(MPAC),
    STORE(VARIANCE),
    ORDERS(SLOAD, SR),
    ADDRESS_PLUS_ONE(INTVAR), /* "INTEGRATION VARIANCE SCALED B-15" */
    SR_COUNT(25),             /* "SCALE IT B-40" */
    ORDERS(TAD, RTB),
    ADDRESS_PLUS_ONE(VARIANCE),
    ADDRESS(TPMODE),
    STORE(VARIANCE),
    ORDERS(BOFF, TAD),
    SWITCH(BOF, SOURCFLG), /* "BRANCH IF NOT VHF RADAR" */
    ADDRESS(REND10A),
    ADDRESS_PLUS_ONE(RVARMIN), /* "VHF RADAR MIN. VARIANCE" */
    ORDERS(BPL, TLOAD),
    ADDRESS(REND10A),
    ADDRESS_PLUS_ONE(RVARMIN),
    ORDER(ABS),      /* "MIN. VALUE WAS STORED AS NEG." */
    STORE(VARIANCE), /* "STORE MIN. VALUE" */
};

/* REND10A -- p. 582: the update, by INCORP1 with the 6 x 6 W matrix
 * ("CLEAR FOR 6 X 6 W MATRIX"). On the first pass, its sizes for verb 06
 * noun 49: the position's (DELTAX) in N49DISP and the velocity's (DELTAX
 * +6) in N49DISP +2, both shifted by X2 (SHIFTNDX); larger than RMAX or
 * VMAX, the crew is asked (RENDISP). Else on at REND12. */
static const word rend10a_words[] = {
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, DMENFLG), /* "CLEAR FOR 6 X 6 W MATRIX" */
    ADDRESS(INCORP1),       /* "CALCULATE UPDATE" */
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDERS(BOFF, CALL),
    SWITCH(BOF, INCORFLG),
    ADDRESS(REND12),
    ADDRESS(SHIFTNDX), /* "GET EARTH MOON SCALING INDEX" */
    ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE_AT(DELTAX, 6),
    ORDER(SR_STAR),
    SR_X2(0),
    STOVL_AT(N49DISP, 2),
    ADDRESS_PLUS_ONE(DELTAX),
    ORDERS(ABVAL, SR_STAR),
    SR_X2(0),
    STORE(N49DISP),
    ORDER(SLOAD),
    ADDRESS_PLUS_ONE(RMAX),
    ORDERS(SR, DSU),
    SR_COUNT(10),
    ADDRESS_PLUS_ONE(N49DISP),
    ORDERS(BMN, SLOAD),
    ADDRESS(RENDISP), /* "BRANCH IF POS UP. GREATER THAN MAX." */
    ADDRESS_PLUS_ONE(VMAX),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE_AT(N49DISP, 2),
    ADDRESS(RENDISP), /* "BRANCH IF VEL. UPDATE GREATER THAN MAX." */
};

/* ---- The interpretive code ---- */

const struct interpretive_code R22_CODE = {034u, R22 + 6u, COUNT(r22_words), r22_words};
const struct interpretive_code REND2_CODE = {034u, REND2 + 010u, COUNT(rend2_words), rend2_words};
const struct interpretive_code REND3_CODE = {034u, REND3, COUNT(rend3_words), rend3_words};
const struct interpretive_code REND4_CODE = {034u, REND4, COUNT(rend4_words), rend4_words};
const struct interpretive_code REND6_CODE = {034u, REND6, COUNT(rend6_words), rend6_words};
const struct interpretive_code REND9_CODE = {034u, REND9, COUNT(rend9_words), rend9_words};
const struct interpretive_code REND10_CODE = {034u, REND10, COUNT(rend10_words), rend10_words};
const struct interpretive_code REND10A_CODE = {034u, REND10A, COUNT(rend10a_words), rend10a_words};

#endif
