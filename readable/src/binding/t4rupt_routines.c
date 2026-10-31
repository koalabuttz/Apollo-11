/* SPDX-License-Identifier: MIT */
/* The readable T4RUPT core (src/t4rupt/) as routines of the flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/t4rupt_core.c): lm_t4rupt_core_routines, its hot copy
 * lm_t4rupt_core_hot_routines, and cm_t4rupt_core_routines.
 *
 * Plumbing only: point the machine at the program's state, give it
 * channel 10 (OUT0, the relay word) and channel 32 (the PRO key) from the
 * program's hardware record, and take the states the faithful routines
 * take: inside the interrupt, in bank 06; at T4RUPT no overflow in A,
 * DSRUPTSW a phase count of 8 at most when positive, DSPCNT 0-012 or -0
 * and no DSPTAB word (0-10) +0 or -0 (the display programs never leave
 * one, and a zero would send DSPSCAN round for ever or run TABLNTH); at
 * T4JUMP, RUPTREG1 0-7. */
#include <stddef.h>
#include "binding.h"
#include "../t4rupt/t4rupt.h"
#include "../agc/names.h"

#define BANK06 014000u

static agc_hardware *hardware_now;

static word channel_in(unsigned channel)
{
    if (channel != CHAN32) not_modelled();
    return hardware_now->channel32;
}

static void channel_out(unsigned channel, word value)
{
    if (channel != OUT0) not_modelled();
    hardware_now->channel10 = value;
}

/* The display table as the display programs keep it. */
static bool display_kept(void)
{
    word count = fetch(ADDRESS_OF(DSPCNT));
    unsigned i;

    if (count > TABLNTH && count != MINUS_ZERO) return false;
    for (i = 0; i <= 10u; ++i)
        if (DSPTAB(i) == PLUS_ZERO || DSPTAB(i) == MINUS_ZERO) return false;
    return true;
}

static bool takes_t4rupt(void)
{
    word phase = fetch(ADDRESS_OF(DSRUPTSW));

    if (!(phase & SIGN_BIT) && phase > 8u) return false;
    return display_kept() && overflow_of(A) == 0;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    word entry;

    if (!state || !program || !program->hardware || !bind_machine(program, state, device))
        return EXEC_OUT_OF_SCOPE;
    hardware_now = program->hardware;
    if (hardware_now->in_interrupt != 1u || FBANK != BANK06) return EXEC_OUT_OF_SCOPE;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    entry = Z;

    if (entry == T4JUMP) {
        if (RUPTREG1 > 7u) return EXEC_OUT_OF_SCOPE;
        t4jump();
#if LUMINARY099
    } else if (entry == T4JUMP + 3u) {
        t4jump_plus3();
    } else if (entry == T4JUMP + 7u) {
        t4jump_plus7();
#else
    } else if (entry == OPTTEST) {
        opttest();
#endif
    } else if (entry == T4RUPT) {
        if (!takes_t4rupt()) return EXEC_OUT_OF_SCOPE;
        t4rupt();
    } else {
        return EXEC_OUT_OF_SCOPE;
    }
    return EXEC_OK;
}

/* Keys: bank 06. */
#define B6(z) (uint16_t)(06u * 1024u + ((z) - 02000u))

#if LUMINARY099
static const agc_routine_entry routines[] = {
    {B6(K_T4RUPT), run}, {B6(K_T4JUMP), run}, {B6(K_T4JUMP + 3u), run}, {B6(K_T4JUMP + 7u), run}};
const agc_routine_table lm_t4rupt_core_routines = {routines, COUNT(routines)};
/* The same entries, which the C64's flight store packs beside T4RUPT's
 * radar step. */
const agc_routine_table lm_t4rupt_core_hot_routines = {routines, COUNT(routines)};
#else
static const agc_routine_entry routines[] = {
    {B6(K_T4RUPT), run}, {B6(K_T4JUMP), run}, {B6(K_OPTTEST), run}};
const agc_routine_table cm_t4rupt_core_routines = {routines, COUNT(routines)};
#endif
