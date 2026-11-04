/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, part 1 (Luminary099 pp. 759-761, bank 36): IMPLBURN and
 * the engine off routines. The faithful C is c64-port/src/lm_p40_p47.c.
 *
 * IMPLBURN, a task, sets up the engine's cutoff for a short burn: ENGOFTSK
 * after the time to go (GETDT, TWIDDLE). ENGOFTSK, the task, calls
 * ENGINOFF, which "ALLOWS ENGINOFF ET AL TO BE USED BOTH BY WAITLIST AND BY
 * TC IBNKCALL". ENGINOFF starts POSTBURN as a job, COASTSET as a task
 * (ENGINOF2), sets the idle bit (ENGINOF1), turns the ullage off, notes the
 * time of the event in TEVENT (ENGINOF4) and turns the engine off
 * (ENGINOF3); each of these is also an entry of its own.
 *
 * Each call out of the file leaves Z at the routine and Q at the word
 * after the TC. */
#include "p40_p47.h"
#include "../agc/names.h"

#if LUMINARY099

static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- IMPLBURN -- pp. 759-760 ---- */

/* IMPLBURN: the time to go (TGO +1) as a delay of at least a
 * centisecond (GETDT), */
void implburn(void)
{
    ca(ADDRESS_OF(TGO) + 1u);
    tc_at(IMPLBURN + 1u);
    getdt();
    implburn_after_getdt();
}

/* and ENGOFTSK after it (TWIDDLE). */
void implburn_after_getdt(void)
{
    tc_to(IMPLBURN + 2u, TWIDDLE); /* ADRES ENGOFTSK */
}

/* "TURN OFF IGNFLAG", */
void implburn_after_twiddle(void)
{
    tc_to(IMPLBURN + 4u, DOWNFLAG); /* ADRES IGNFLAG */
}

/* "TURN OFF ASTNFLG", */
void implburn_after_ignflag(void)
{
    tc_to(IMPLBURN + 6u, DOWNFLAG); /* ADRES ASTNFLAG */
}

/* "TURN OFF IMPULSW", */
void implburn_after_astnflag(void)
{
    tc_to(IMPLBURN + 010u, DOWNFLAG); /* ADRES IMPULSW */
}

/* "RESTART PROTECT ENGOFTSK (ENGINOFF)" (PHASCHNG, OCT 40114), */
void implburn_after_impulsw(void)
{
    tc_to(IMPLBURN + 012u, PHASCHNG);
}

/* "WAIT HALF A SECOND" (FIXDELAY, DEC 50), */
void implburn_after_phaschng(void)
{
    tc_to(IMPLBURN + 014u, FIXDELAY);
}

/* "TURN OFF ULLAGE" (NOULLAGE), */
void implburn_after_fixdelay(void)
{
    tc_to(IMPLBURN + 016u, NOULLAGE);
}

/* and the task ends. */
void implburn_after_noullage(void)
{
    tc_to(IMPLBURN + 017u, TASKOVER);
}

/* ---- ENGOFTSK and ENGINOFF -- pp. 760-761 ---- */

/* ENGOFTSK: ENGINOFF (IBNKCALL), */
void engoftsk(void)
{
    tc_to(ENGOFTSK, IBNKCALL); /* CADR ENGINOFF */
}

/* and the task ends. */
void engoftsk_after_enginoff(void)
{
    tc_to(ENGOFTSK + 2u, TASKOVER);
}

/* ENGINOFF: POSTBURN as a job, "MUST BE LOWER PRIO THAN CLOCKJOB"
 * (PRIO12, the number the instruction TCF 10000 is). FINDVAC comes back
 * at ENGINOF2. */
void enginoff(void)
{
    set_A(PRIO12_WORD);
    tc_to(ENGINOFF + 1u, FINDVAC); /* 2CADR POSTBURN */
}

/* ENGINOF2: COASTSET as a task in a centisecond (WAITLIST, BIT1), which
 * comes back at ENGINOF1. */
void enginof2(void)
{
    set_A(BIT1);
    tc_to(ENGINOF2 + 1u, WAITLIST); /* 2CADR COASTSET */
}

/* ENGINOF1: "SET THE IDLE BIT." (IDLEFLAG, FLAGWRD7), and the ullage off
 * (NOULLAGE), which comes back at ENGINOF4. */
void enginof1(void)
{
    cs(ADDRESS_OF(FLAGWRD7));
    mask(IDLEFBIT);
    ads(ADDRESS_OF(FLAGWRD7));
    tc_to(ENGINOF1 + 3u, NOULLAGE);
}

/* ENGINOF4: the time now in TEVENT, and on at ENGINOF3. */
void enginof4(void)
{
    dca(ADDRESS_OF(TIME2)); /* EXTEND; DCA */
    dxch(ADDRESS_OF(TEVENT));
    enginof3();
}

/* ENGINOF3: "INSURE ENGONFLG IS CLEAR." (FLAGWRD5); "TURN OFF THE ENGINE
 * -- DPS OR APS" (DSALMOUT: bit 13, engine on, clear; bit 14, engine
 * off, set); "TURN OFF TRIM GIMBAL" (USEQRJTS of DAPBOOLS set); "ZERO
 * AUTO-THROTTLE WHENEVER THE ENGINE IS TURNED OFF." (THRUST, and bit 4 of
 * channel 14); then back to the caller (ISWRETRN). "ENGINOF3 IS USED AS A
 * PRE-ENGINE ARM SUBROUTINE." */
void enginof3(void)
{
    set_A(negate(ENGONBIT));
    mask_at(ADDRESS_OF(FLAGWRD5));
    ts(ADDRESS_OF(FLAGWRD5));
    set_A(negate(PRIO30));
    rand_channel(DSALMOUT); /* EXTEND; RAND */
    ad(PRIO20);
    write_channel(DSALMOUT); /* EXTEND; WRITE */
    cs(ADDRESS_OF(DAPBOOLS));
    mask(USEQRJTS);
    ads(ADDRESS_OF(DAPBOOLS));
    set_A(negate(HIRTHROT));
    ts(ADDRESS_OF(THRUST));
    set_A(BIT4);
    wor_channel(CHAN14); /* EXTEND; WOR */
    tc_to(ENGINOF3 + 021u, ISWRETRN);
}

/* COASTSET: "DO DAP COASTING INITIALIZATION" (ALLCOAST), */
void coastset(void)
{
    tc_to(COASTSET, IBNKCALL); /* CADR ALLCOAST */
}

/* and the task ends. */
void coastset_after_allcoast(void)
{
    tc_to(COASTSET + 2u, TASKOVER);
}

#endif
