/* SPDX-License-Identifier: MIT */
/* The readable keyboard and uplink interrupts (src/keyrupt/) as routines
 * of the flight program, under the table names and keys of the faithful
 * port's (c64-port/src/keyrupt_uprupt.c): lm_keyrupt_routines and
 * cm_keyrupt_routines.
 *
 * Plumbing only: point the machine at the program's state, give it
 * channel 15 (MNKEYIN, the key code) and channel 11 (DSALMOUT, the UPACT
 * light) from the program's hardware record, as the faithful routines
 * keep them in both missions, and take the states the faithful routines
 * take: inside the interrupt, FBANK at the file's bank, no overflow in A
 * where the routine starts with TS, and LOCCTR at a core set on NOVAC's
 * return. */
#include <stddef.h>
#include "binding.h"
#include "../keyrupt/keyrupt.h"
#include "../agc/names.h"
#include "keyrupt_uprupt.h"

#if LUMINARY099
#define FILE_BANK 04u /* KEYRUPT_UPRUPT.agc's bank */
#else
#define FILE_BANK 07u
#endif

/* Keys: the file's bank. */
#define KEY(z) (uint16_t)(FILE_BANK * 1024u + ((z) - 02000u))

static agc_hardware *hardware_now;

/* The one channel the routines read: MNKEYIN (15). */
static word channel_in(unsigned channel)
{
    if (channel != MNKEYIN) not_modelled();
    return (word)(hardware_now->channel15 & MINUS_ZERO);
}

/* NOVAC leaves LOCCTR at the new job's core set: 12k. The faithful
 * routines take core sets 0-6 (LOCCTR 0-72). */
static bool core_set(word locctr)
{
    return locctr % CORE_SET_WORDS == 0u && locctr <= 6u * CORE_SET_WORDS;
}

static bool takes(word entry)
{
    if (*agc.in_interrupt != 1u || FBANK != (word)(FILE_BANK << 10)) return false;
    if (entry == KEYRUPT1 || entry == UPRUPT || entry == KEYCOM) return overflow_of(A) == 0;
    if (entry == ACCEPTUP + 4u) return core_set(LOCCTR);
    return true;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    word entry;

    if (!state || !program || !program->hardware || !bind_machine(program, state, device))
        return EXEC_OUT_OF_SCOPE;
    hardware_now = program->hardware;
    agc.dsalmout = &hardware_now->channel11;
    agc.channel_in = channel_in;
    entry = Z;
    if (!takes(entry)) return EXEC_OUT_OF_SCOPE;

    if (entry == KEYRUPT1)
        keyrupt1();
    else if (entry == KEYRUPT1 + 4u)
        keyrupt1_after_lodsampt();
    else if (entry == KEYCOM)
        keycom();
    else if (entry == ACCEPTUP + 4u)
        acceptup_after_novac();
    else if (entry == UPRUPT)
        uprupt();
    else if (entry == UPRUPT + 4u)
        uprupt_after_lodsampt();
    else
        return EXEC_OUT_OF_SCOPE;
    return EXEC_OK;
}

/* In the faithful table's order (by key). */
static const agc_routine_entry routines[] = {{KEY(K_KEYRUPT1), run}, {KEY(K_KEYRUPT1 + 4u), run},
                                             {KEY(K_KEYCOM), run},   {KEY(K_ACCEPTUP + 4u), run},
                                             {KEY(K_UPRUPT), run},   {KEY(K_UPRUPT + 4u), run}};
#if LUMINARY099
const agc_routine_table lm_keyrupt_routines = {routines, COUNT(routines)};
#else
const agc_routine_table cm_keyrupt_routines = {routines, COUNT(routines)};
#endif
