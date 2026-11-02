/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 7 (Comanche055 pp. 586-589): R22's subroutines in bank
 * 27, from SETINTG to INITIALW.
 *
 * SETINTG sets up an integration of the state vector to the mark's time.
 * CNTCHK and CONTCHK decide whether R22 may go on: not without a good
 * REFSMMAT or the track flag (ENDPLAC ends the job); while R61's maneuver
 * runs (R61CNTR negative) or updating is not allowed, it waits four
 * seconds at a time (WAITONE, and REDOR22, where a restart comes back to).
 * RANGERD reads the VHF range: the read is started in channel 13, R22's
 * job waits in RADSTALL, and the range (RM, in .01 nautical miles) goes to
 * VHFRANGE in meters; LIGHTON lights the tracker lamp for a bad reading.
 * VHFREAD is the interrupt's end of the read: it keeps the count in RM and
 * leaves through GOODEND or BADEND. SHIFTNDX gives the index for the
 * positions' scaling (earth or moon), and INITIALW the W matrix's starting
 * values.
 *
 * Each call out of the file (PHASCHNG, PRIOCHNG, BANKCALL, DOWNFLAG,
 * TRFAILOF, TRFAILON, POSTJUMP) leaves Z at the routine and Q at the word
 * after the TC, where the routine comes back to; RADSTALL comes back to
 * the word after its CADR when the read failed, the next when it was
 * good. TASKOVER and POSTJUMP's routines don't come back. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void lighton(void);
static void rangerd3(void);
static void rangerd1_positive(void);
static void bypass(void);
static void bypass1(void);

/* SETINTG -- p. 586: the return in EGRESS; INTSTALL to "RESERVE
 * INTEGRATION"; TDEC1, the time to integrate to, from MARKTIME; the state
 * vector to be updated (STATEFLG), a "PRECISION INTEGRATION" (INTYPFLG
 * clear, and DIM0FLAG) of the "VEHICLE EQ. CSM" (VINTFLAG), with the "W
 * MATRIX DIM. EQ 6" (D6OR9FLG clear), and back through EGRESS. */
static const word setintg_words[] = {
    ORDERS(STQ, CALL),
    ADDRESS(EGRESS),
    /* "RESERVE INTEGRATION" */
    ADDRESS(INTSTALL),
    ORDERS(DLOAD, SET),
    ADDRESS_PLUS_ONE(MARKTIME),
    SWITCH(SET, STATEFLG),
    /* "MARKTIME" */
    STORE(TDEC1),
    ORDERS(CLEAR, CLEAR),
    /* "PRECISION INTEGRATION" */
    SWITCH(CLEAR, INTYPFLG),
    SWITCH(CLEAR, DIM0FLAG),
    ORDERS(SET, CLRGO),
    /* "SET VEHICLE EQ. CSM" */
    SWITCH(SET, VINTFLAG),
    /* "SET W MATRIX DIM. EQ 6" */
    SWITCH(CLRGO, D6OR9FLG),
    /* "EXIT" */
    ADDRESS(EGRESS),
};

/* CNTCHK: the return in POINTEX. CONTCHK: "BRANCH TO END OF JOB IF
 * REFSMMAT NO GOOD"; R61CNTR negative, a maneuver under way: wait
 * (WAITONE1). Updating allowed (UPDATFLG): back to the caller. Without the
 * track flag ("IS TRACK FLAG SET"), the end of the job; with it, out of
 * the interpreter to REDOR22, to wait. */
static const word cntchk_words[] = {
    ORDER(STQ),
    ADDRESS(POINTEX),
    /* CONTCHK */
    ORDER(BOFF),
    SWITCH(BOF, REFSMFLG), /* "BRANCH TO END OF JOB IF REFSMMAT NO GOOD" */
    ADDRESS(ENDPLAC),
    ORDERS(SLOAD, BMN),
    ADDRESS_PLUS_ONE(R61CNTR),
    ADDRESS(WAITONE1),
    ORDERS(BON, BOFF), /* "IS TRACK FLAG SET" */
    SWITCH(BON, UPDATFLG),
    ADDRESS(POINTEX),
    SWITCH(BOF, TRACKFLG),
    ADDRESS(ENDPLAC),
    ORDER(EXIT),
};

/* REDOR22 -- after CONTCHK's EXIT, and where a restart comes back to: a
 * restart phase change (PHASCHNG, OCT 00132), */
void redor22(void)
{
    tc_to(REDOR22, PHASCHNG);
}

/* REDOR22 +2: the job's priority to 26 (PRIOCHNG), */
void redor22_after_phaschng(void)
{
    set_A(PRIO26);
    tc_to(REDOR22 + 3u, PRIOCHNG);
}

