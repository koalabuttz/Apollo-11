/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, part 6 (Comanche055 pp. 723-728, bank 16): S40.9 and
 * S40.13. The faithful C is c64-port/src/cm_p40_p47.c.
 *
 * S40.9, VTOGAIN, a job every so many passes of SERVICER during an
 * aimpoint (Lambert) burn: Lambert's velocity from the state vector at
 * this pass (HAVEGUES), the change of the required velocity since the
 * last pass, scaled to two seconds (BDT, less the gravity GDT/2),
 * and the velocity to be gained (VGTEMP) with the oblateness term
 * (GOBL/2) near the earth. COPY40.9 keeps this pass's values for the
 * next; REDO40.9, its restart, ends it gracefully.
 *
 * S40.13, TIMEBURN, a job at the countdown: whether the burn is long
 * enough to steer, and if not how long it is (TGO), from the velocity to
 * be gained, the mass, the thrust and the ullage (two or four jets, with
 * +X translation or not).
 *
 * Each call out of the file leaves Z at the routine and Q at the word
 * after the TC; a TCF out of it leaves Z there. */
#include "p40_p47_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- S40.9 -- pp. 723-725 ---- */

/* S40.9: to the interpreter, for S40_9_CODE. */
void s40_9(void)
{
    tc_at(S40_9);
    intpret();
}

/* S40_9_CODE: Lambert's tolerance, EP4(45)L, or EP4(10)L with NORMSW
 * clear; GUESSW clear; HAVEGUES; to basic code. */
static const word s40_9_words[] = {
    ORDERS(SETPD, DLOAD),       ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(LO6ZEROS), ORDER(PDDL),
    ADDRESS_PLUS_ONE(EP4_45L),  ORDERS(BON, DLOAD),
    SWITCH(BON, NORMSW),        ADDRESS_AT(S40_9, 012u), /* +2 */
    ADDRESS_PLUS_ONE(EP4_10L),  ORDER(PUSH),
    ORDERS(CLEAR, CALL),        SWITCH(CLEAR, GUESSW),
    ADDRESS(HAVEGUES),          ORDER(EXIT),
};

/* After the EXIT: restart group 1 (PHASCHNG, OCT 05021, OCT 10000), "SAVE
 * TIME BY NOT REDOING LAMBERT CALCS", */
void s40_9_after_exit(void)
{
    tc_to(S40_9 + 017u, PHASCHNG); /* "SAVE TIME BY NOT REDOING LAMBERT CALCS" */
}

/* and to the interpreter, for ENDLAMB_CODE. */
void s40_9_after_phaschng(void)
{
    tc_at(S40_9 + 022u);
    intpret();
}

/* ENDLAMB: unless this is the first pass (FIRSTFLG set), BDT, the
 * change of Lambert's velocity (VIPRIME less VRPREV) scaled to two
 * seconds (200CSHI over TNIT less TNITPREV), less GDT/2. FIRSTTME: near
 * the moon (RTX2 positive), MOONCASE; near the earth VGTEMP gains the
 * oblateness term, GOBL/2 times EARTHMU and the time since NOMTIG over
 * RN's length squared (34D, which UNIT leaves) ("NOTE: NO TEST IS MADE
 * TO SUBTRACT GOBL"). MOONCASE: to basic code. */
static const word endlamb_words[] = {
    ORDER(BON),
    SWITCH(BON, FIRSTFLG),
    ADDRESS(FIRSTTME),
    ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(VIPRIME),
    ADDRESS_PLUS_ONE(VRPREV),
    ORDERS(PDDL, DSU),
    ADDRESS_PLUS_ONE(TNIT),
    ADDRESS_PLUS_ONE(TNITPREV),
    ORDERS(SL, BDDV),
    SL_COUNT(17),
    ADDRESS_PLUS_ONE(200CSHI),
    ORDER(VXSC),
    ORDERS(VSU, VSL1),
    ADDRESS_PLUS_ONE(GDT_2),
    STORE(BDT),
    /* FIRSTTME */ ORDERS(SLOAD, DCOMP),
    ADDRESS_PLUS_ONE(RTX2),
    ORDER(BMN),
    ADDRESS(MOONCASE),
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(RN),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(PIPTIME),
    ADDRESS_PLUS_ONE(NOMTIG),
    ORDERS(DMP, DDV),
    ADDRESS_PLUS_ONE(EARTHMU),
    ADDRESS_PLUS_ONE_D(34),
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(GOBL_2),
    ADDRESS_PLUS_ONE(VGTEMP), /* "NOTE: NO TEST IS MADE TO SUBTRACT GOBL" */
    STORE(VGTEMP),            /* "INSIDE 165-195 DEGREE CONE AREA." */
    /* MOONCASE */ ORDER(EXIT),
};

/* After the EXIT: restart group 1 (PHASCHNG, OCT 04021, "C, JOB BELOW"). */
void mooncase_after_exit(void)
{
    tc_to(MOONCASE + 1u, PHASCHNG);
}

/* COPY40.9: to the interpreter, for COPY40_9_CODE. */
void copy40_9(void)
{
    tc_at(COPY40_9);
    intpret();
}

