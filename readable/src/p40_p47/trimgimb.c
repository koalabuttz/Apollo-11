/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, part 3 (Luminary099 pp. 781-784, bank 27): TRIMGIMB, a
 * waitlist task in basic code, and S41.1 in the listing's notation. The
 * faithful C is c64-port/src/lm_p40_p47.c.
 *
 * TRIMGIMB, "(FORMERLY S40.6)", which R03 starts by the waitlist, "TRIMS
 * DPS ENGINE TO MINIMIZE THRUST/CG OFFSET.": it drives the descent
 * engine's gimbal to full + pitch and + roll for a minute, then back in -
 * pitch and - roll for PITTIME and ROLLTIME, through the gimbal drive
 * bits of channel 12 (bits 9 and 10 pitch, 11 and 12 roll). The second of
 * the two to finish starts TRIMDONE, back in R03; GMBDRVSW tells which
 * finished first.
 *
 * S41.1 turns a velocity in reference coordinates to the LM's body axes.
 *
 * Each call out of the file leaves Z at the routine and Q at the word
 * after the TC. */
#include "p40_p47.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- TRIMGIMB -- pp. 781-782 ---- */

/* TRIMGIMB: GMBDRVSW clear (DOWNFLAG), "GMBDRVSW FLAG IS SET WHEN EITHER
 * ROLL OR", "PITCH IS COMPLETED, WHICHEVER IS FIRST." */
void trimgimb(void)
{
    tc_to(TRIMGIMB, DOWNFLAG); /* ADRES GMBDRVSW */
}

/* "TURN OFF - PITCH, - ROLL, IF ON." (PRIO5: bits 10 and 12), "TURN ON +
 * PITCH, + ROLL." (EBANK5: bits 9 and 11), and "WAIT ONE MINUTE TO MAKE
 * SURE ENGINE IS", "AT FULL + PITCH AND FULL + ROLL" (FIXDELAY, DEC
 * 6000). */
void trimgimb_after_downflag(void)
{
    set_A(negate(PRIO5));
    wand_channel(CHAN12); /* EXTEND; WAND */
    set_A(EBANK5);
    wor_channel(CHAN12); /* EXTEND; WOR */
    tc_to(TRIMGIMB + 010u, FIXDELAY);
}

/* "TURN OFF + PITCH, + ROLL.", "TURN ON - PITCH, - ROLL."; "GET TIME TO
 * SHUT OFF - PITCH AND SET UP", "TWIDDLE-TASK TO TURN IT OFF THEN"
 * (PITCHOFF after PITTIME), */
void trimgimb_after_fixdelay(void)
{
    set_A(negate(EBANK5));
    wand_channel(CHAN12); /* EXTEND; WAND */
    set_A(PRIO5);
    wor_channel(CHAN12);             /* EXTEND; WOR */
    ca(ADDRESS_OF(PITTIME));         /* CAE */
    tc_to(TRIMGIMB + 021u, TWIDDLE); /* ADRES PITCHOFF */
}

/* and "GET TIME TO SHUT OFF - ROLL AND GO AWAY", "UNTIL THEN" (VARDELAY
 * for ROLLTIME). */
void trimgimb_after_twiddle(void)
{
    ca(ADDRESS_OF(ROLLTIME)); /* CAE */
    tc_to(TRIMGIMB + 024u, VARDELAY);
}

/* ROLLOVER: "IF HERE INLINE (ROLL DONE) IS PITCH DONE", "IF HERE FROM
 * PITCHOFF, IS ROLL DONE?" (GMBDRVSW). "NO.  SET FLAG, ROLL OR PITCH
 * DONE.": GMBDRVSW set (UPFLAG). Yes: "RETURN TO R03." (TRIMDONE as a
 * job, NOVAC). */
static void rollover(void)
{
    ca(ADDRESS_OF(FLAGWRD6));
    mask(GMBDRBIT);
    if (A_is_zero()) {                /* EXTEND; BZF PITCHOFF +4 */
        tc_to(PITCHOFF + 4u, UPFLAG); /* ADRES GMBDRVSW */
        return;
    }
    set_A(PRIO10);
    tc_to(ROLLOVER + 5u, NOVAC); /* 2CADR TRIMDONE */
}

/* After ROLLTIME: "SHUT OFF ROLL" (BIT12), and ROLLOVER. */
void trimgimb_after_vardelay(void)
{
    set_A(negate(BIT12));
    wand_channel(CHAN12); /* EXTEND; WAND */
    rollover();
}

/* After NOVAC: the task ends. */
void rollover_after_novac(void)
{
    tc_to(ROLLOVER + 010u, TASKOVER);
}

/* PITCHOFF, the task after PITTIME: "SHUT OFF PITCH" (BIT10), and "SEE
 * IF ROLL HAS FINISHED ALSO." (TCF ROLLOVER). */
void pitchoff(void)
{
    set_A(negate(BIT10));
    wand_channel(CHAN12); /* EXTEND; WAND */
    rollover();
}

/* After UPFLAG, "ROLL DONE; OR PITCH DONE; BUT NOT BOTH.": the task
 * ends. */
void pitchoff_after_upflag(void)
{
    tc_to(PITCHOFF + 6u, TASKOVER);
}

/* ---- S41.1 -- pp. 783-784 ---- */

/* S41_1_CODE: "CONVERT VECTOR IN MPAC FROM REF AT 2(+7)", "TO SM AND
 * RESCALE DUE TO HALF-UNIT MATRIX" (REFSMMAT), then "CONVERT TO BODY AT
 * 2(+7) USING PRESENT", "CDU ANGLES.  CDU*SMNB WILL RETURN" (GOTO
 * CDU*SMNB). */
static const word s41_1_words[] = {
    ORDERS(MXV, VSL1),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDER(GOTO),
    ADDRESS(CDU_STAR_SMNB),
};

const struct interpretive_code S41_1_CODE = {027u, S41_1, COUNT(s41_1_words), s41_1_words};

#endif
