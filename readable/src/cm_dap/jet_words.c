/* SPDX-License-Identifier: MIT */
/* JET_SELECTION_LOGIC.agc (Comanche055 pp. 1053-1058, bank 17): the
 * second channel words. "WHEN THE ROTATION COMMANDS ARE COMPLETED, IT IS
 * NECESSARY TO REPLACE THESE COMMANDS BY NEW COMMANDS WHICH CONTINUE ON
 * WITH THE TRANSLATIONS IF ANY ARE PRESENT": for each axis a second word
 * (RWORD2, PWORD2, YWORD2) the T6 program loads when the axis' on-time
 * (BLAST, BLAST1, BLAST2) has run. With no rotation command on an axis
 * (its BYPASS), the second word is the first and the on-time +0. A
 * translation's roll jets that go on after the roll's on-time add their
 * share of the cycle to DFT. The order is roll, pitch, yaw (ROLLTIME,
 * PITCHTIM, YAWTIME in jet_times.c), then the sort of the on-times
 * (T6SETUP, jet_sort.c). The faithful C is c64-port/src/cm_jet_select.c,
 * whose runs stop at the keys of its parts: the functions here start at
 * them. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

static void acbd2y(void);
static void no2y(void);
static void fpx2_pjets(void);
static void fyx2_yjets(void);

/* INDEX, then CA of a word of the file's tables. */
static unsigned table(unsigned field, word index)
{
    return jet_index_read(field, index);
}

/* ---- The end of the roll on-time -- p. 1054 ---- */

/* RBLAST +015: "LIMIT THE LENGTH OF ROLL ROTATION COMMANDS TO 0.1 SEC"
 * (BLAST), so that only the Y and Z translations go on. */
void rblast_limit(void)
{
    set_A(EQ_P_1SEC);
    ts(ADDRESS_OF(BLAST));
    asmblwr();
}

/* AD14MSR: BLAST at least 14 ms ("MINIMUM IMPULSE TIME"). */
void ad14msr(void)
{
    cs(ADDRESS_OF(BLAST));
    ad(EQ_14MS);
    if (!A_is_zero_or_minus()) { /* EXTEND; BZMF RBLASTOK */
        set_A(EQ_14MS);
        ts(ADDRESS_OF(BLAST));
    }
    rblastok();
}

/* RBLASTOK: DFT the on-time by the net number of roll jets, and TAU
 * zeroed (the product's top word, +0). */
void rblastok(void)
{
    ca(ADDRESS_OF(BLAST));
    mp_at(ADDRESS_OF(NRJETS));
    lxch(ADDRESS_OF(DFT));
    ts(ADDRESS_OF(TAU));
    asmblwr();
}

/* ---- The second pitch word -- p. 1055 ---- */

/* ASMBLWP: "GENERATION OF THE SECOND PITCH(X-TRANS) WORD...PWORD2": the
 * PYTABLE word for the X translation alone (its XLNNDX bias, no
 * rotation). "IF FAILURE ON AC IGNORE X-TRANSLATION": FPX2. */
void asmblwp(void)
{
    switch (ccs_at(ADDRESS_OF(RACFAIL))) {
    case POSITIVE:
    case NEGATIVE: fpx2(); return;
    default: break;
    }
    ca(table(ADDRESS_OF(XLNNDX), fetch(ADDRESS_OF(XNDX1))));
    ca(table(ADDRESS_OF(PYTABLE), a_word()));
    fpx2_pjets();
}

/* FPX2: PYTABLE's first word (no jets), unindexed. */
void fpx2(void)
{
    ca(ADDRESS_OF(PYTABLE));
    fpx2_pjets();
}

/* FPX2 +1: the pitch jets, PWORD2, and on to YAWTIME. */
void fpx2_plus1(void)
{
    fpx2_pjets();
}

static void fpx2_pjets(void)
{
    mask(PJETS);
    ts(ADDRESS_OF(PWORD2));
    continue_at(YAWTIME);
}

/* PBYPASS: no pitch command: "THE T6 PROGRAM WILL LOAD PWORD2 UPON
 * ENTRY", PWORD1 itself, and BLAST1 +0, "THERE IS NO PWORD2". */
void pbypass(void)
{
    ca(ADDRESS_OF(PWORD1));
    ts(ADDRESS_OF(PWORD2));
    set_A(ZERO);
    ts(ADDRESS_OF(BLAST1));
    continue_at(YAWTIME);
}

/* ---- The second roll word -- pp. 1056-1057 ---- */

/* ASMBLWR: "GENERATION OF THE SECOND ROLL (Y,Z) WORD (RWORD2)": "CHECK
 * FOR Y-TRANS" (ACBD2Y), then for Z. With none, RWORD2 +0 (NO2Y). (A
 * negative YNDX leaves its CCS value for RWORD2, -0 leaves RWORD2 as it
 * was; the translation indexes are never negative in flight.) */
