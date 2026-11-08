/* SPDX-License-Identifier: MIT */
/* JET_SELECTION_LOGIC.agc (Comanche055 pp. 1039-1049, bank 17): phase 3 of
 * the T5 program, the jet selection logic, which T6PROG points T5LOC at
 * (JETSLECT). It resets TIME5 for phase 1 ("= 60 MS"), sets TIME6 to
 * start the jet channels in 14 ms, reads the translation commands of the
 * hand controller (channel 31), and picks the jets for each axis'
 * rotation command (TAU1 pitch, TAU2 yaw, TAU roll) and the translations
 * that can go with it, from the tables:
 *
 *   PYTABLE  "TABLE FOR PITCH(YAW) COMMANDS": a word by rotation (0, +,
 *            -) plus a bias, 3 for +X translation, 6 for -X (XLNNDX), 9
 *            or 12 for an A(B) or C(D) quad failure, which leaves one jet
 *            and no X translation. PJETS and YJETS mask the axis' jets;
 *            "BITS 9, 10 CONTAIN THE NUMBER OF PITCH JETS" (11, 12 yaw).
 *   RTABLE   "TABLE FOR ROLL, Y AND Z-TRANSLATION COMMANDS", the same way
 *            for roll on the AC quads (ACRJETS) or the BD ones (BDRJETS,
 *            ACORBD picks), with the Y (AC) or Z (BD) translation.
 *   YZTABLE  "Y, Z TRANSLATION TABLE": the other quad pair's translation,
 *            with its own failure bias (XLN1NDX: 3 or 6).
 *
 * The roll words carry "THE NET ROLL TORQUE" coded in three bits; a
 * translation on the other pair is taken unless it cancels a roll that
 * was commanded (NRJETS 0 with TAU not zero). Each axis goes on to its
 * jet on-time (ROLLTIME, PITCHTIM, YAWTIME). The faithful C is
 * c64-port/src/cm_jet_select.c, whose runs stop at the keys of its parts:
 * the functions here start at them. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

static void pword(void);
static void taucheck(void);
static void acrbdz(void);

/* ---- The tables -- pp. 1040-1049 ---- */

/* INDEX, then CA (or AD) of a word of the file's tables. */
static unsigned table(unsigned field, word index)
{
    return jet_index_read(field, index);
}

/* "CHECK FOR ... COMMANDS": the rotation's index for the tables, 0 for
 * none, 1 for +, 2 for -, by the sign of the axis' TAU. */
static void rotation_index(unsigned tau, unsigned rotation)
{
    switch (ccs_at(tau)) {
    case POSITIVE: set_A(ONE); break;
    case NEGATIVE: set_A(TWO); break;
    default: break; /* "0 = NO PITCH": A +0 */
    }
    ts(rotation);
}

/* The table's bias, from the quad failure flag: 9 for one failed quad (+),
 * 12 for the other (-), "IF FAILURES ARE PRESENT IGNORE X-TRANSLATIONS ON
 * THIS AXIS"; with none, the translation's (XLNNDX by its index). */
static void table_bias(unsigned failure, unsigned translation)
{
    switch (ccs_at(failure)) {
    case POSITIVE: set_A(NINE); break;
    case NEGATIVE: set_A(TWELVE); break;
    default: ca(table(ADDRESS_OF(XLNNDX), fetch(translation))); break;
    }
}

/* The other quad pair's translation bias: 3 or 6 for a failed quad, 0
 * for none, before XLN1NDX by the translation's index. */
static void translation_bias(unsigned failure)
{
    switch (ccs_at(failure)) {
    case POSITIVE: set_A(THREE); break;
    case NEGATIVE: set_A(SIX); break;
    default: break; /* A +0 */
    }
}

/* ---- JETSLECT -- pp. 1039-1040 ---- */

