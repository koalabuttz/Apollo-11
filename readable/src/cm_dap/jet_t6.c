/* SPDX-License-Identifier: MIT */
/* JET_SELECTION_LOGIC.agc (Comanche055 pp. 1061-1062, bank 17): "T6
 * PROGRAM AND CHANNEL SETUP", the T6 interrupt that switches the jets.
 * The first T6RUPT of a cycle puts the first channel words on the jets
 * (channel 6 roll, channel 5 pitch and yaw); then, with the on-times
 * sorted (jet_sort.c), each later one replaces an axis' first word by its
 * second (REPLACER, REPLACEP, REPLACEY, by the axis' branch index) as its
 * on-time runs out, and sets TIME6 for the next. The faithful C is
 * c64-port/src/cm_jet_select.c, whose runs stop at the keys of its parts:
 * the functions here start at them, and a routine's TC Q goes on at a key
 * of the same part (the binding runs it). */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

static void t6l1(void);
static void replace(void);
static void enabt6(void);

/* INDEX BLAST +1 (and so on); TC REPLACER: the axis' channel word routine
 * by its branch index, 0 (REPLACER, roll), 4 (REPLACEP, pitch) or 11
 * (REPLACEY, yaw), Q the word after the TC. Not modelled: any other index
 * (a TC into the routines' middle). */
static void tc_replacer(unsigned branch_index, word at)
{
    switch (fetch(branch_index)) {
    case 0u: tc_to(at, REPLACER); break;
    case 4u: tc_to(at, REPLACEP); break;
    case 11u: tc_to(at, REPLACEY); break;
    default: not_modelled(); break;
    }
}

/* ---- T6START -- p. 1061 ---- */

/* T6START: the T6 interrupt. */
void t6start(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    t6start_plus1();
}

/* T6START +1: "CHECK TO SEE IF TIME6 WAS RESET AFTER T6RUPT OCCURED(IN
 * T5RUPT)": "IF SO WAIT FOR NEXT T6RUPT BEFORE TAKING ACTION" (RESUME).
 * "IF BIT1 IS 0 RESET TO 1 AND INITIALIZE CHANNEL" (RCSFLAGS): the first
 * T6RUPT of a cycle, channel 6 RWORD1 and channel 5 PWORD1 plus YWORD1,
 * at T6START +020; else T6RUPTOR. */
void t6start_plus1(void)
{
    qxch(ADDRESS_OF(QRUPT));
    switch (ccs_at(ADDRESS_OF(TIME6))) {
    case POSITIVE:
    case NEGATIVE: continue_at(RESUME); return;
    default: break;
    }
    cs(ADDRESS_OF(RCSFLAGS));
    mask(BIT1);
    if (A_is_zero()) { /* EXTEND; BZF T6RUPTOR */
        t6ruptor();
        return;
    }
    ads(ADDRESS_OF(RCSFLAGS));
    ca(ADDRESS_OF(RWORD1)); /* "INITIALIZE CHANNELS 5,6 WITH WORD1" */
    write_channel(6);
    ca(ADDRESS_OF(PWORD1));
    t6start_yword1();
}

/* T6START +020: plus YWORD1, into channel 5. */
void t6start_yword1(void)
{
    ad_at(ADDRESS_OF(YWORD1));
    write_channel(5);
    t6ruptor();
}

/* T6RUPTOR: the axes in order of their on-times. A positive on-time is
 * the wait to the next T6RUPT (ZBLAST); +0 or -0, its time has come:
 * REPLACE its first word; negative (-1, done), the next axis (T6L1). */
void t6ruptor(void)
{
    switch (ccs_at(ADDRESS_OF(BLAST))) {
    case POSITIVE: continue_at(ZBLAST); break; /* "ZERO BLAST1" */
    case NEGATIVE: t6l1(); break;
    default: replace(); break;
    }
}

