/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 6 (Comanche055 pp. 582-585): R22 from REND12, the
 * update into the state vector, a range's B vector, and the crew's check
 * of a large update (RENDISP) (bank 34).
 *
 * REND12 puts the update INCORP1 worked out into the state vector
 * (INCORP2). An optics mark then takes its second pass (REND13, with the
 * new RCLP) and is counted (REND17, TRKMKCNT); a range is counted at once
 * (REND16, VHFCNT). REND14 is a range's measurement: its B vector is the
 * line of sight itself, and DELTAQ the range less RCLP's length. CMPOS and
 * LMPOS give each vehicle's position from its state vector.
 *
 * When the first pass's update is larger than RMAX or VMAX, RENDISP shows
 * it with verb 06 noun 49 in a job of its own (RENDISP2, at a higher
 * priority), while R22's job waits in RENDISP7 for the answer in TEMPOR1:
 * proceed (negative) puts the update in all the same, at REND12; recycle
 * (positive) throws the measurement away, at RENDISP3; terminate ends P20
 * by verb 56 (GOTOV56).
 *
 * Each call out of the file (PHASCHNG, NOVAC, BANKCALL) leaves Z at the
 * routine and Q at the word after the TC, where the routine comes back to
 * (NOVAC past its 2CADR); ENDOFJOB doesn't come back. A display through
 * PRIODSP comes back to the word after its CADR on terminate and the next
 * on proceed. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void rendisp_source(void);

/* REND12 -- pp. 582-583: "INCORPORATE UPDATE VALUES INTO STATE VEC". A
 * range is done ("BRANCH IF DATA IS RADAR", REND16); so is an optics
 * mark's second pass (INCORFLG clear, REND17). After the first pass, the
 * updated vehicle's position again (LMPOS, or REND18's CMPOS "IF CSM
 * UPDATE"); REND13: RCLP, "LM - CSM", from it, and "BRANCH FOR 2ND PASS
 * THIS OPTICS MARK" (REND7), INCORFLG clear. CSMUPP (the end of p. 580's
 * REND4): for a CSM update, the LM integrated first ("SET INTEGRATION
 * VEHICLE EQ LM"), then the CSM ("SET FLAGS FOR INTEGRATION"), at REND5. */
static const word rend12_words[] = {
    ORDER(CALL),
    ADDRESS(INCORP2), /* "INCORPORATE UPDATE VALUES INTO STATE VEC" */
    ORDERS(BON, BOFF),
    SWITCH(BON, SOURCFLG),
    ADDRESS(REND16), /* "BRANCH IF DATA IS RADAR" */
    SWITCH(BOF, INCORFLG),
    ADDRESS(REND17),
    ORDER(CALL),
    ADDRESS(SHIFTNDX), /* "GET EARTH MOON SCALING INDEX" */
    ORDERS(BON, CALL),
    SWITCH(BON, VEHUPFLG),
    ADDRESS(REND18), /* "BRANCH IF CSM UPDATE" */
    ADDRESS(LMPOS),  /* "GET LM POSITION" */
    /* REND13 */
    ORDER(CALL),
    ADDRESS(GRP2PC), /* "PHASE CHANGE" */
    ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(LEMPOS),
    ADDRESS_PLUS_ONE(CSMPOS),
    STORE(RCLP), /* "LM - CSM" */
    ORDER(CLRGO),
    SWITCH(CLRGO, INCORFLG),
    ADDRESS(REND7), /* "BRANCH FOR 2ND PASS THIS OPTICS MARK" */
    /* CSMUPP */
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, VINTFLAG), /* "SET INTEGRATION VEHICLE EQ LM" */
    ADDRESS(INTEGRV),
    ORDER(CALL),
    ADDRESS(GRP2PC), /* "PHASE CHANGE" */
    ORDER(CALL),
    ADDRESS(SETINTG), /* "SET FLAGS FOR INTEGRATION" */
    ORDER(GOTO),
    ADDRESS(REND5),
};

/* REND14 -- p. 583: the "VHF RADAR BVECTOR", RCLP's unit vector (left in
 * MPAC by REND7's UNIT) with two zero vectors. DELTAQ is the range,
 * "VHFRANGE SCALED B-27" (shifted two more, B-29, unless MOONTHIS: the
 * positions' scaling), less RCLP's length ("ABVAL (RCLP)", which UNIT
 * leaves in 36D); INCORFLG set, so that a large update goes to the crew
 * too. For a CSM update the B vector turns round, as at REND9. REND14A:
 * the range's variance, RVAR, to REND10. REND15: for R23's marks, "GET ALT
 * LOS VARIANCE" (ALTVAR, the "BACKUP OPTICS") plus the "IMU VARIANCE". */