void asmblwr(void)
{
    switch (ccs_at(ADDRESS_OF(YNDX))) {
    case POSITIVE: acbd2y(); break;
    case IS_PLUS_ZERO: no2y(); break;
    case NEGATIVE: no2y_plus1(); break;
    case IS_MINUS_ZERO: no2y_plus2(); break;
    }
}

/* NO2Y: no Y translation, RWORD2 +0. */
static void no2y(void)
{
    set_A(ZERO);
    no2y_plus1();
}

/* NO2Y +1: A into RWORD2. (An overflow in A at the key skips to the TCF
 * ACBD2Z.) */
void no2y_plus1(void)
{
    if (ts(ADDRESS_OF(RWORD2))) {
        continue_at(ACBD2Z);
        return;
    }
    no2y_plus2();
}

/* NO2Y +2: "CHECK FOR Z-TRANS": ACBD2Z; with none, NO2Z. ("RWORD2
 * ASSEMBLED", on to PITCHTIM. A negative ZNDX adds its CCS value to
 * RWORD2; -0 adds nothing.) */
void no2y_plus2(void)
{
    switch (ccs_at(ADDRESS_OF(ZNDX))) {
    case POSITIVE: continue_at(ACBD2Z); break;
    case IS_PLUS_ZERO: no2z(); break;
    case NEGATIVE: no2z_plus1(); break;
    case IS_MINUS_ZERO: continue_at(PITCHTIM); break;
    }
}

/* NO2Z: no Z translation, +0 to RWORD2. */
void no2z(void)
{
    set_A(ZERO);
    no2z_plus1();
}

/* NO2Z +1: A added to RWORD2, "RWORD2 ASSEMBLED". */
void no2z_plus1(void)
{
    ads(ADDRESS_OF(RWORD2));
    continue_at(PITCHTIM);
}

/* ACBD2Y: a Y translation. Rolling on the BD quads (ACORBD + or +0), it
 * "CAN DO Y-TRANS" on AC, at AC2Y. Rolling on AC ("USING AC FOR ROLL"),
 * none if "AC HAS FAILED" (NO2Y); else the RTABLE word for the Y
 * translation on the AC roll jets, at ACBD2Y +012. */
static void acbd2y(void)
{
    switch (ccs_at(ADDRESS_OF(ACORBD))) {
    case POSITIVE:
    case IS_PLUS_ZERO: continue_at(AC2Y); return;
    default: break;
    }
    switch (ccs_at(ADDRESS_OF(RACFAIL))) {
    case POSITIVE:
    case NEGATIVE: no2y(); return;
    default: break;
    }
    ca(table(ADDRESS_OF(XLNNDX), fetch(ADDRESS_OF(YNDX)))); /* "NO FAILURES, CAN DO Y" */
    continue_at(ACBD2Y + 012u);
}

/* ACBD2Y +012: the RTABLE word. */
void acbd2y_rtable(void)
{
    ca(table(ADDRESS_OF(RTABLE), a_word()));
    acbd2y_acrjets();
}

/* ACBD2Y +014: its AC roll jets, into RWORD2 at NO2Y +1. */
void acbd2y_acrjets(void)
{
    mask(ACRJETS);
    correct_L(); /* the MASK ends, as every instruction does, with L sign-corrected */
    continue_at(NO2Y + 1u);
}

/* AC2Y: the Y translation on the AC quads while BD rolls: the YZTABLE
 * word, by the AC quads' failure bias, at AC2Y +4. */
void ac2y(void)
{
    translation_bias(ADDRESS_OF(RACFAIL));
    ac2y_index();
}

/* AC2Y +4: the Y translation's XLN1NDX, the YZTABLE word. */
void ac2y_index(void)
{
    ad_at(table(ADDRESS_OF(XLN1NDX), fetch(ADDRESS_OF(YNDX))));
    ca(table(ADDRESS_OF(YZTABLE), a_word()));
    ac2y_acyjets();
}

/* AC2Y +010: its jets, RWORD2, and NRJETS, the net roll jets they fire
 * (bits 12-14 less 2). They fire for the rest of the cycle (.1 s less
 * BLAST), which goes into DFT by NRJETS; on at NO2Y +2. (A, a small
 * product less 2, holds no overflow at the TS NRJETS.) */
void ac2y_acyjets(void)
{
    mask(ACYJETS);
    ts(ADDRESS_OF(RWORD2));
    mp(BIT4);
    ad(NEG2);
    ts(ADDRESS_OF(NRJETS));
    cs(ADDRESS_OF(BLAST));
    ad(EQ_P_1SEC);
    mp_at(ADDRESS_OF(NRJETS));
    ca(L_ADDRESS);
    ads(ADDRESS_OF(DFT));
    continue_at(NO2Y + 2u);
}