/* REDOR22 +4: and the wait, at WAITONE +3. */
void redor22_after_priochng(void)
{
    tc_at(REDOR22 + 4u);
    waitone_after_exit();
}

/* WAITONE: the return in POINTEX. WAITONE1: out of the interpreter, */
static const word waitone_words[] = {
    ORDER(STQ),
    ADDRESS(POINTEX),
    /* WAITONE1 */
    ORDER(EXIT),
};

/* WAITONE +3 -- after the EXIT: "WAIT 4 SECS." (DELAYJOB), */
void waitone_after_exit(void)
{
    set_A(K4SECS);
    tc_to(WAITONE + 4u, BANKCALL); /* CADR DELAYJOB */
}

/* WAITONE +6: and in interpretive code again, */
void waitone_after_delayjob(void)
{
    tc_at(WAITONE + 6u);
    intpret();
}

/* WAITONE +7: CONTCHK, "CHECK AGAIN NOW". */
static const word waitone_again_words[] = {
    ORDER(GOTO),
    ADDRESS(CONTCHK), /* "CHECK AGAIN NOW" */
};

/* RANGERD -- p. 586: out of the interpreter, */
static const word rangerd_words[] = {
    ORDER(EXIT),
};

/* RANGERD +1 -- after the EXIT: with interrupts held, "ZERO OUT BITS 1-4
 * OF CHANNEL 13" and set bits 1 and 4 (OCT11): "GENERAGE SHIFT PULSES TO
 * RADR, SET R. BIT". Then "READ PRESENT TIME" into MARKTIME, and SKIPVHF
 * cleared, so that VHFREAD takes the reading. */
void rangerd_after_exit(void)
{
    inhint();
    set_A(negate(OCT17)); /* CS OCT17 */
    wand_channel(013);
    set_A(OCT11);
    wor_channel(013);
    relint();
    dca(ADDRESS_OF(TIME2));
    dxch(ADDRESS_OF(MARKTIME));
    tc_to(RANGERD + 014u, DOWNFLAG); /* ADRES SKIPVHF */
}

/* RANGERD +016: "WAIT FOR RANGE COMPLETE" (RADSTALL). */
void rangerd_after_skipvhf(void)
{
    tc_to(RANGERD + 016u, BANKCALL); /* CADR RADSTALL */
}

/* RANGERD +020: the read failed, "BAD DATA GOOD BIT" (LIGHTON). */
void rangerd_bad(void)
{
    tc_at(RANGERD + 020u);
    lighton();
}

/* RANGERD +021: the read was good: "TURN TRACKER LIGHT OFF". */
void rangerd_good(void)
{
    tc_to(RANGERD + 021u, TRFAILOF);
}

/* RANGERD1 -- after TRFAILOF: the "15 BIT UNSIGNED RANGE" in RM, by its
 * sign: positive ("GR + 0"), RANGERD4; +0, LIGHTON +4, no range;
 * negative or -0, RANGERD3 (the top bit set). */
void rangerd1(void)
{
    switch (ccs_at(ADDRESS_OF(RM))) {
    case POSITIVE: rangerd1_positive(); break;
    case IS_PLUS_ZERO: rangerd1_plus_zero(); break;
    case NEGATIVE: rangerd1_negative(); break;
    case IS_MINUS_ZERO: rangerd1_minus_zero(); break;
    }
}

static void rangerd1_positive(void)
{
    tc_at(RANGERD1 + 1u);
    rangerd4();
}

void rangerd1_plus_zero(void)
{
    tc_at(RANGERD1 + 2u);
    lighton_after_trfailon();
}

void rangerd1_negative(void)
{
    tc_at(RANGERD1 + 3u);
    rangerd3();
}

void rangerd1_minus_zero(void)
{
    tc_at(RANGERD1 + 4u);
    rangerd3();
}

/* RANGERD4: in interpretive code, */
void rangerd4(void)
{
    tc_at(RANGERD4);
    intpret();
}

/* RANGERD4 +1: "CONVERT RANGE TO METERS B-27" (CONVRNGE, 18.52 meters
 * per .01 nautical mile). RANGERD2: to VHFRANGE, the "SOURCE OF DATA TO
 * VHF RADAR" (SOURCFLG), and back. */
static const word rangerd4_words[] = {
    ORDERS(SLOAD, DMP),
    ADDRESS_PLUS_ONE(RM),
    ADDRESS_PLUS_ONE(CONVRNGE), /* "CONVERT RANGE TO METERS B-27" */
    /* RANGERD2 */
    STORE(VHFRANGE),
    ORDERS(SET, RVQ),
    SWITCH(SET, SOURCFLG), /* "SOURCE OF DATA TO VHF RADAR" */
};