/* JETSLECT: "RESET TO EXECUTE PHASE1" in 60 ms after T5TIME (an overflow
 * at the TS skips the TCF: "20 MS TO ASSURE A T5RUPT"), "RESET T6 TO
 * INITIALIZE THE JET CHANNELS IN 14 MS", T6's enable (channel 13's bit
 * 15). Then "EXAMINE THE TRANSLATION HAND CONTROLLER", channel 31's bits
 * 7-12 (clear for a command): the X, Y and Z translation indexes, "1 = +
 * XLN", "2 = - XLN", "3 = NO XLN"; with none, "ZERO ALL REQUESTS FOR
 * TRANSLATION" (NOXLNCMD). ATTKALMN -3 with the LEM on (DAPDATR1 bit 14),
 * -2 off, for the filter gains. XTRANS picks the quads for X translation:
 * + "USING BD-X", XNDX1 zeroed; - "USING AC-X", XNDX2 zeroed. */
void jetslect(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    set_A(DELTATT3);
    ad_at(ADDRESS_OF(T5TIME));
    if (ts(ADDRESS_OF(TIME5))) { /* TS TIME5; TCF +3 */
        set_A(DELATT20);
        ts(ADDRESS_OF(TIME5));
    }
    set_A(EQ_14MS);
    ts(ADDRESS_OF(TIME6));
    set_A(NEGMAX);
    wor_channel(013);
    qxch(ADDRESS_OF(QRUPT));
    set_A(XLNMASK);
    rxor_channel(031);
    mask(XLNMASK);
    if (A_is_zero()) { /* EXTEND; BZF NOXLNCMD */
        ts(ADDRESS_OF(XNDX1));
        ts(ADDRESS_OF(XNDX2));
        ts(ADDRESS_OF(YNDX));
        ts(ADDRESS_OF(ZNDX));
        pword();
        return;
    }
    ts(ADDRESS_OF(T5TEMP));
    mp(BIT9);
    mask(THREE);
    ts(ADDRESS_OF(XNDX1)); /* "AC QUAD  X-TRANSLATION INDEX" */
    ts(ADDRESS_OF(XNDX2)); /* "BD QUAD  X-TRANSLATION INDEX" */
    ca(ADDRESS_OF(T5TEMP));
    mp(BIT7);
    mask(THREE);
    ts(ADDRESS_OF(YNDX));
    ca(ADDRESS_OF(T5TEMP));
    mp(BIT5);
    mask(THREE);
    ts(ADDRESS_OF(ZNDX));
    ca(ADDRESS_OF(DAPDATR1));
    mask(BIT14);
    if (A_is_zero()) /* EXTEND; BZF NOLEM */
        set_A(negate(TWO));
    else
        set_A(negate(THREE));
    ts(ADDRESS_OF(ATTKALMN));
    switch (ccs_at(ADDRESS_OF(XTRANS))) {
    case POSITIVE: ts(ADDRESS_OF(XNDX1)); break;
    case NEGATIVE: ts(ADDRESS_OF(XNDX2)); break;
    default: break;
    }
    pword();
}

/* PWORD: "CHECK FOR PITCH COMMANDS", with the AC quads' failures
 * (RACFAIL) and X translation, and TABPCOM: the PYTABLE word. Its MASK
 * runs at the key TABPCOM +5. */
static void pword(void)
{
    rotation_index(ADDRESS_OF(TAU1), ADDRESS_OF(PINDEX));
    table_bias(ADDRESS_OF(RACFAIL), ADDRESS_OF(XNDX1));
    ad_at(ADDRESS_OF(PINDEX));
    ca(table(ADDRESS_OF(PYTABLE), a_word()));
    continue_at(TABPCOM + 5u);
}

/* ---- The pitch, yaw and roll words -- pp. 1041-1043 ---- */

/* TABPCOM +5: the pitch jets, PWORD1, and NPJETS, "NO. OF PITCH JETS".
 * YWORD and TABYCOM the same for yaw, with the BD quads' failures
 * (RBDFAIL) and X translation: YWORD1 and NYJETS. RWORD: "CHECK FOR ROLL
 * COMMANDS", and ACORBD's choice, "+, +0 = BD ROLL", "-, -0 = AC ROLL".
 * ACROLL: the AC quads' failures, or the Y translation (RXLNS), into
 * TABRCOM. */
