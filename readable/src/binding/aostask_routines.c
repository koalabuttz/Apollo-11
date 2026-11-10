/* SPDX-License-Identifier: MIT */
/* The readable AOSTASK_AND_AOSJOB (src/aostask/), the LM's, as routines of
 * its flight program, under the table names and keys of the faithful
 * port's (c64-port/src/lm_aostask.c): lm_aostask_basic, its eleven keys in
 * bank 20, with the empty lm_aostask_interpretive beside it (the file has
 * no interpretive code).
 *
 * Plumbing only: point the machine at the program's state, give it channel
 * 12 from the program's hardware record (when there is one) and the rope's
 * words the file reads by address (bank 20 and fixed-fixed: the inertia
 * and CG coefficients, GIMBLBTS, the jet failure masks, and the divisors
 * DVOVSUB finds by the ADRES after its TC), and take the states the
 * faithful routines take: FB bank 20, the bank registers agreeing, ZERO
 * +0, the latches in range and every erasable word an ordinary one. A run
 * that stops at not_modelled() leaves the state, the io record and the
 * hardware record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "lm_aostask.h"
#include "../aostask/aostask.h"
#include "../agc/names.h"

#if LUMINARY099

#define BANK20 020u

#define B20(z) (uint16_t)(BANK20 * 1024u + ((z) - 02000u))

static jmp_buf back;
static agc_hardware *hardware_now;

static void leave(void)
{
    longjmp(back, 1);
}

static word channel_in(unsigned number)
{
    if (number != CHAN12 || !hardware_now) not_modelled();
    return hardware_now->channel12;
}

/* The rope's words in bank 20 and fixed-fixed (banks 2 and 3). */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if ((physical < BANK20 * 02000u || physical >= (BANK20 + 1u) * 02000u) &&
        (physical < 2u * 02000u || physical >= 4u * 02000u))
        not_modelled();
    if (!apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static bool as_expected(void)
{
    unsigned i;

    if (FBANK != (word)(BANK20 << 10) || agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u ||
        (EBANK & 0377u) || BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device, void (*routine)(void))
{
    mission_io *io = device, kept_io;
    agc_hardware kept_hardware;
    apollo_overlay_state before;

    if (!state || !program || !io || !bind_machine(program, state, io) || !as_expected())
        return EXEC_OUT_OF_SCOPE;
    hardware_now = program->hardware;
    agc.channel_in = channel_in;
    agc.fixed_memory = rope_word;
    agc.leave = leave;

    before = *state;
    kept_io = *io;
    if (hardware_now) kept_hardware = *hardware_now;
    if (setjmp(back)) {
        *state = before;
        *io = kept_io;
        if (hardware_now) *hardware_now = kept_hardware;
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    return EXEC_OK;
}

#define KEY(name)                                                                                  \
    static enum exec_status key_##name(const agc_native_program *program,                          \
                                       apollo_overlay_state *state, void *device)                  \
    {                                                                                              \
        return run(program, state, device, name);                                                  \
    }
KEY(one_accset)
KEY(one_accjob)
KEY(one_accjob_return)
KEY(one_accs)
KEY(one_accs_plus2)
KEY(one_accs_plus3)
KEY(docktest)
KEY(getaosuv_plus6)
KEY(u_axis_stored)
KEY(v_axis_stored)
KEY(invert)

static const agc_routine_entry keys[] = {
    {B20(ONE_ACCSET), key_one_accset},
    {B20(ONE_ACCJOB), key_one_accjob},
    {B20(ONE_ACCJOB + 2u), key_one_accjob_return},
    {B20(ONE_ACCS), key_one_accs},
    {B20(ONE_ACCS + 2u), key_one_accs_plus2},
    {B20(ONE_ACCS + 3u), key_one_accs_plus3},
    {B20(DOCKTEST), key_docktest},
    {B20(GETAOSUV + 6u), key_getaosuv_plus6},
    {B20(STORV - 013u), key_u_axis_stored},
    {B20(STORV + 6u), key_v_axis_stored},
    {B20(INVERT), key_invert},
};

const agc_routine_table lm_aostask_basic = {keys, COUNT(keys)};
const agc_routine_table lm_aostask_interpretive = {NULL, 0};

#endif