/* ACBD2Z: a Z translation. Rolling on BD ("USING BD-ROLL"), BDF2Z "MUST
 * CHECK FOR BD FAILURES"; "USING AC FOR ROLL, CAN DO Z-TRANS": the BD
 * quads' failure bias, at ACBD2Z +010. */
void acbd2z(void)
{
    switch (ccs_at(ADDRESS_OF(ACORBD))) {
    case POSITIVE:
    case IS_PLUS_ZERO: continue_at(BDF2Z); return;
    default: break;
    }
    translation_bias(ADDRESS_OF(RBDFAIL));
    acbd2z_index();
}

/* ACBD2Z +010: the Z translation's YZTABLE word, its BD jets added to
 * RWORD2, and NRJETS (bits 9-11 less 2), the rest of the cycle by NRJETS,
 * on at ACBD2Z +026 into DFT. (A, a small product less 2, holds no
 * overflow at the TS NRJETS.) */
void acbd2z_index(void)
{
    ad_at(table(ADDRESS_OF(XLN1NDX), fetch(ADDRESS_OF(ZNDX))));
    ca(table(ADDRESS_OF(YZTABLE), a_word()));
    mask(BDZJETS);
    ads(ADDRESS_OF(RWORD2));
    mp(BIT7);
    ad(NEG2);
    ts(ADDRESS_OF(NRJETS));
    cs(ADDRESS_OF(BLAST));
    ad(EQ_P_1SEC);
    mp_at(ADDRESS_OF(NRJETS));
    continue_at(ACBD2Z + 026u);
}

/* ACBD2Z +026: the product's low word into DFT, and on to PITCHTIM. */
void acbd2z_dft(void)
{
    ca(L_ADDRESS);
    ads(ADDRESS_OF(DFT));
    continue_at(PITCHTIM);
}

/* BDF2Z: the Z translation on the BD roll jets: none "USING BD-ROLL AND
 * BD HAS FAILED" (NO2Z); else at BDF2Z +4. */
void bdf2z(void)
{
    switch (ccs_at(ADDRESS_OF(RBDFAIL))) {
    case POSITIVE:
    case NEGATIVE: continue_at(NO2Z); return;
    default: break;
    }
    bdf2z_index();
}

/* BDF2Z +4: the RTABLE word for the Z translation, its BD roll jets, on
 * at NO2Z +1. */
void bdf2z_index(void)
{
    ca(table(ADDRESS_OF(XLNNDX), fetch(ADDRESS_OF(ZNDX))));
    ca(table(ADDRESS_OF(RTABLE), a_word()));
    mask(BDRJETS);
    continue_at(NO2Z + 1u);
}

/* RBYPASS: no roll command: RWORD2 the first word, and BLAST +0 at
 * RBYPASS +2. */
void rbypass(void)
{
    ca(ADDRESS_OF(RWORD1));
    ts(ADDRESS_OF(RWORD2));
    rbypass_blast();
}

/* RBYPASS +2: BLAST +0, and on to PITCHTIM. */
void rbypass_blast(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(BLAST));
    continue_at(PITCHTIM);
}

/* ---- The second yaw word -- p. 1058 ---- */

/* ASMBLWY: "GENERATION OF THE SECOND YAW (X-TRANS) WORD...YWORD2", as
 * ASMBLWP's with the BD quads: "IF FAILURE ON BD IGNORE X-TRANSLATION". */
void asmblwy(void)
{
    switch (ccs_at(ADDRESS_OF(RBDFAIL))) {
    case POSITIVE:
    case NEGATIVE: fyx2(); return;
    default: break;
    }
    asmblwy_index();
}

/* ASMBLWY +4: the PYTABLE word for the X translation. */
void asmblwy_index(void)
{
    ca(table(ADDRESS_OF(XLNNDX), fetch(ADDRESS_OF(XNDX2))));
    ca(table(ADDRESS_OF(PYTABLE), a_word()));
    fyx2_yjets();
}

/* FYX2: PYTABLE's first word, unindexed. */
void fyx2(void)
{
    ca(ADDRESS_OF(PYTABLE));
    fyx2_yjets();
}

/* FYX2 +1: the yaw jets, YWORD2, and on to T6SETUP. */
static void fyx2_yjets(void)
{
    mask(YJETS);
    ts(ADDRESS_OF(YWORD2));
    t6setup();
}

/* YBYPASS: no yaw command: YWORD2 the first word, BLAST2 +0. */
void ybypass(void)
{
    ca(ADDRESS_OF(YWORD1));
    ts(ADDRESS_OF(YWORD2));
    set_A(ZERO);
    ts(ADDRESS_OF(BLAST2));
    t6setup();
}

#endif
