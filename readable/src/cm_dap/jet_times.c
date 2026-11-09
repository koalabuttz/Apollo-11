/* SPDX-License-Identifier: MIT */
/* JET_SELECTION_LOGIC.agc (Comanche055 pp. 1050-1054, bank 17): the jet
 * on-times. Each axis' rotation command (TAU1 pitch, TAU2 yaw, TAU roll,
 * in 1-jet terms) becomes BLAST, the time its jets fire, by the number of
 * jets the jet selection logic chose (NPJETS, NYJETS, NRJETS: NJET holds
 * 1 over each count), and DFT, the change for the rate filter in 1-jet
 * terms. A command longer than "THE T5 CYCLE TIME" (.1 s) fires for .1 s
 * and leaves the rest in TAU for the next cycle, "SO THAT ONLY
 * X-TRANSLATIONS WILL CONTINUE ON SWITCH OVER TO TVC"; a shorter one fires
 * for at least "A MINIMUM IMPULSE (14MS)" and zeroes TAU. With no command
 * the axis goes straight to its channel words (PBYPASS, YBYPASS,
 * RBYPASS). The faithful C is c64-port/src/cm_jet_select.c, whose runs
 * stop at the keys of its parts: the functions here start at them. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

static void ptaupos(void);
static void ad14msp(void);
static void ytauneg(void);
static void rblast(void);

/* INDEX, then the word of NJET ("+1", "+1/2", "+1/3" and their
 * negatives, by the count of jets) or DFTMAX (+-.1, .2 or .3 s). */
static unsigned table(unsigned field, unsigned jets)
{
    return jet_index_read(field, fetch(jets));
}

/* ---- Pitch -- pp. 1050-1051 ---- */

/* PITCHTIM: "PITCH JET ON-TIME CALCULATION". With no pitch command, DFT1
 * +0 and on to PBYPASS; a negative one turns NPJETS's sign (PTAUNEG), so
 * the on-time comes out positive. */
void pitchtim(void)
{
    switch (ccs_at(ADDRESS_OF(TAU1))) {
    case POSITIVE: ptaupos(); break;
    case NEGATIVE:
        cs(ADDRESS_OF(NPJETS)); /* PTAUNEG */
        ts(ADDRESS_OF(NPJETS));
        ptaupos();
        break;
    default:
        ts(ADDRESS_OF(DFT1)); /* "NO PITCH ROTATION", A +0 */
        continue_at(PBYPASS);
        break;
    }
}

/* PTAUPOS: BLAST1, TAU1 over the number of jets. Over .1 s: DFT1 from
 * DFTMAX, TAU1 less it ("UPDATE TAU1"), and BLAST1 .1 s; else AD14MSP.
 * (At exactly .1 s the sum with =-.1SEC is -0, which BZMF takes; a sum
 * with a word that is not +0 is never +0. So for YTAUPOS and RBLAST.) */
static void ptaupos(void)
{
    ca(ADDRESS_OF(TAU1));
    mp_at(table(ADDRESS_OF(NJET), ADDRESS_OF(NPJETS)));
    ts(ADDRESS_OF(BLAST1));
    ad(EQ_M_1SEC);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF AD14MSP */
        ad14msp();
        return;
    }
    ca(table(ADDRESS_OF(DFTMAX), ADDRESS_OF(NPJETS)));
    ts(ADDRESS_OF(DFT1));
    com();
    ads(ADDRESS_OF(TAU1));
    set_A(EQ_P_1SEC); /* "LIMIT THE LENGTH OF PITCH ROTATION" */
    ts(ADDRESS_OF(BLAST1));
    continue_at(ASMBLWP);
}

/* AD14MSP: BLAST1 at least 14 ms ("MINIMUM IMPULSE TIME"). PBLASTOK: "THE
 * PITCH COMMANDS WILL BE COMPLETED" within the cycle: DFT1 the
 * on-time by the number of jets, "FOR USE IN UPDATING RATE FILTER", and
 * TAU1 zeroed (the product's top word, +0). */
static void ad14msp(void)
{
    cs(ADDRESS_OF(BLAST1));
    ad(EQ_14MS);
    if (!A_is_zero_or_minus()) { /* EXTEND; BZMF PBLASTOK */
        set_A(EQ_14MS);
        ts(ADDRESS_OF(BLAST1));
    }
    ca(ADDRESS_OF(BLAST1)); /* PBLASTOK */
    mp_at(ADDRESS_OF(NPJETS));
    lxch(ADDRESS_OF(DFT1));
    ts(ADDRESS_OF(TAU1));
    continue_at(ASMBLWP);
}

/* ---- Yaw -- p. 1052 ---- */

/* YAWTIME: "YAW JET ON-TIME CALCULATION", as PITCHTIM's with TAU2,
 * NYJETS, BLAST2 and DFT2, on to YBYPASS or ASMBLWY. Its CCS's four
 * words, and the TS DFT2 after them, are keys of their own (below). */