/* T6L1: the second axis, as the first. */
static void t6l1(void)
{
    switch (ccs_at(ADDRESS_OF(BLAST1))) {
    case POSITIVE: continue_at(ZBLAST1); break;
    case NEGATIVE: t6l2(); break;
    default: continue_at(REPLACE1); break;
    }
}

/* T6L2: the third, and with all done, RESUME. */
void t6l2(void)
{
    switch (ccs_at(ADDRESS_OF(BLAST2))) {
    case POSITIVE: continue_at(ZBLAST2); break;
    case NEGATIVE: continue_at(RESUME); break;
    default: continue_at(REPLACE2); break;
    }
}

/* ---- REPLACE -- pp. 1061-1062 ---- */

/* REPLACE: the first axis' second word on its jets, and it returns to
 * REPLACE +2. */
static void replace(void)
{
    tc_replacer(ADDRESS_OF(BLAST) + 1u, REPLACE + 1u);
}

/* REPLACE +2: BLAST -1, done; on to T6L1. */
void replace_plus2(void)
{
    set_A(negate(ONE));
    ts(ADDRESS_OF(BLAST));
    t6l1();
}

/* REPLACE1: the second axis', returning to REPLACE1 +2. */
void replace1(void)
{
    tc_replacer(ADDRESS_OF(BLAST1) + 1u, REPLACE1 + 1u);
}

/* REPLACE1 +2: BLAST1 -1; on to T6L2. */
void replace1_plus2(void)
{
    set_A(negate(ONE));
    ts(ADDRESS_OF(BLAST1));
    continue_at(T6L2);
}

/* REPLACE2: the third axis', returning to REPLACE2 +2. */
void replace2(void)
{
    tc_replacer(ADDRESS_OF(BLAST2) + 1u, REPLACE2 + 1u);
}

/* REPLACE2 +2: BLAST2 -1, and RESUME. */
void replace2_plus2(void)
{
    set_A(negate(ONE));
    ts(ADDRESS_OF(BLAST2));
    continue_at(RESUME);
}

/* REPLACER: "INITIALIZE CHANNELS 5,6 WITH WORD2": RWORD2 into channel 6
 * (roll). */
void replacer(void)
{
    ca(ADDRESS_OF(RWORD2));
    write_channel(6);
    tc_q();
}

/* REPLACEP: PWORD2 into channel 5's pitch jets, its yaw jets kept. Its
 * TC Q is a key of its own (REPLACEP +6). */
void replacep(void)
{
    set_A(YJETS);
    rand_channel(5);
    ad_at(ADDRESS_OF(PWORD2));
    write_channel(5);
    replacep_return();
}

/* REPLACEP +6: TC Q. */
void replacep_return(void)
{
    tc_q();
}

/* REPLACEY: YWORD2 into channel 5's yaw jets, its pitch jets kept. */
void replacey(void)
{
    set_A(PJETS);
    rand_channel(5);
    ad_at(ADDRESS_OF(YWORD2));
    write_channel(5);
    tc_q();
}

/* ---- The next T6RUPT -- p. 1062 ---- */

/* ZBLAST: the first axis' on-time to TIME6, BLAST +0 (its words change
 * at the next T6RUPT). ZBLAST +1, a key of its own, is the XCH. */
void zblast(void)
{
    set_A(ZERO);
    zblast_plus1();
}

/* ZBLAST +1. */
void zblast_plus1(void)
{
    xch(ADDRESS_OF(BLAST));
    enabt6();
}

/* ZBLAST1: the second axis'. */
void zblast1(void)
{
    set_A(ZERO);
    xch(ADDRESS_OF(BLAST1));
    enabt6();
}

/* ZBLAST2: the third axis'. */
void zblast2(void)
{
    set_A(ZERO);
    xch(ADDRESS_OF(BLAST2));
    enabt6();
}

/* ENABT6: TIME6 the wait, "ENABLE T6RUPT" (channel 13's bit 15), and
 * RESUME. */
static void enabt6(void)
{
    ts(ADDRESS_OF(TIME6));
    set_A(NEGMAX);
    wor_channel(013);
    continue_at(RESUME);
}

#endif