/* RANGERD3: a range with the top bit set: the bit masked off ("MASK OUT
 * NEG. SIGN BIT") into MPAC, and in interpretive code. (A word masked by
 * POSMAX leaves no overflow for the TS to skip on.) */
static void rangerd3(void)
{
    ca(ADDRESS_OF(RM));
    mask(POSMAX);
    if (ts(ADDRESS_OF(MPAC))) not_modelled();
    tc_at(RANGERD3 + 3u);
    intpret();
}

/* RANGERD3 +4: "CONVERT FROM NM TO METERS AND SCALE B-27", and the top
 * bit back, as meters: RANGEB14, the "VALUE IN METERS OF SIGN BIT SCALED
 * B-27"; to RANGERD2. */
static const word rangerd3_words[] = {
    ORDER(DMP),
    /* "CONVERT FROM NM TO METERS AND SCALE B-27" */
    ADDRESS_PLUS_ONE(CONVRNGE),
    ORDERS(DAD, GOTO),
    /* "VALUE IN METERS OF SIGN BIT SCALED B-27" */
    ADDRESS_PLUS_ONE(RANGEB14),
    ADDRESS(RANGERD2),
};

/* LIGHTON: the tracker lamp on (TRFAILON) unless VHFREAD left VHFRANGE
 * +0 (the update flag or the priority displays flag stopped the read). */
static void lighton(void)
{
    ca(ADDRESS_OF(VHFRANGE));
    if (A_is_zero()) { /* EXTEND; BZF +2 */
        lighton_after_trfailon();
        return;
    }
    tc_to(LIGHTON + 3u, TRFAILON);
}

/* LIGHTON +4: in interpretive code, */
void lighton_after_trfailon(void)
{
    tc_at(LIGHTON + 4u);
    intpret();
}

/* LIGHTON +5: no range this time: the time of the try to VHFTIME (the
 * next is a minute on), and on at REND1. */
static const word lighton_words[] = {
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(MARKTIME),
    /* the time of the try */
    STORE(VHFTIME),
    ORDER(GOTO),
    ADDRESS(REND1),
};

/* VHFREAD -- pp. 587-588, from the interrupt at the read's end. A, with
 * the superbank bits ("MUST SAVE SBANK BECAUSE OF RUPT", it "EXITS VIA
 * TASKOVER BADEND OR GOODEND"), goes to BANKRUPT, -0 to RUPTAGN and Q to
 * QRUPT. With the SKIPVHF flag set ("BRANCH IF VHF RESTART BIT SET"), out
 * through TASKOVER. Without UPDATFLG ("UPDATEFLG"), or with PDSPFLAG set,
 * BYPASS1. Else "SAVE RANGE" in RM, and "READ DATA GOOD BIT" (channel 33
 * bit 2): 0, "BRANCH IF DATA GOOD BIT EQUALS GOOD", VHFGOOD; else BYPASS
 * with the range. (A bank or channel word leaves no overflow for a TS to
 * skip on, nor do -0 and RNRAD.) */
void vhfread(void)
{
    ror_superbnk();
    if (ts(ADDRESS_OF(BANKRUPT))) not_modelled();
    set_A(negate(ZERO)); /* CS ZERO */
    if (ts(ADDRESS_OF(RUPTAGN))) not_modelled();
    qxch(ADDRESS_OF(QRUPT));
    cs(ADDRESS_OF(STATE) + 2u);
    mask(SKIPVBIT); /* "SKIPVHF FLAG" */
    if (A_is_zero()) {
        continue_at(TASKOVER); /* "BRANCH IF VHF RESTART BIT SET" */
        return;
    }
    set_A(UPDATBIT);
    mask_at(ADDRESS_OF(STATE) + 1u); /* "UPDATEFLG" */
    if (A_is_zero()) {
        bypass1();
        return;
    }
    cs(ADDRESS_OF(STATE) + 4u);
    mask(PDSPFBIT); /* "PDSPFLAG" */
    if (A_is_zero()) {
        bypass1();
        return;
    }
    ca(ADDRESS_OF(RNRAD));
    if (ts(ADDRESS_OF(RM))) not_modelled(); /* "SAVE RANGE" */
    set_A(BIT2);
    rand_channel(033); /* "READ DATA GOOD BIT" */
    if (A_is_zero()) {
        vhfgood();
        return;
    }
    bypass();
}

/* BYPASS: VHFRANGE gets A, "STORE NON ZERO VALUE" (BIT2, the data good
 * bit), and out through BADEND. (A constant leaves no overflow for the TS
 * to skip on.) */
