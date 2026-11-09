/* SPDX-License-Identifier: MIT */
/* INTEGRATION_INITIALIZATION.agc, INTWAKEU to GRP2PC (LM pp. 1225-1226,
 * CM pp. 1332-1333), in UPDATE2 (bank 4 in the LM, 27 in the CM).
 * The interpretive words are in the listing's notation
 * (src/interpreter/notation.h); the basic code between them is C.
 *
 * INTWAKEU ends an uplinked state vector update (UPDATE_PROGRAM, P27):
 * the vector, put into RRECT and VRECT with UPSVFLAG saying whose it is,
 * becomes the CSM's or the LM's permanent one (ATOPCSM, ATOPLEM) in the
 * earth's sphere or the moon's (UPMNSVCD, "UPDATE MOON STATE VECTOR
 * CODE"); the W-matrix is no longer valid (ORBWFLAG or RENDWFLG cleared).
 * Then UPSVFLAG is cleared, the integration given back (INTWAKE0), and
 * the return taken from INTWAKUQ under restart group 2's phase. GRP2PC
 * is a phase change of group 2 made from interpretive code. */
#include "integration_init.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

/* TC at "at" to a routine outside the file (PHASCHNG), which reads the
 * word after the TC. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* INTWAKEU -- LM p. 1225 (CM p. 1332): the return kept ("SAVE Q FOR
 * RETURN"), and into the interpreter. */
void intwakeu(void)
{
    relint();
    qxch(ADDRESS_OF(INTWAKUQ));   /* EXTEND; QXCH INTWAKUQ */
    tc_at((word)(INTWAKEU + 3u)); /* TC INTPRET */
    intpret();
}

static const word intwakeu_words[] = {
    /* "IS THIS A CSM/LEM STATE VECTOR UPDATE REQUEST": with UPSVFLAG
     * zero, INTWAKUP. Otherwise RRECT and VRECT into RCV and VCV (RECTIFY
     * +13D, which also zeroes TDELTAV, TNUV, TC and XKEP). */
    ORDERS(SLOAD, BZE),
    ADDRESS_PLUS_ONE(UPSVFLAG),
    ADDRESS(INTWAKUP),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(RRECT),
    STOVL(RCV),
    ADDRESS_PLUS_ONE(VRECT),
    ORDER(CALL),
    ADDRESS_AT(RECTIFY, 13),
    /* UPSVFLAG's size equal to UPMNSVCD: the moon's sphere (INTWAKEM);
     * otherwise the earth's. X2 the central body's index, and MOONFLAG. */
    ORDERS(SLOAD, ABS),
    ADDRESS_PLUS_ONE(UPSVFLAG),
    ORDERS(DSU, BZE),
    ADDRESS_PLUS_ONE(UPMNSVCD),
    ADDRESS(INTWAKEM),
    ORDERS(AXT_2, CLRGO), /* "EARTH SPHERE OF INFLUENCE." */
    NUMBER(0),
    SWITCH(CLRGO, MOONFLAG),
    ADDRESS(INTWAKEC),
    /* INTWAKEM */ ORDERS(AXT_2, SET), /* "LUNAR SPHERE OF INFLUENCE." */
    NUMBER(2),
    SWITCH(SET, MOONFLAG),
    /* INTWAKEC -- LM p. 1226 (CM p. 1332): UPSVFLAG negative for the LM
     * (INTWAKLM, ATOPLEM, RENDWFLG cleared), positive for the CSM
     * (ATOPCSM, ORBWFLAG cleared). */
    /* INTWAKEC */ ORDERS(SLOAD, BMN),
    ADDRESS_PLUS_ONE(UPSVFLAG),
    ADDRESS(INTWAKLM),
    ORDER(CALL), /* "UPDATE CSM STATE VECTOR" */
    ADDRESS(ATOPCSM),
    ORDERS(CLEAR, GOTO),
    SWITCH(CLEAR, ORBWFLAG),
    ADDRESS(INTWAKEX),
    /* INTWAKLM */ ORDER(CALL), /* "UPDATE LM STATE VECTOR" */
    ADDRESS(ATOPLEM),
    /* INTWAKEX */ ORDER(CLEAR),
    SWITCH(CLEAR, RENDWFLG),
    /* INTWAKUP: UPSVFLAG zeroed ("REMOVE"), INTWAKE0 ("RELEASE"), and
     * out of the interpreter. */
    /* INTWAKUP */ ORDERS(SSP, CALL),
    ADDRESS_PLUS_ONE(UPSVFLAG),
    NUMBER(0),
    ADDRESS(INTWAKE0),
    ORDER(EXIT),
};

/* The words after INTWAKEU's TC INTPRET. */
const struct interpretive_code INTWAKEU_CODE = {
    CODE_BANK(I_INTWAKEU), (word)(CODE_Z(I_INTWAKEU) + 4u), COUNT(intwakeu_words), intwakeu_words};

/* After INTWAKUP's EXIT: group 2's phase (OCT 04026), and back by the
 * word in INTWAKUQ, the caller's return, which runs as an instruction (a
 * TC; anything else, which no program leaves, is not modelled): Q gets
 * the word after INTWAKUQ. */
void intwakup_after_exit(void)
{
    tc_to((word)(INTWAKUP + 5u), PHASCHNG); /* OCT 04026 */
}

void intwakup_after_phaschng(void)
{
    word instruction = read_word(ADDRESS_OF(INTWAKUQ));

    tc_at((word)(INTWAKUP + 7u)); /* TC INTWAKUQ */
    if ((instruction & 070000u) || instruction < 010u) not_modelled();
    tc_at(ADDRESS_OF(INTWAKUQ));
    continue_at(instruction);
}

static const word grp2pc_words[] = {
    /* UPMNSVCD: the code of a moon state vector. */
    /* UPMNSVCD */ NUMBER(2),
    NUMBER(0),
    /* GRP2PC -- LM p. 1226 (CM p. 1333): the return kept in GRP2SVQ, and
     * out of the interpreter. */
    /* GRP2PC */ ORDERS(STQ, EXIT),
    ADDRESS(GRP2SVQ),
};

const struct interpretive_code GRP2PC_CODE = {CODE_BANK(I_UPMNSVCD), CODE_Z(I_UPMNSVCD),
                                              COUNT(grp2pc_words), grp2pc_words};

/* After GRP2PC's EXIT: group 2's phase (OCT 04022), and back into the
 * interpreter, to the return. */
void grp2pc_after_exit(void)
{
    tc_to((word)(GRP2PC + 2u), PHASCHNG); /* OCT 04022 */
}

void grp2pc_after_phaschng(void)
{
    tc_at((word)(GRP2PC + 4u)); /* TC INTPRET */
    intpret();
}

static const word grp2pc_goto_words[] = {
    ORDER(GOTO),
    ADDRESS(GRP2SVQ),
};

/* The words after GRP2PC's TC INTPRET. */
const struct interpretive_code GRP2PC_GOTO_CODE = {CODE_BANK(I_GRP2PC),
                                                   (word)(CODE_Z(I_GRP2PC) + 5u),
                                                   COUNT(grp2pc_goto_words), grp2pc_goto_words};