static const word rend14_words[] = {
    STOVL(BVECTOR), /* "VHF RADAR BVECTOR" */
    ADDRESS_PLUS_ONE(ZEROVECS),
    STORE_AT(BVECTOR, 6),
    STOVL_AT(BVECTOR, 12),
    ADDRESS_PLUS_ONE(RCLP),
    ORDERS(UNIT, DLOAD),
    ADDRESS_PLUS_ONE(VHFRANGE), /* "VHFRANGE SCALED B-27" */
    ORDERS(BON, SR2),
    SWITCH(BON, MOONTHIS),
    ADDRESS_AT(REND14, 012), /* +1 */
    ORDERS(DSU, SET),
    ADDRESS_PLUS_ONE_D(36), /* "ABVAL (RCLP)" */
    SWITCH(SET, INCORFLG),
    STORE(DELTAQ),
    ORDERS(BOFF, VLOAD),
    SWITCH(BOF, VEHUPFLG),
    ADDRESS(REND14A),
    ADDRESS_PLUS_ONE(BVECTOR),
    ORDER(VCOMP),
    STORE(BVECTOR),
    /* REND14A */
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDERS(DLOAD, GOTO),
    ADDRESS_PLUS_ONE(RVAR),
    ADDRESS(REND10),
    /* REND15 */
    ORDERS(SLOAD, DAD),       /* "GET ALT LOS VARIANCE" */
    ADDRESS_PLUS_ONE(ALTVAR), /* "BACKUP OPTICS" */
    ADDRESS_PLUS_ONE(IMUVAR), /* "IMU VARIANCE" */
    ORDER(GOTO),
    ADDRESS(REND10),
    /* REND16: the "VHF RADAR UPDATE COUNT" one more, and on at REND1. */
    ORDERS(LXA_1, INCR_1),
    ADDRESS(VHFCNT), /* "VHF RADAR UPDATE COUNT" */
    NUMBER(1),
    ORDERS(SXA_1, GOTO),
    ADDRESS(VHFCNT), /* "UPDATE COUNT" */
    ADDRESS(REND1),
    /* REND17: the "OPTICS MARK COUNT" one more, and on at REND3. */
    ORDERS(LXA_1, INCR_1),
    ADDRESS(TRKMKCNT), /* "OPTICS MARK COUNT" */
    NUMBER(1),
    ORDERS(SXA_1, GOTO),
    ADDRESS(TRKMKCNT), /* "UPDATE COUNT" */
    ADDRESS(REND3),
    /* REND18 */
    ORDER(CALL),
    ADDRESS(CMPOS), /* "GET CSM POSITION" */
    ORDER(GOTO),
    ADDRESS(REND13),
};

/* CMPOS and LMPOS -- p. 584: each vehicle's position, its conic position
 * (RCVCSM, RCVLEM) plus its deviation from it (DELTACSM, DELTALEM, shifted
 * by X2, SHIFTNDX's index), "SCALED B-27 OR B-29". */
static const word cmpos_words[] = {
    ORDERS(VLOAD, VSR_STAR),
    ADDRESS_PLUS_ONE(DELTACSM),
    SR_X2(7),
    ORDER(VAD),
    ADDRESS_PLUS_ONE(RCVCSM),
    STORE(CSMPOS), /* "CSM POSITION SCALED B-27 OR B-29" */
    ORDER(RVQ),
    /* LMPOS */
    ORDERS(VLOAD, VSR_STAR),
    ADDRESS_PLUS_ONE(DELTALEM),
    SR_X2(7),
    ORDER(VAD),
    ADDRESS_PLUS_ONE(RCVLEM),
    STORE(LEMPOS), /* "LM POSITION SCALED B-27 OR B-29" */
    ORDER(RVQ),
};

/* RENDISP: out of the interpreter. */
static const word rendisp_words[] = {
    ORDER(EXIT),
};

/* RENDISP +1 -- after the EXIT: noun 49's third word, the data's source:
 * 2 for the VHF radar (SOURCBIT, FLAGWRD9), */
void rendisp_after_exit(void)
{
    ca(ADDRESS_OF(FLAGWRD9));
    mask(SOURCBIT);
    if (A_is_zero()) { /* EXTEND; BZF +3 */
        rendisp_optics();
        return;
    }
    set_A(BIT2);
    tc_at(RENDISP + 6u); /* TC +2 */
    rendisp_source();
}

/* RENDISP +7: 1 for the optics. */
void rendisp_optics(void)
{
    set_A(BIT1);
    rendisp_source();
}

/* RENDISP +010: to N49DISP +4; TEMPOR1 +0, "SET TEMPOR1 > ZERO TO
 * INDICATE", "V06 N49 DISPLAY HASN'T BEEN ANSWERED"; and a restart phase
 * change (PHASCHNG, OCT 04022). (No A here overflows, for a TS to skip
 * on.) */
static void rendisp_source(void)
{
    if (ts(ADDRESS_OF(N49DISP) + 4u)) not_modelled();
    set_A(ZERO);
    if (ts(ADDRESS_OF(TEMPOR1))) not_modelled();
    tc_to(RENDISP + 013u, PHASCHNG);
}