static void bypass(void)
{
    if (ts(ADDRESS_OF(VHFRANGE))) not_modelled(); /* "STORE NON ZERO VALUE" */
    set_A(TWO);
    tc_to(BYPASS + 2u, POSTJUMP); /* CADR BADEND */
}

/* BYPASS1: BYPASS with +0, which LIGHTON looks for. */
static void bypass1(void)
{
    set_A(ZERO);
    tc_at(BYPASS1 + 1u);
    bypass();
}

/* VHFGOOD: out through GOODEND. */
void vhfgood(void)
{
    set_A(TWO);
    tc_to(VHFGOOD + 1u, POSTJUMP); /* CADR GOODEND */
}

/* SHIFTNDX -- p. 588: X2, the shift for the positions' scaling ("SCALED
 * B-27 OR B-29"): 0, or -2 when the vehicle's sphere is the moon's: the
 * CSM's (CMOONFLG) for a CSM update ("VEHICLE IS CSM", SHIFTA), else the
 * LM's (LMOONFLG). */
static const word shiftndx_words[] = {
    ORDERS(AXT_2, BON),
    NUMBER(0),
    SWITCH(BON, VEHUPFLG),
    ADDRESS(SHIFTA), /* "VEHICLE IS CSM" */
    ORDERS(BON, RVQ),
    SWITCH(BON, LMOONFLG),
    ADDRESS_AT(SHIFTNDX, 7), /* +1 */
    ORDERS(INCR_2, RVQ),
    MINUS_NUMBER(2),
    /* SHIFTA */
    ORDERS(BON, RVQ),
    SWITCH(BON, CMOONFLG),
    ADDRESS_AT(SHIFTA, 3), /* +1: "MOON ORB." */
    ORDERS(INCR_2, RVQ),
    MINUS_NUMBER(2),
};

/* INITIALW -- pp. 588-589: the W matrix's words "0 - 35" and "54 - 89"
 * cleared, a vector (six words, S1) at a time, and its diagonal set:
 * "POSITION VALUE", the push-down list's word 0, in W, W +8D and W +16D,
 * and "VELOCITY VALUE", word 1, in W +72D, W +80D and W +88D. */
static const word initialw_words[] = {
    ORDERS(AXT_1, SSP),
    NUMBER(36),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(ZEROVECS),
    /* INITA */
    STORE_AT_X1(W, 36), /* "CLEAR 0 - 35" */
    ORDERS(TIX_1, AXT_1),
    ADDRESS(INITA),
    NUMBER(36),
    /* INITB */
    STORE_AT_X1(W, 90), /* "CLEAR 54 - 89" */
    ORDERS(TIX_1, SLOAD),
    ADDRESS(INITB),
    ADDRESS_PLUS_ONE_D(0), /* "POSITION VALUE" */
    STORE(W),              /* "INITIALIZE DIAGONAL W POSITION" */
    STORE_AT(W, 8),
    STORE_AT(W, 16),
    ORDER(SLOAD),
    ADDRESS_PLUS_ONE_D(1), /* "VELOCITY VALUE" */
    STORE_AT(W, 72),       /* "INITIALIZE DIAGONAL W VELOCITY" */
    STORE_AT(W, 80),
    STORE_AT(W, 88),
    ORDER(RVQ),
};

/* ---- The interpretive code ---- */

const struct interpretive_code SETINTG_CODE = {027u, SETINTG, COUNT(setintg_words), setintg_words};
const struct interpretive_code CNTCHK_CODE = {027u, CNTCHK, COUNT(cntchk_words), cntchk_words};
const struct interpretive_code WAITONE_CODE = {027u, WAITONE, COUNT(waitone_words), waitone_words};
const struct interpretive_code WAITONE_AGAIN_CODE = {027u, WAITONE + 7u, COUNT(waitone_again_words),
                                                     waitone_again_words};
const struct interpretive_code RANGERD_CODE = {027u, RANGERD, COUNT(rangerd_words), rangerd_words};
const struct interpretive_code RANGERD4_CODE = {027u, RANGERD4 + 1u, COUNT(rangerd4_words),
                                                rangerd4_words};
const struct interpretive_code RANGERD3_CODE = {027u, RANGERD3 + 4u, COUNT(rangerd3_words),
                                                rangerd3_words};
const struct interpretive_code LIGHTON_CODE = {027u, LIGHTON + 5u, COUNT(lighton_words),
                                               lighton_words};
const struct interpretive_code SHIFTNDX_CODE = {027u, SHIFTNDX, COUNT(shiftndx_words),
                                                shiftndx_words};
const struct interpretive_code INITIALW_CODE = {027u, INITIALW, COUNT(initialw_words),
                                                initialw_words};

#endif