/* COPY40_9_CODE: TNIT to TNITPREV, VIPRIME to VRPREV, FIRSTFLG clear; to
 * basic code. */
static const word copy40_9_words[] = {
    ORDER(DLOAD),  ADDRESS_PLUS_ONE(TNIT), STOVL(TNITPREV),         ADDRESS_PLUS_ONE(VIPRIME),
    STORE(VRPREV), ORDERS(CLEAR, EXIT),    SWITCH(CLEAR, FIRSTFLG),
};

/* ENDS40.9 -2, "REDO40.9 (RESTART) ENTRY TO END S40.9": NBRCYCLS -1;
 * ENDS40.9: restart group 1 ended (PHASCHNG, OCT 00001). */
static void ends40_9_minus_2(void)
{
    set_A(negate(ONE)); /* CS ONE */
    ts(ADDRESS_OF(NBRCYCLS));
    tc_to(ENDS40_9, PHASCHNG);
}

/* After COPY40_9_CODE's EXIT: ENDS40.9 -2. */
void copy40_9_after_exit(void)
{
    ends40_9_minus_2();
}

/* ENDS40.9 +2: the job ends (TCF ENDOFJOB). */
void ends40_9_after_phaschng(void)
{
    continue_at(ENDOFJOB);
}

/* REDO40.9, where "S40.9 RESTARTS COME HERE TO GRACEFULLY": to the
 * interpreter, for REDO40_9_CODE. */
void redo40_9(void)
{
    tc_at(REDO40_9);
    intpret();
}

/* REDO40_9_CODE: DELVSUM and NBRCYCLS zero, VGPREV to VGTEMP; to basic
 * code. */
static const word redo40_9_words[] = {
    ORDER(VLOAD),    ADDRESS_PLUS_ONE(LO6ZEROS), STODL(DELVSUM), ADDRESS_PLUS_ONE(LO6ZEROS),
    STOVL(NBRCYCLS), ADDRESS_PLUS_ONE(VGPREV),   STORE(VGTEMP),  ORDER(EXIT),
};

/* After the EXIT: TCF ENDS40.9 -2, "STORE 0,0 COVERED NBRCYCLS,P -- FIX UP
 * S". */
void redo40_9_after_exit(void)
{
    ends40_9_minus_2();
}

/* ---- S40.13 -- pp. 726-728 ---- */

/* S40.13: to the interpreter, for S40_13_CODE. */
void s40_13(void)
{
    tc_at(S40_13);
    intpret();
}

/* S40_13_CODE: IMPULSW set, "ASSUME NO STEERING UNTIL FOUND OTHERWISE";
 * the length of VGTIG; to basic code. */
static const word s40_13_words[] = {
    ORDERS(SETPD, SET),   ADDRESS_PLUS_ONE_D(0),
    SWITCH(SET, IMPULSW),                          /* "ASSUME NO STEERING UNTIL FOUND OTHERWISE" */
    ORDERS(VLOAD, ABVAL), ADDRESS_PLUS_ONE(VGTIG), /* "VELOCITY TO BE GAINED AT +7" */
    ORDER(EXIT),
};

static void notaddul(void);

/* After the EXIT: "TEST +X TRANSLATION" (bit 7 of channel 31, inverted
 * by RXOR): none, NOTADDUL; else to the interpreter, for
 * ULLAGE_CODE. */
void s40_13_after_exit(void)
{
    set_A(BIT7);       /* "TEST +X TRANSLATION" */
    rxor_channel(031); /* CHAN31 */
    mask(BIT7);
    if (A_is_zero()) { /* EXTEND; BZF NOTADDUL */
        notaddul();
        return;
    }
    tc_at(S40_13 + 015u);
    intpret();
}

/* ULLAGE_CODE: the velocity to be gained less the ullage's, S40.135
 * over WEIGHT/G ("COMPENSATION FOR 2 JET ULLAGE AT +24"), twice with four
 * jets (NJETSFLG clear). S40.130: if that is less than K1VAL over
 * WEIGHT/G, TGO is less than 100 CS, S40.131; if more than 600 CS,
 * S40.133; between, TGO from EMDOT, FANG, 3.5SEC and 5SECOND, and on at
 * S40.132. S40.131: TGO from K2VAL and K3VAL. S40.132: to basic code. */