/* RENDISP +015: "SET UP DISPLAY JOB WITH HIGHER PRIORITY", "THAN PRESENT
 * JOB": RENDISP2, priority 27 (NOVAC). */
void rendisp_after_phaschng(void)
{
    set_A(PRIO27);
    tc_to(RENDISP + 016u, NOVAC); /* 2CADR RENDISP2 */
}

/* RENDISP7 -- p. 585: R22's job waits for the answer, in interpretive
 * code. */
void rendisp7(void)
{
    tc_at(RENDISP7);
    intpret();
}

/* RENDISP7 +1: TEMPOR1 +0, "DISPLAY HAS NOT BEEN ANSWERED YET": round
 * again. (Between orders the interpreter lets a job of higher priority,
 * the display's, run.) Negative, "NEG INDICATES PROCEED", REND12;
 * positive, "POS INDICATES RECYCLE", RENDISP3. */
static const word rendisp7_words[] = {
    STORE(MPAC),
    ORDERS(SLOAD, BZE),
    ADDRESS_PLUS_ONE(TEMPOR1),
    ADDRESS_AT(RENDISP7, 1), /* "DISPLAY HAS NOT BEEN ANSWERED YET" */
    ORDERS(BMN, GOTO),
    ADDRESS(REND12),   /* "NEG INDICATES PROCEED" */
    ADDRESS(RENDISP3), /* "POS INDICATES RECYCLE" */
};

/* RENDISP2: the display job, verb 06 noun 49, by PRIODSP. */
void rendisp2(void)
{
    set_A(V06N49);
    tc_to(RENDISP2 + 1u, BANKCALL); /* CADR PRIODSP */
}

/* RENDISP2 +3: "TERM EXIT P20 VIA V56". */
void rendisp2_terminate(void)
{
    tc_at(RENDISP2 + 3u);
    gotov56();
}

/* RENDISP2 +4: proceed: TEMPOR1 -1, "NEG INDICATES PROCEED RENDISP7 JOB",
 * and "GO COMPLETE ABOVE JOB" (ENDOFJOB). (On recycle PRIODSP comes back
 * to the TS, "POS INDICATES RECYCLE RENDISP7 JOB".) (CS ONE leaves no
 * overflow for the TS to skip on.) */
void rendisp2_proceed(void)
{
    set_A(negate(ONE)); /* CS ONE */
    if (ts(ADDRESS_OF(TEMPOR1))) not_modelled();
    tc_to(RENDISP2 + 6u, ENDOFJOB);
}

/* RENDISP3: recycle, the measurement thrown away. "DATA WAS RADAR GO LOOK
 * FOR OPTICS NEXT" (REND1); else out of the interpreter. */
static const word rendisp3_words[] = {
    ORDER(BON),
    SWITCH(BON, SOURCFLG),
    ADDRESS(REND1), /* "DATA WAS RADAR GO LOOK FOR OPTICS NEXT" */
    ORDER(EXIT),
};

/* RENDISP3 +4 -- after the EXIT: the mark buffers' E bank (BUFBANK, E7),
 * and both buffers -1, "ERASE MARK ONE BUFFER", "ERASE MARK TWO BUFFER",
 * with interrupts held. (A change of F bank, which BUFBANK's BBCON could
 * make, the faithful port refuses: the readable code does too. A CA of a
 * constant leaves no overflow for a TS to skip on.) */
void rendisp3_after_exit(void)
{
    inhint();
    set_A(BUFBANK);
    if ((A & 076000u) != (FBANK & 076000u)) not_modelled();
    if (ts(BBANK_ADDRESS)) not_modelled();
    set_A(NEGONE);
    if (ts(ADDRESS_OF(MRKBUF1))) not_modelled();
    if (ts(ADDRESS_OF(MRKBUF2))) not_modelled();
    relint();
    tc_at(RENDISP4);
    intpret();
}

/* RENDISP4 +1: and on at REND3, to wait for the next mark. */
static const word rendisp4_words[] = {
    ORDER(GOTO),
    ADDRESS(REND3),
};

/* ---- The interpretive code ---- */

const struct interpretive_code REND12_CODE = {034u, REND12, COUNT(rend12_words), rend12_words};
const struct interpretive_code REND14_CODE = {034u, REND14, COUNT(rend14_words), rend14_words};
const struct interpretive_code CMPOS_CODE = {034u, CMPOS, COUNT(cmpos_words), cmpos_words};
const struct interpretive_code RENDISP_CODE = {034u, RENDISP, COUNT(rendisp_words), rendisp_words};
const struct interpretive_code RENDISP7_CODE = {034u, RENDISP7 + 1u, COUNT(rendisp7_words),
                                                rendisp7_words};
const struct interpretive_code RENDISP3_CODE = {034u, RENDISP3, COUNT(rendisp3_words),
                                                rendisp3_words};
const struct interpretive_code RENDISP4_CODE = {034u, RENDISP4 + 1u, COUNT(rendisp4_words),
                                                rendisp4_words};

#endif