void tabpcom_pjets(void)
{
    mask(PJETS);
    ts(ADDRESS_OF(PWORD1));
    mp(BIT7);
    ts(ADDRESS_OF(NPJETS));
    rotation_index(ADDRESS_OF(TAU2), ADDRESS_OF(YINDEX)); /* YWORD */
    table_bias(ADDRESS_OF(RBDFAIL), ADDRESS_OF(XNDX2));
    ad_at(ADDRESS_OF(YINDEX)); /* TABYCOM */
    ca(table(ADDRESS_OF(PYTABLE), a_word()));
    mask(YJETS);
    ts(ADDRESS_OF(YWORD1));
    mp(BIT5);
    ts(ADDRESS_OF(NYJETS));
    rotation_index(ADDRESS_OF(TAU), ADDRESS_OF(RINDEX)); /* RWORD */
    switch (ccs_at(ADDRESS_OF(ACORBD))) {
    case POSITIVE:
    case IS_PLUS_ZERO: continue_at(BDROLL); return;
    default: break;
    }
    table_bias(ADDRESS_OF(RACFAIL), ADDRESS_OF(YNDX)); /* ACROLL */
    ad_at(ADDRESS_OF(RINDEX));                         /* TABRCOM */
    tabrcom_rtable();
}

/* TABRCOM +1: the AC roll word from RTABLE, RWORD1. BDZCHECK: "CHECK FOR
 * Z-TRANSLATIONS ON BD", none (ZNDX +0 or less) at NOBDZ; else the BD
 * quads' failure bias, and on at BDZCHECK +7. */
void tabrcom_rtable(void)
{
    ca(table(ADDRESS_OF(RTABLE), a_word()));
    mask(ACRJETS);
    ts(ADDRESS_OF(RWORD1));
    ca(ADDRESS_OF(ZNDX));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF NOBDZ */
        continue_at(NOBDZ);
        return;
    }
    translation_bias(ADDRESS_OF(RBDFAIL));
    continue_at(BDZCHECK + 7u);
}

/* ---- Z translation on the BD quads -- p. 1044 ---- */

/* BDZCHECK +7: the Z translation's XLN1NDX. */
void bdzcheck_index(void)
{
    ad_at(table(ADDRESS_OF(XLN1NDX), fetch(ADDRESS_OF(ZNDX))));
    bdzcheck_yztable();
}

/* BDZCHECK +011: Z translation is possible "AS LONG AS ROLL COMMANDS CAN
 * BE SATISFIED WITH THE AC ROLL JETS": the YZTABLE word "ADD TO ROLL
 * COMMANDS" in T5TEMP, and NRJETS, "NET NO. OF +,- ROLL JETS ON" (its
 * bits 9-11 less 4). Not zero, ACRBDZ: "Z-TRANSLATION ACCEPTED". Zero,
 * TAUCHECK. (An overflow at the TS T5TEMP skips the EXTEND: MP BIT7 runs
 * as MASK BIT7. Either way A is small, and holds no overflow at the TS
 * NRJETS.) */
void bdzcheck_yztable(void)
{
    ca(table(ADDRESS_OF(YZTABLE), a_word()));
    mask(BDZJETS);
    ad_at(ADDRESS_OF(RWORD1));
    if (ts(ADDRESS_OF(T5TEMP)))
        mask(BIT7);
    else
        mp(BIT7);
    ad(NEG4);
    ts(ADDRESS_OF(NRJETS));
    if (A_is_zero()) /* EXTEND; BZF TAUCHECK */
        taucheck();
    else
        acrbdz();
}

/* ACRBDZ: "Z-TRANSLATION ACCEPTED EVEN THO WE MAY HAVE INTRODUCED AN
 * UNDESIREABLE ROLL"; "BRANCH TO JET ON-TIME CALCULATIONS". */
static void acrbdz(void)
{
    ca(ADDRESS_OF(T5TEMP));
    ts(ADDRESS_OF(RWORD1));
    continue_at(ROLLTIME);
}

/* TAUCHECK: "IF THE RESULTANT NET ROLL COMMANDS = 0 (WITH Z-TRANSLATION)
 * AND IF TAU = 0, THEN INCLUDE THE BD Z-TRANSLATION COMMANDS"; with TAU
 * not zero, NOBDZ. */