static const word ullage_words[] = {
    ORDERS(PDDL, DDV),          /* "00D = MAG OF VGTIG AT +7" */
    ADDRESS_PLUS_ONE(S40_135),  /* "COMPENSATION FOR 2 JET ULLAGE AT +24" */
    ADDRESS_PLUS_ONE(WEIGHT_G), /* "MASS IN KGMS AT +16" */
    ORDERS(BON, SL1),           /* "DOUBLE CORRECTION IF FOUR JETS" */
    SWITCH(BON, NJETSFLG),
    ADDRESS(S40_130),
    /* S40.130 */ ORDER(BDSU),
    ORDERS(PDDL, DDV),       /* "00D = MAG OF VGTIG CORRECTED FOR ULLAGE" */
    ADDRESS_PLUS_ONE(K1VAL), /* "M.NEWTON-CS AT +24" */
    ADDRESS_PLUS_ONE(WEIGHT_G),
    ORDERS(BDSU, BMN),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS(S40_131),  /* "TGO LESS THAN 100 CS" */
    ORDERS(PDDL, DMP), /* "02D = TEMP1 AT +7" */
    ADDRESS_PLUS_ONE(EMDOT),
    ADDRESS_PLUS_ONE(3_5SEC), /* "350 CS AT +14" */
    ORDERS(BDSU, PDDL),
    ADDRESS_PLUS_ONE(WEIGHT_G),
    ADDRESS_PLUS_ONE(FANG),
    ORDERS(DMP, SR2),
    ADDRESS_PLUS_ONE(5SECOND), /* "500 CS AT +14" */
    ORDERS(DDV, PUSH),         /* "04D = TEMP2" */
    ORDERS(BDSU, BPL),
    ADDRESS_PLUS_ONE_D(2),
    ADDRESS(S40_133), /* "TGO GREATER THAN 600 CS" */
    ORDERS(DLOAD, BDDV),
    ORDERS(DMP, DAD),
    ADDRESS_PLUS_ONE(5SECOND), /* "500 CS AT +14" */
    ADDRESS_PLUS_ONE(1SEC2D),  /* "100 CS AT +14" */
    ORDER(GOTO),
    ADDRESS(S40_132),
    /* S40.131 */ ORDERS(DLOAD, DMP), /* "TGO LESS THAN 100 CS" */
    ADDRESS_PLUS_ONE(WEIGHT_G),
    ORDERS(DAD, DDV),
    ADDRESS_PLUS_ONE(K2VAL), /* "M.NEWTON CS AT +24" */
    ADDRESS_PLUS_ONE(K3VAL), /* "M.NEWTON AT +10" */
    /* S40.132 */ ORDER(EXIT),
};

static void s40_134(void);

/* After S40.132's EXIT: MPAC's three words made to agree in sign (TC
 * TPAGREE), */
void s40_132_after_exit(void)
{
    tc_to(S40_132 + 1u, TPAGREE);
}

/* and TGO, +0 and MPAC, "TGO IN CS AT +28"; S40.134 (TC). */
void s40_132_after_tpagree(void)
{
    ca(ADDRESS_OF(MPAC));
    xch(L_ADDRESS);
    set_A(ZERO);
    dxch(ADDRESS_OF(TGO)); /* "TGO IN CS AT +28" */
    tc_at(S40_132 + 6u);
    s40_134();
}

/* S40.133: IMPULSW clear, "WILL STEER VEHICLE"; to basic code. */
static const word s40_133_words[] = {
    ORDERS(CLEAR, EXIT), /* "WILL STEER VEHICLE" */
    SWITCH(CLEAR, IMPULSW),
};

/* S40.134: "KILL GROUP 3" (PHASCHNG, OCT 3), */
static void s40_134(void)
{
    tc_to(S40_134, PHASCHNG); /* "KILL GROUP 3" */
}

/* after S40.133's EXIT, */
void s40_133_after_exit(void)
{
    s40_134();
}

/* and the job ends (TCF ENDOFJOB). */
void s40_134_after_phaschng(void)
{
    continue_at(ENDOFJOB);
}

/* NOTADDUL: to the interpreter, for NOTADDUL_CODE. */
static void notaddul(void)
{
    tc_at(NOTADDUL);
    intpret();
}

/* NOTADDUL_CODE: on at S40.130 +1, "DO NOT COMPENSATE FOR 7 SEC OF
 * ULLAGE". */
static const word notaddul_words[] = {
    ORDER(GOTO),
    ADDRESS_AT(S40_130, 1u), /* "DO NOT COMPENSATE FOR 7 SEC OF ULLAGE" */
};

/* ---- The interpretive code ---- */

const struct interpretive_code S40_9_CODE = {016u, S40_9 + 1u, COUNT(s40_9_words), s40_9_words};
const struct interpretive_code ENDLAMB_CODE = {016u, ENDLAMB, COUNT(endlamb_words), endlamb_words};
const struct interpretive_code COPY40_9_CODE = {016u, COPY40_9 + 1u, COUNT(copy40_9_words),
                                                copy40_9_words};
const struct interpretive_code REDO40_9_CODE = {016u, REDO40_9 + 1u, COUNT(redo40_9_words),
                                                redo40_9_words};
const struct interpretive_code S40_13_CODE = {016u, S40_13 + 1u, COUNT(s40_13_words), s40_13_words};
const struct interpretive_code ULLAGE_CODE = {016u, S40_13 + 016u, COUNT(ullage_words),
                                              ullage_words};
const struct interpretive_code S40_133_CODE = {016u, S40_133, COUNT(s40_133_words), s40_133_words};
const struct interpretive_code NOTADDUL_CODE = {016u, NOTADDUL + 1u, COUNT(notaddul_words),
                                                notaddul_words};

#endif