void yawtime(void)
{
    switch (ccs_at(ADDRESS_OF(TAU2))) {
    case POSITIVE: ytaupos(); break;
    case NEGATIVE: ytauneg(); break;
    default: yawtime_dft2(); break;
    }
}

/* YAWTIME +1: TCF YTAUPOS. */
void yawtime_to_ytaupos(void)
{
    ytaupos();
}

/* YAWTIME +2: TCF +2, to the TS DFT2. */
void yawtime_to_dft2(void)
{
    yawtime_dft2();
}

/* YAWTIME +3: TCF YTAUNEG. */
void yawtime_to_ytauneg(void)
{
    ytauneg();
}

/* YAWTIME +4: DFT2 +0, "NO YAW ROTATION COMMANDS", and on to YBYPASS.
 * (From the CCS, A is +0; an overflow in A at the key skips the TCF, to
 * YTAUNEG.) */
void yawtime_dft2(void)
{
    if (ts(ADDRESS_OF(DFT2))) {
        ytauneg();
        return;
    }
    continue_at(YBYPASS);
}

/* YTAUNEG: NYJETS's sign turned. */
static void ytauneg(void)
{
    cs(ADDRESS_OF(NYJETS));
    ts(ADDRESS_OF(NYJETS));
    ytaupos();
}

/* YTAUPOS: BLAST2, TAU2 over the number of jets; over .1 s on at
 * YTAUPOS +010, else AD14MSY. */
void ytaupos(void)
{
    ca(ADDRESS_OF(TAU2));
    mp_at(table(ADDRESS_OF(NJET), ADDRESS_OF(NYJETS)));
    ts(ADDRESS_OF(BLAST2));
    ad(EQ_M_1SEC);
    if (A_is_zero_or_minus()) /* EXTEND; BZMF AD14MSY */
        ad14msy();
    else
        ytaupos_dftmax();
}

/* YTAUPOS +010: "YAW COMMANDS WILL LAST LONGER THAN .1SEC": DFT2 from
 * DFTMAX, TAU2 less it ("DECREMENT TAU2"), and BLAST2 .1 s. */
void ytaupos_dftmax(void)
{
    ca(table(ADDRESS_OF(DFTMAX), ADDRESS_OF(NYJETS)));
    ts(ADDRESS_OF(DFT2));
    com();
    ads(ADDRESS_OF(TAU2));
    set_A(EQ_P_1SEC); /* "LIMIT THE LENGTH OF YAW ROTATION COMMAND" */
    ts(ADDRESS_OF(BLAST2));
    continue_at(ASMBLWY);
}

/* AD14MSY: BLAST2 at least 14 ms. YBLASTOK: "YAW COMMANDS WILL BE
 * COMPLETED WITHIN THE T5CYCLE TIME": DFT2, and TAU2 zeroed. */
void ad14msy(void)
{
    cs(ADDRESS_OF(BLAST2));
    ad(EQ_14MS);
    if (!A_is_zero_or_minus()) { /* EXTEND; BZMF YBLASTOK */
        set_A(EQ_14MS);
        ts(ADDRESS_OF(BLAST2));
    }
    ca(ADDRESS_OF(BLAST2)); /* YBLASTOK */
    mp_at(ADDRESS_OF(NYJETS));
    lxch(ADDRESS_OF(DFT2));
    ts(ADDRESS_OF(TAU2));
    continue_at(ASMBLWY);
}

/* ---- Roll -- pp. 1053-1054 ---- */

/* ROLLTIME: "ROLL ON-TIME CALCULATION". With no roll command, DFT from
 * DFTMAX all the same ("UPDATE DFT EVEN THO NO ROLL COMMANDS ARE
 * PRESENT": a translation's roll jets) and on to RBYPASS. Either sign
 * goes to RBLAST, NRJETS carrying the sign of the net roll. */
void rolltime(void)
{
    switch (ccs_at(ADDRESS_OF(TAU))) {
    case POSITIVE:
    case NEGATIVE: rblast(); break;
    default:
        ca(table(ADDRESS_OF(DFTMAX), ADDRESS_OF(NRJETS)));
        ts(ADDRESS_OF(DFT));
        continue_at(RBYPASS);
        break;
    }
}

/* RBLAST: BLAST ("AN INTERMEDIATE VARIABLE USED IN DETERMINING THE JET
 * ON-TIMES"), TAU over the net number of roll jets. At most .1 s,
 * AD14MSR; over, DFT from DFTMAX and TAU less it, and on at RBLAST +015
 * (BLAST .1 s). */
static void rblast(void)
{
    ca(ADDRESS_OF(TAU));
    mp_at(table(ADDRESS_OF(NJET), ADDRESS_OF(NRJETS)));
    ts(ADDRESS_OF(BLAST));
    ad(EQ_M_1SEC);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF AD14MSR */
        continue_at(AD14MSR);
        return;
    }
    ca(table(ADDRESS_OF(DFTMAX), ADDRESS_OF(NRJETS)));
    ts(ADDRESS_OF(DFT));
    com();
    ads(ADDRESS_OF(TAU));
    continue_at(RBLAST + 015u);
}

#endif