static void taucheck(void)
{
    switch (ccs_at(ADDRESS_OF(TAU))) {
    case POSITIVE:
    case NEGATIVE: nobdz(); break;
    default: acrbdz(); break;
    }
}

/* NOBDZ: "Z-TRANSLATION NOT ACCEPTED": NRJETS from the roll word alone
 * (bits 9-11 less 2), and the roll's on-time. (A, a small product less
 * 2, holds no overflow at the TS.) */
void nobdz(void)
{
    ca(ADDRESS_OF(RWORD1));
    mp(BIT7);
    ad(NEG2);
    ts(ADDRESS_OF(NRJETS));
    continue_at(ROLLTIME);
}

/* ---- Roll on the BD quads, Y translation on AC -- pp. 1045-1046 ---- */

/* BDROLL: "BD QUAD SELECTION FOR ROLL COMMANDS": the BD quads' failures,
 * or the Z translation (RZXLNS), into TABRZCMD: the BD roll word, RWORD1.
 * ACYCHECK: "ANY Y-TRANSLATION", none at NOACY; else the AC quads'
 * failure bias, the Y translation's XLN1NDX, the YZTABLE word, and on at
 * ACYCHECK +014. */
void bdroll(void)
{
    table_bias(ADDRESS_OF(RBDFAIL), ADDRESS_OF(ZNDX));
    ad_at(ADDRESS_OF(RINDEX)); /* TABRZCMD */
    ca(table(ADDRESS_OF(RTABLE), a_word()));
    mask(BDRJETS);
    ts(ADDRESS_OF(RWORD1));
    ca(ADDRESS_OF(YNDX)); /* ACYCHECK */
    if (A_is_zero()) {    /* EXTEND; BZF NOACY */
        continue_at(NOACY);
        return;
    }
    translation_bias(ADDRESS_OF(RACFAIL));
    ad_at(table(ADDRESS_OF(XLN1NDX), fetch(ADDRESS_OF(YNDX))));
    ca(table(ADDRESS_OF(YZTABLE), a_word()));
    mask(ACYJETS);
    continue_at(ACYCHECK + 014u);
}

/* ACYCHECK +014: added to the roll word in T5TEMP, and "FOR EXPLANATION
 * SEE CODING ON RTABLE": the net roll from bits 12-14. (An overflow at
 * the TS skips the EXTEND: MP BIT4 runs as MASK BIT4.) */
void acycheck_rword1(void)
{
    ad_at(ADDRESS_OF(RWORD1));
    if (ts(ADDRESS_OF(T5TEMP)))
        mask(BIT4);
    else
        mp(BIT4);
    acycheck_net();
}

/* ACYCHECK +020: NRJETS, "NO. OF NET ROLL JETS"; "IF NRJETS = 0",
 * TAUCHCK, else BDRACZ. (An overflow at the TS skips the EXTEND: the BZF
 * runs as TCF TAUCHCK.) */
void acycheck_net(void)
{
    ad(NEG4);
    if (ts(ADDRESS_OF(NRJETS)) || A_is_zero())
        tauchck();
    else
        bdracz();
}

/* BDRACZ: "Y-TRANSLATION ACCEPTED". */
void bdracz(void)
{
    ca(ADDRESS_OF(T5TEMP));
    ts(ADDRESS_OF(RWORD1));
    bdracz_rolltime();
}

/* BDRACZ +2: "BRANCH TO JET ON-TIME CALCULATIONS". */
void bdracz_rolltime(void)
{
    continue_at(ROLLTIME);
}

/* TAUCHCK: as TAUCHECK, with TAU not zero NOACY, else BDRACZ. */
void tauchck(void)
{
    switch (ccs_at(ADDRESS_OF(TAU))) {
    case POSITIVE:
    case NEGATIVE: noacy(); break;
    default: bdracz(); break;
    }
}

/* NOACY: "Y-TRANSLATION NOT ACCEPTED": NRJETS from the roll word alone,
 * and the roll's on-time. (A, a small product less 2, holds no overflow
 * at the TS.) */
void noacy(void)
{
    ca(ADDRESS_OF(RWORD1));
    mp(BIT4);
    ad(NEG2);
    ts(ADDRESS_OF(NRJETS));
    continue_at(ROLLTIME);
}

#endif
